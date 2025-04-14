#pragma once
#include "square.hpp"

Square::Square() 
{
	width = height = 0.0f;
}

Square::Square(double width )
{
	this->SetWidth(width);
	this->SetHeight(width);
}