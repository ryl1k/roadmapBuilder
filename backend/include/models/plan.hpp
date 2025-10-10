#pragma once

#include <string>
#include <vector>

struct PlanStep {
	int step;
	int courseId;
	int hours;
	std::string note;
};

class Plan {
	std::vector<PlanStep> steps;
	int totalHours;
public:

	// Getters

	const std::vector<PlanStep>& getSteps() const { return steps; }
	int getTotalHours() const { return totalHours; }

	// Setters

	void setSteps(const std::vector<PlanStep>& s) { steps = s; }
	void setTotalHours(int h) { totalHours = h; }

};