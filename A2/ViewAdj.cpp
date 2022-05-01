#include "ViewAdj.hpp"

// Clamped Float --------------------------------------------
ViewAdjustor::ClampedFloat::ClampedFloat(float value, float increment, float max, float min) : 
	value(value), increment(increment), maximum(max), minimum(min) 
{

}

void ViewAdjustor::ClampedFloat::incrementValue(bool positive) 
{
	float new_value = value + (positive ? increment : -1*increment);

	if (new_value <= minimum || new_value >= maximum) {
		return;
	}

	value = new_value;
}

// View Adjustor-------------------------------------------------------------------------------
void ViewAdjustor::initLeft(float value, float increment, float max, float min) 
{
	left_ = ClampedFloat(value, increment, max, min);
}

void ViewAdjustor::initMiddle(float value, float increment, float max, float min) 
{
	middle_ = ClampedFloat(value, increment, max, min);
}

void ViewAdjustor::initRight(float value, float increment, float max, float min) 
{
	right_ = ClampedFloat(value, increment, max, min);
}

void ViewAdjustor::initAll(float value, float increment, float max, float min)
{
	initLeft(value, increment, max, min);	
	initRight(value, increment, max, min);	
	initMiddle(value, increment, max, min);	
}

// Getters 
float ViewAdjustor::left() const
{
	return left_.value;
}

float ViewAdjustor::right() const
{
	return right_.value;
}

float ViewAdjustor::middle() const
{
	return middle_.value;
}

