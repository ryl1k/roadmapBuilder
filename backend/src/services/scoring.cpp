#include "../../include/services/scoring.hpp"
#include <algorithm>
#include <cmath>

double ScoringService::matchScore(const Course& course, const UserProfile& profile) {
    double score = 0.0;

    // 1. Domain match (20% weight - reduced because we filter by domain first)
    if (course.getDomain() == profile.getTargetDomain()) {
        score += 0.2;
    }
    // Bonus for AI/Data Science cross-compatibility
    else if ((profile.getTargetDomain() == "AI" && course.getDomain() == "Data Science") ||
             (profile.getTargetDomain() == "Data Science" && course.getDomain() == "AI")) {
        score += 0.15;
    }

    // 2. Level appropriateness (30% weight)
    double levelScore = 0.0;
    std::string courseLevel = course.getLevel();
    std::string userLevel = profile.getCurrentLevel();

    if (courseLevel == userLevel) {
        levelScore = 1.0; // Perfect match
    } else if (userLevel == "Beginner" && courseLevel == "Intermediate") {
        levelScore = 0.8; // Good progression
    } else if (userLevel == "Intermediate" && courseLevel == "Advanced") {
        levelScore = 0.8; // Good progression
    } else if (userLevel == "Intermediate" && courseLevel == "Beginner") {
        levelScore = 0.4; // Too easy but might fill gaps
    } else if (userLevel == "Advanced" && courseLevel == "Intermediate") {
        levelScore = 0.5; // Prerequisites/refresher
    } else if (userLevel == "Advanced" && courseLevel == "Beginner") {
        levelScore = 0.2; // Rarely needed
    } else {
        levelScore = 0.1; // Poor match
    }
    score += 0.3 * levelScore;

    // 3. Interest/tags match (50% weight - INCREASED for better relevance)
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
        score += 0.5 * tagMatchRatio;
    }

    // Bonus: Use course's inherent score if available
    if (course.getScore() > 0) {
        score *= course.getScore();
    }

    return score;
}
