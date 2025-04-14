#pragma once



class Square
{
public:
	Square();
	Square(float width, float height);
	inline float GetWidth() { return width; }
	inline void SetWidth(float width) { this->width = width; }

	inline float GetHeight() { return height; }
	inline void SetHeight(float height) { this->height = height;  }

	inline float GetArea() { return width * height; }
private:
	float width;
	float height;
	float x;
	float y;
};