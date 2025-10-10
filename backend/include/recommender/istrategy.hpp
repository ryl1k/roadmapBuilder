#pragma once

#include "../models/plan.hpp"
#include "../models/course.hpp"
#include "../models/user_profile.hpp"

class IRecommenderStrategy {
public:
    virtual Plan makePlan(const UserProfile& profile, const std::vector<Course>& allCourses) = 0;
    virtual ~IRecommenderStrategy() = default;
};