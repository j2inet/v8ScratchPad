(

    function () {
        x = 5;
        var mx = 2;
        x = mx * x;
        print(x, 'test');
        var sqr = makeSquare(7);
        print('The square width is ', sqr.width);
        return x;
    }
)()