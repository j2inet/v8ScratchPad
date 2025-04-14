#pragma once



class Square
{
public:
	Square();
	Square(double width);
	inline float GetWidth() { return width; }
	inline void SetWidth(float width) { this->width = width; }

	inline float GetHeight() { return height; }
	inline void SetHeight(float height) { this->height = height;  }

	inline float GetArea() { return width * height; }
private:
	double width;
	double height;
	double x;
	double y;
};