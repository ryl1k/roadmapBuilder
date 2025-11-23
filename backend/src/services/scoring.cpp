#include "../../include/services/scoring.hpp"
#include <algorithm>
#include <cmath>

double ScoringService::matchScore(const Course& course, const UserProfile& profile) {
    double score = 0.0;

    // 1. Domain match (40% weight)
    if (course.getDomain() == profile.getTargetDomain()) {
        score += 0.4;
    }

    // 2. Level appropriateness (30% weight)
    double levelScore = 0.0;
    std::string courseLevel = course.getLevel();
    std::string userLevel = profile.getCurrentLevel();

    if (courseLevel == userLevel) {
        levelScore = 1.0; // Perfect match
    } else if (userLevel == "Beginner" && courseLevel == "Intermediate") {
        levelScore = 0.7; // Acceptable progression
    } else if (userLevel == "Intermediate" && courseLevel == "Advanced") {
        levelScore = 0.7; // Acceptable progression
    } else if (userLevel == "Intermediate" && courseLevel == "Beginner") {
        levelScore = 0.3; // Too easy but acceptable
    } else if (userLevel == "Advanced" && courseLevel == "Intermediate") {
        levelScore = 0.3; // Too easy but acceptable
    } else {
        levelScore = 0.1; // Poor match
    }
    score += 0.3 * levelScore;

    // 3. Interest/tags match (30% weight)
    int matchingTags = 0;
    auto interests = profile.getInterests();
    auto tags = course.getTags();

    for (const auto& interest : interests) {
        for (const auto& tag : tags) {
            if (tag.find(interest) != std::string::npos ||
                interest.find(tag) != std::string::npos) {
                matchingTags++;
                break;
            }
        }
    }

    if (!interests.empty()) {
        double tagMatchRatio = static_cast<double>(matchingTags) / interests.size();
        score += 0.3 * tagMatchRatio;
    }

    // Bonus: Use course's inherent score if available
    if (course.getScore() > 0) {
        score *= course.getScore();
    }

    return score;
}
