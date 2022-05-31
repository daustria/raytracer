#pragma once

#include "SceneNode.hpp"

class JointNode : public SceneNode {
public:
	JointNode(const std::string & name);
	virtual ~JointNode();

	void set_joint_x(double min, double init, double max);
	void set_joint_y(double min, double init, double max);

	virtual void rotate(char axis, float angle) override;

	struct JointRange {
		double min, init, max;
	};

	JointRange m_joint_x, m_joint_y;

	// Keep track of total rotations in the x and y axis, so that 
	// we can choose to not rotate when the total rotations exceed
	// the bounds of JointRange
private:	
	double m_total_rotations_x; 
	double m_total_rotations_y; 

	void update_total_rotations(char axis, double angle);
};
