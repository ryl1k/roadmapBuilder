#pragma once

#include "../models/course.hpp"
#include "../models/user_profile.hpp"
#include "../models/plan.hpp"
#include "../third_party/json.hpp"
#include <string>

using json = nlohmann::json;

// Course serialization
inline json courseToJson(const Course& course) {
    return json{
        {"id", course.getId()},
        {"title", course.getTitle()},
        {"domain", course.getDomain()},
        {"level", course.getLevel()},
        {"durationHours", course.getDurationHours()},
        {"score", course.getScore()},
        {"tags", course.getTags()},
        {"prerequisiteCourseIds", course.getPrerequisiteCourseIds()}
    };
}

inline Course jsonToCourse(const json& j) {
    Course course;
    course.setId(j.value("id", 0));
    course.setTitle(j.value("title", ""));
    course.setDomain(j.value("domain", ""));
    course.setLevel(j.value("level", ""));
    course.setDurationHours(j.value("durationHours", 0));
    course.setScore(j.value("score", 1.0));

    if (j.contains("tags")) {
        course.setTags(j["tags"].get<std::vector<std::string>>());
    }

    if (j.contains("prerequisiteCourseIds")) {
        course.setPrerequisiteCourseIds(j["prerequisiteCourseIds"].get<std::vector<int>>());
    }

    return course;
}

// UserProfile serialization
inline json profileToJson(const UserProfile& profile) {
    return json{
        {"userId", profile.getUserId()},
        {"targetDomain", profile.getTargetDomain()},
        {"currentLevel", profile.getCurrentLevel()},
        {"interests", profile.getInterests()},
        {"hoursPerWeek", profile.getHoursPerWeek()},
        {"deadlineWeeks", profile.getDeadlineWeeks()}
    };
}

inline UserProfile jsonToProfile(const json& j) {
    UserProfile profile;
    profile.setUserId(j.value("userId", 0));
    profile.setTargetDomain(j.value("targetDomain", ""));
    profile.setCurrentLevel(j.value("currentLevel", ""));
    profile.setInterests(j.value("interests", std::vector<std::string>{}));
    profile.setHoursPerWeek(j.value("hoursPerWeek", 0));
    profile.setDeadlineWeeks(j.value("deadlineWeeks", 0));
    return profile;
}

// PlanStep serialization
inline json planStepToJson(const PlanStep& step) {
    return json{
        {"step", step.step},
        {"courseId", step.courseId},
        {"hours", step.hours},
        {"note", step.note}
    };
}

// Plan serialization
inline json planToJson(const Plan& plan) {
    json stepsArray = json::array();
    for (const auto& step : plan.getSteps()) {
        stepsArray.push_back(planStepToJson(step));
    }

    return json{
        {"totalHours", plan.getTotalHours()},
        {"steps", stepsArray}
    };
}

// Helper for vectors
inline json coursesToJson(const std::vector<Course>& courses) {
    json array = json::array();
    for (const auto& course : courses) {
        array.push_back(courseToJson(course));
    }
    return array;
}
