#pragma once

#include <string>
#include <vector>

class UserProfile {
	int userId;
	std::string targetDomain;
	std::string currentLevel;
	std::vector<std::string> interests;
	int hoursPerWeek;
	int deadlineWeeks;
public:

	// Getters

	int getUserId() const { return userId; }
	std::string getTargetDomain() const { return targetDomain; }
	std::string getCurrentLevel() const { return currentLevel; }
	std::vector<std::string> getInterests() const { return interests; }
	int getHoursPerWeek() const { return hoursPerWeek; }
	int getDeadlineWeeks() const { return deadlineWeeks; }

	// Setters

	void setUserId(int id) { userId = id; }
	void setTargetDomain(const std::string& domain) { targetDomain = domain; }
	void setCurrentLevel(const std::string& level) { currentLevel = level; }
	void setInterests(const std::vector<std::string>& interestList) { interests = interestList; }
	void setHoursPerWeek(int hours) { hoursPerWeek = hours; }
	void setDeadlineWeeks(int weeks) { deadlineWeeks = weeks; }

};