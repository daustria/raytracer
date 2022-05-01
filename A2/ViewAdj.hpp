#ifndef VIEW_ADJUSTOR_H
#define VIEW_ADJUSTOR_H

class ViewAdjustor {

public:
	//Default constructor is good enough

	void initLeft(float value, float increment, float max, float min);
	void initMiddle(float, float, float, float);
	void initRight(float, float, float, float);
	void initAll(float, float, float, float);

	float left() const;
	float right() const;
	float middle() const;

private:

	struct ClampedFloat {

		ClampedFloat(float value = 0, float increment = 0, float max = 0, float min = 0);
		void incrementValue(bool positive = true); 

		float value;
		float increment;
		float maximum;
		float minimum;
	};

	ClampedFloat left_;
	ClampedFloat right_;
	ClampedFloat middle_;

};


#endif //VIEW_ADJUSTOR_H
