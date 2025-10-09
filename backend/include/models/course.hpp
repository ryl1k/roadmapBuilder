#pragma once

#include <string>
#include <vector>

class Course {
	int id;
	std::string title;
	std::string domain;
	std::string level;
	int durationHours;
	double score;
	std::vector<std::string> tags;
	std::vector<int> prerequisiteCourseIds;

public:

	int getId() const { return id; }
	std::string getTitle() const { return title; }
	std::string getDomain() const { return domain; }
	std::string getLevel() const { return level; }
	int getDurationHours() const { return durationHours; }
	double getScore() const { return score; }
	std::vector<std::string> getTags() const { return tags; }
	std::vector<int> getPrerequisiteCourseIds() const { return prerequisiteCourseIds; }


};