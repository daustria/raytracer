#ifndef VIEW_ADJUSTOR_H
#define VIEW_ADJUSTOR_H

class ViewAdjustor {

public:
	//Default constructor is good enough

	void initX(float value, float increment, float max, float min);
	void initY(float, float, float, float);
	void initZ(float, float, float, float);
	void initAll(float, float, float, float);

	void incrementX(bool positive = true);
	void incrementY(bool positive = true);
	void incrementZ(bool positive = true);
	void incrementAll(bool positive = true);

	float x() const;
	float y() const;
	float z() const;


private:

	struct ClampedFloat {

		ClampedFloat(float value = 0, float increment = 0, float max = 0, float min = 0);
		void incrementValue(bool positive = true); 

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
