#pragma once

#include "../models/course.hpp"
#include "../models/user_profile.hpp"

class ScoringService {
public:
    double matchScore(const Course& course, const UserProfile& profile);
};
