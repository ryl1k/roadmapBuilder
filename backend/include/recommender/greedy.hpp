#pragma once

#include "../services/scoring.hpp"
#include "istrategy.hpp"

class GreedyRecommender : public IRecommenderStrategy {
    ScoringService scorer;
public:
    Plan makePlan(const UserProfile& profile, const std::vector<Course>& allCourses) override;
};