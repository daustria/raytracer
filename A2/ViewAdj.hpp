#ifndef VIEW_ADJUSTOR_H
#define VIEW_ADJUSTOR_H

class ViewAdjustor {

public:
	ViewAdjustor();

	void initX(float value, float increment, float max, float min);
	void initY(float, float, float, float);
	void initZ(float, float, float, float);
	void initAll(float, float, float, float);

	void incrementX(float factor, bool positive = true);
	void incrementY(float factor, bool positive = true);
	void incrementZ(float factor, bool positive = true);
	void incrementAll(float factor, bool positive = true);

	//provide read-only access to the fields
	const float &x; 
	const float &y;
	const float &z;

private:

	struct ClampedFloat {

		ClampedFloat(float value = 0, float increment = 0, float max = 0, float min = 0);
		void incrementValue(float factor, bool positive = true); 

		float value;
		float increment;
		float maximum;
		float minimum;
	};

	ClampedFloat x_;
	ClampedFloat y_;
	ClampedFloat z_;

};

#endif //VIEW_ADJUSTOR_H
