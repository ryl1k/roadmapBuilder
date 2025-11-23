#include "../../include/recommender/greedy.hpp"
#include <algorithm>
#include <set>

Plan GreedyRecommender::makePlan(const UserProfile& profile, const std::vector<Course>& allCourses) {
    Plan plan;
    std::vector<PlanStep> steps;
    int totalHours = 0;

    int totalAvailableHours = profile.getHoursPerWeek() * profile.getDeadlineWeeks();

    // Score all courses
    std::vector<std::pair<double, Course>> scoredCourses;
    for (const auto& course : allCourses) {
        double score = scorer.matchScore(course, profile);
        scoredCourses.push_back({score, course});
    }

    // Sort by score descending
    std::sort(scoredCourses.begin(), scoredCourses.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Greedy selection with prerequisite handling
    std::set<int> completedCourseIds;
    int stepNumber = 1;

    for (const auto& [score, course] : scoredCourses) {
        // Check if we have enough time
        if (totalHours + course.getDurationHours() > totalAvailableHours) {
            continue;
        }

        // Check prerequisites are met
        bool prereqsMet = true;
        for (int prereqId : course.getPrerequisiteCourseIds()) {
            if (completedCourseIds.find(prereqId) == completedCourseIds.end()) {
                prereqsMet = false;
                break;
            }
        }

        if (!prereqsMet) {
            continue;
        }

        // Add course to plan
        PlanStep step;
        step.step = stepNumber++;
        step.courseId = course.getId();
        step.hours = course.getDurationHours();
        step.note = "Score: " + std::to_string(score);

        steps.push_back(step);
        totalHours += course.getDurationHours();
        completedCourseIds.insert(course.getId());
    }

    plan.setSteps(steps);
    plan.setTotalHours(totalHours);
    return plan;
}