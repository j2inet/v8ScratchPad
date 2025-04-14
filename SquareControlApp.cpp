// SquareControlApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _ITERATOR_DEBUG_LEVEL 0
#include <iostream>
#include <fstream>
#include <sstream>
#include "v8_options.hpp"
#include "square.hpp"


v8::Global<v8::Context> currentContext;
v8::Isolate* isolate_;


// Implementing a global variable named X that will be available in the 
// JavaScript environment. The variable is an integer. Whenever the variable
// is read or written to, the C++ code below is invoked. In addition to 
// enabling the two environments to share information, this could be used to 
// invoke other functionality. Ex: If this variable were for the position of a
// window, then the C++ code could perform the additional tasks needed to move
// the window.
int static x = -1;
void XGetter(
    v8::Local<v8::Name> propertyName, 
    const v8::PropertyCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(x);
}

void XSetter(
    v8::Local<v8::Name> propertyName,
    v8::Local<v8::Value> value, 
    const v8::PropertyCallbackInfo<void>& info)
{
    auto  v = value->Int32Value(currentContext.Get(isolate_));
    if (v.IsJust())
    {
        x = v.FromJust();
    }
}

// I'm not sure that I should be using a v8::Local as a global variable. Let's
//consider this to be incorrect. It works, but being fucntional is a necessary
// but not sufficient requirement.
v8::Local<v8::ObjectTemplate> square_templ;

// Any square objects made in the code will be placed here. 
std::vector<Square> squareList;

//Creates a square in native code and returns a JavaScript handle to the object.
void MakeSquareFunction(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::HandleScope handle_scope(info.GetIsolate());
    auto context = info.GetIsolate()->GetCurrentContext();
    double width = 1;
    if (info.Length() >= 1)
    {
        v8::Maybe<double> size = info[0]->NumberValue(context);
        if (size.IsJust())
        {
            width = size.FromMaybe(width);
        }
        Square sq(width);
        squareList.push_back(sq);
        v8::MaybeLocal<v8::Object> obj = square_templ->NewInstance(context);
        if (!obj.IsEmpty())
        {
            v8::Local<v8::Object> resolvedObject;
            if (obj.ToLocal(&resolvedObject))
            {
                auto field = v8::External::New(info.GetIsolate(), &squareList[squareList.size() - 1]);
                resolvedObject->SetInternalField(0, field);
                info.GetReturnValue().Set(resolvedObject);
            }
        }
    }
}

// Implementing a static global function that will be available in the 
// JavaScript environment. This implementation accepts any number of 
// parameters, including zero parameters. It reads the actual number of params
// from the `Length()` accessor of the `info` parameter. All values are 
// converted to strings and then printed to the console.Null values are not
// written
void PrintFunction(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    bool first = true;
    v8::HandleScope handle_scope(info.GetIsolate());
    for (int i = 0; i < info.Length(); ++i)
    {        
        if (first)
        {
            first = false;
        }
        else
        {
            std::wcout << L" ";
        }
        auto itemStringValue = info[i]->ToString(info.GetIsolate()->GetCurrentContext());
        if (!itemStringValue.IsEmpty())
        {
            v8::Local<v8::String> localStringValue = itemStringValue.ToLocalChecked();
            
            std::vector<char> buffer(localStringValue->Length());
            localStringValue->WriteUtf8(info.GetIsolate(), buffer.data(), buffer.size());
            std::string printableString(buffer.data(), localStringValue->Length());
            std::cout << printableString;
        }
    }
    std::cout << std::endl;
}


void SquareFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();
    args.GetReturnValue().Set(v8::String::NewFromUtf8Literal(isolate, "This function is not for returning values"));
}

void SquareWidthGetter(
    v8::Local<v8::Name> propertyName,  
    const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = self->GetInternalField(0).As<v8::Value>().As<v8::External>();
    //v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    float value = static_cast<Square*>(ptr)->GetWidth();
    info.GetReturnValue().Set(value);
}

void SquareWidthSetter(
    v8::Local<v8::Name> propertyName, 
    v8::Local<v8::Value> value, 
    const v8::PropertyCallbackInfo<void>& info)
{
    auto numberValue = value->NumberValue(currentContext.Get(isolate_));
    if (numberValue.IsJust())
    {
        v8::Local<v8::Object> self = info.Holder();
        v8::Local<v8::Data> internalField = self->GetInternalField(0);
        if (internalField->IsValue())
        {
            v8::Local<v8::Value> internalValue = v8::Local<v8::Value>::Cast(internalField);
        }
        //v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(internalField);
        v8::Local<v8::External> wrap = self->GetInternalField(0).As<v8::Value>().As<v8::External>();
        void* ptr = wrap->Value();
        static_cast<Square*>(ptr)->SetWidth(numberValue.FromJust());
    }
}

