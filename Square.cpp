#pragma once
#include "square.hpp"

Square::Square() 
{
	width = height = 0.0f;
}

Square::Square(float width, float height)
{
	this->SetWidth(width);
	this->SetHeight(height);
}