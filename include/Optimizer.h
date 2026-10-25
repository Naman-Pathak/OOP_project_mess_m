#pragma once
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "Analyzer.h"
#include "Forecaster.h"
#include "Tracker.h"

struct CookPlanItem {
    std::string dish;
    bool hasHistory = false;
    double perHeadKg = 0;     // demand per student used for the plan
    double plannedKg = 0;     // recommended quantity to cook
    double baselineKg = 0;    // what is normally cooked for this dish
    double savedKg = 0;       // baseline - planned
    double savedCost = 0;     // Rs
};

struct CookPlan {
    std::string date, meal;
    int forecastHeads = 0;
    std::string method;
    std::vector<CookPlanItem> items;
    std::vector<std::string> unknownDishes;
    double totalPlannedKg = 0, totalBaselineKg = 0, totalSavedKg = 0, totalSavedCost = 0;
};

// Weights of the dish score. They should add up to 1.
struct MenuWeights {
    double acceptance = 0.45;  // how much of it students actually eat
    double cost = 0.25;        // cheaper than similar dishes
    double nutrition = 0.30;   // protein compared with similar dishes
};

struct MenuOption {
    std::vector<Dish> dishes;
    double score = 0;
    double costPerPlate = 0;
    int calories = 0;
    double protein = 0;
};

class Optimizer {
public:
    explicit Optimizer(const Tracker& tracker, double safetyBuffer = 0.05,
                       MenuWeights weights = MenuWeights())
        : tracker_(tracker), buffer_(safetyBuffer), weights_(weights) {}

    // How much of each dish to cook for the forecast headcount.
    CookPlan planQuantities(const std::string& date, const std::string& meal,
                            const std::vector<std::string>& dishNames) const;

    // Score (0..1) of every dish.
    std::map<std::string, double> dishScores() const;

    // Best combination of dishes for a meal within a budget and calorie minimum.
    bool bestMenu(const std::string& meal, double budget, int minCalories,
                  const std::set<std::string>& excluded, MenuOption& out) const;

    // 7-day plan; avoids repeating the same dishes on consecutive days.
    std::vector<std::pair<std::string, MenuOption>> weeklyPlan(
        const std::string& meal, const std::string& startDate, double budget,
        int minCalories) const;

    static double defaultBudget(const std::string& meal);   // Rs per plate
    static int defaultMinCalories(const std::string& meal);

private:
    const Tracker& tracker_;
    double buffer_;
    MenuWeights weights_;
};