// Because I tend to use wide characters by default, I'm using wmain instead
// of main.
int wmain(int argc, wchar_t** argv)
{
    //Personal preference, I move the arguments from the array of pointers
    //to safer structures to minimize working with pointers whenever possible.
    //This is in furtherence of safer code and less need to debug.
    std::vector<std::wstring> argList(argc);
    for (auto i = 0; i < argc; ++i)
    {
        argList.push_back(std::wstring(argv[0]));
    }
    //Though I prefer wide characters, v8 requires utf8 for many functions. I'm
    //converting the first argument to utf8 to call a v8 function.
    std::string firstArgument = std::string( argList[0].begin(), argList[0].end());
    v8::V8::InitializeICUDefaultLocation(firstArgument.c_str());
    v8::V8::InitializeExternalStartupData(firstArgument.c_str());
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate_ = v8::Isolate::New(create_params);
    {
/*
        v8::Local<v8::FunctionTemplate> squareTemplate = v8::FunctionTemplate::New(isolate_);
        squareTemplate->Set(isolate_, "Square", v8::FunctionTemplate::New(isolate_, SquareFunction));
        squareTemplate->PrototypeTemplate()->Set(
            v8::String::NewFromUtf8(isolate_,"Square"),
            v8::FunctionTemplate::New(isolate_, SquareMethodCallback)->GetFunction()
        );
  */      
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);

        v8::Local<v8::ObjectTemplate> global_templ = v8::ObjectTemplate::New(isolate_);
        global_templ->SetNativeDataProperty(v8::String::NewFromUtf8Literal(isolate_, "x"), XGetter, XSetter);
        global_templ->Set(isolate_, "print", v8::FunctionTemplate::New(isolate_, PrintFunction));
        global_templ->Set(isolate_, "makeSquare", v8::FunctionTemplate::New(isolate_, MakeSquareFunction));
       


        /*v8::Local<v8::ObjectTemplate>*/ square_templ = v8::ObjectTemplate::New(isolate_);
        square_templ->SetInternalFieldCount(1);
        square_templ->SetNativeDataProperty(v8::String::NewFromUtf8Literal(isolate_, "width"), SquareWidthGetter, SquareWidthSetter);



        

        // Create a new context.
        v8::Local<v8::Context> context = v8::Context::New(isolate_, NULL, global_templ);
        currentContext.Reset(isolate_, context);
        v8::Context::Scope context_scope(context);

    /*
        Square square1, square2;
        v8::Local<v8::Object> square_1 = square_templ->NewInstance(context).ToLocalChecked();
        v8::Local<v8::Object> square_2 = square_templ->NewInstance(context).ToLocalChecked();
            auto v1 = v8::External::New(isolate_, &square1);
            square_1->SetInternalField(0, v8::External::New(isolate_, &square1));
            square_2->SetInternalField(0, v8::External::New(isolate_, &square2));
            global_templ->SetNativeDataProperty(v8::String::NewFromUtf8Literal(isolate_, "sq1"))
      */  
        // Enter the context for compiling and running the hello world script.

        std::string scriptString;
        std::ifstream scriptSourceFile("testScript.js");
        if (!scriptSourceFile.is_open())
        {
            return -1;
        }
        std::stringstream buffer;
        buffer << scriptSourceFile.rdbuf();
        std::string scriptSource= buffer.str();
        //char script[8];
        v8::Local<v8::String> sourceCode;
        v8::String::NewFromUtf8(isolate_, scriptSource.c_str(), v8::NewStringType::kNormal, scriptSource.length()).ToLocal(&sourceCode);;
        // v8::String::NewFromUtf8Literal(isolate_, scriptSource.c_str(), v8::NewStringType::kNormal, scriptSource.length());
        /*"(function(){x = 5;"
        ""
        "var mx = 2;"
        "x = mx * x;"
        "print(x,'test');"
        "return x;})()"*/
        v8::TryCatch trycatch(isolate_);
        v8::Local<v8::Script> script;

        v8::MaybeLocal<v8::Script> maybeScript = v8::Script::Compile(context, sourceCode);
        if (!maybeScript.IsEmpty())
        {
            script = maybeScript.ToLocalChecked();
        } 
        else
        {
            //Something catastrophic happened. Not sure what to do here.
        }

        //We could have gotten this far even with a syntax error.
        //How do we detect this before the script runs, or capture
        //the error that rises after it runs?
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        if (result.IsEmpty()) {
            //Something bad happened
            v8::Local<v8::Value> exc = trycatch.Exception();
            v8::String::Utf8Value exception_str(isolate_, exc);
            std::cout << *exception_str << std::endl;
        }
        v8::MaybeLocal<v8::String> resultString = result->ToString(context);
        v8::Local<v8::String> resolvedString;
        if (!resultString.ToLocal(&resolvedString))
        {
            v8::String::Utf8Value utf8(isolate_, resolvedString);
            std::string retVal(*utf8);
            std::cout << "Result " << retVal << std::endl;
        }
        else
        {
            std::cout << "No value returned" << std::endl;
        }
        std::cout << "Final value of X:" << x << std::endl;

    }
    // Dispose the isolate and tear down V8.
    currentContext.Reset();
    isolate_->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}


