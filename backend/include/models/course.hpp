#pragma once

#include <string>
#include <vector>

// Сутність "Курс", що містить інформацію про курс, його характеристики та вимоги


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

	// Getters

	int getId() const { return id; }
	std::string getTitle() const { return title; }
	std::string getDomain() const { return domain; }
	std::string getLevel() const { return level; }
	int getDurationHours() const { return durationHours; }
	double getScore() const { return score; }
	std::vector<std::string> getTags() const { return tags; }
	std::vector<int> getPrerequisiteCourseIds() const { return prerequisiteCourseIds; }

	// Setters

	void setId(int _id) { id = _id; }
	void setTitle(const std::string& _title) { title = _title; }
	void setDomain(const std::string& _domain) { domain = _domain; }
	void setLevel(const std::string& _level) { level = _level; }
	void setDurationHours(int _durationHours) { durationHours = _durationHours; }
	void setScore(double _score) { score = _score; }
	void setTags(const std::vector<std::string>& _tags) { tags = _tags; }
	void setPrerequisiteCourseIds(const std::vector<int>& _prerequisiteCourseIds) { prerequisiteCourseIds = _prerequisiteCourseIds; }


};