#include "Optimizer.h"

#include <cmath>

#include "Utils.h"

namespace {

// Which categories make up a plate at each meal.
struct Structure {
    std::vector<std::string> required;  // exactly one dish from each
    std::vector<std::string> optional;  // zero or one dish from each
};

Structure structureFor(const std::string& meal) {
    // Breakfast: a main item, tea/coffee and milk, plus one optional side.
    if (meal == "Breakfast") return {{"Main", "Beverage", "Milk"}, {"Side"}};
    // Lunch and Dinner follow the mess slots: Common (staple + dal), Dry item, Gravy, + optional Side, Dessert.
    return {{"Staple", "Dal", "Dry", "Gravy"}, {"Side", "Dessert"}};
}

struct SearchState {
    const std::vector<std::vector<const Dish*>>& options;  // nullptr entry = "skip this category"
    const std::map<std::string, double>& scores;
    double budget;
    int minCalories;
    std::vector<const Dish*> current;
    MenuOption best;
    bool found;
};

// Exhaustive search over all combinations (the numbers are small: a few hundred).
void search(SearchState& st, size_t idx, double cost, int calories, double protein, double score) {
    if (idx == st.options.size()) {
        if (calories >= st.minCalories && (!st.found || score > st.best.score + 1e-12)) {
            st.found = true;
            st.best.score = score;
            st.best.costPerPlate = cost;
            st.best.calories = calories;
            st.best.protein = protein;
            st.best.dishes.clear();
            for (const Dish* d : st.current) st.best.dishes.push_back(*d);
        }
        return;
    }
    for (const Dish* d : st.options[idx]) {
        if (d == nullptr) {
            search(st, idx + 1, cost, calories, protein, score);
            continue;
        }
        const double newCost = cost + d->costPerPlate();
        if (newCost > st.budget + 1e-9) continue;  // over budget: prune this branch
        auto it = st.scores.find(d->name);
        st.current.push_back(d);
        search(st, idx + 1, newCost, calories + d->calories, protein + d->proteinG,
               score + (it != st.scores.end() ? it->second : 0.0));
        st.current.pop_back();
    }
}

}  // namespace

double Optimizer::defaultBudget(const std::string& meal) {
    if (meal == "Breakfast") return 45.0;
    return 75.0;  // Lunch and Dinner
}

int Optimizer::defaultMinCalories(const std::string& meal) {
    if (meal == "Breakfast") return 450;
    return 650;  // Lunch and Dinner
}

std::map<std::string, double> Optimizer::dishScores() const {
    std::map<std::string, DishStats> stats;
    for (const auto& s : Analyzer(tracker_).dishStats()) stats[s.dish] = s;

    struct Range { double cMin = 1e18, cMax = -1e18, pMin = 1e18, pMax = -1e18; };
    std::map<std::string, Range> ranges;  // compare dishes only with dishes of the same category
    for (const auto& d : tracker_.dishes()) {
        Range& r = ranges[d.category];
        r.cMin = std::min(r.cMin, d.costPerPlate());
        r.cMax = std::max(r.cMax, d.costPerPlate());
        r.pMin = std::min(r.pMin, d.proteinG);
        r.pMax = std::max(r.pMax, d.proteinG);
    }

    std::map<std::string, double> out;
    for (const auto& d : tracker_.dishes()) {
        const Range& r = ranges[d.category];
        auto it = stats.find(d.name);
        const double acceptance = (it != stats.end()) ? it->second.acceptance : 0.5;  // unknown = neutral
        const double costScore =
            (r.cMax - r.cMin < 1e-9) ? 1.0 : 1.0 - (d.costPerPlate() - r.cMin) / (r.cMax - r.cMin);
        const double nutritionScore =
            (r.pMax - r.pMin < 1e-9) ? 0.5 : (d.proteinG - r.pMin) / (r.pMax - r.pMin);
        out[d.name] = weights_.acceptance * acceptance + weights_.cost * costScore +
                      weights_.nutrition * nutritionScore;
    }
    return out;
}

bool Optimizer::bestMenu(const std::string& meal, double budget, int minCalories,
                         const std::set<std::string>& excluded, MenuOption& out) const {
    const Structure st = structureFor(meal);
    auto candidates = [&](const std::string& category) {
        std::vector<const Dish*> v;
        for (const auto& d : tracker_.dishes())
            if (d.category == category && d.servedAt(meal) && !excluded.count(d.name)) v.push_back(&d);
        return v;
    };

    std::vector<std::vector<const Dish*>> options;
    for (const auto& cat : st.required) {
        auto v = candidates(cat);
        if (v.empty()) return false;  // a required category has no dish available
        options.push_back(v);
    }
    for (const auto& cat : st.optional) {
        auto v = candidates(cat);
        if (v.empty()) continue;
        v.insert(v.begin(), nullptr);  // "no dish from this category" is allowed
        options.push_back(v);
    }

    const auto scores = dishScores();
    SearchState state{options, scores, budget, minCalories, {}, MenuOption(), false};
    search(state, 0, 0.0, 0, 0.0, 0.0);
    if (!state.found) return false;
    out = state.best;
    return true;
}

std::vector<std::pair<std::string, MenuOption>> Optimizer::weeklyPlan(
    const std::string& meal, const std::string& startDate, double budget, int minCalories) const {
    std::vector<std::pair<std::string, MenuOption>> plan;
    std::vector<std::set<std::string>> usedPerDay;
    for (int day = 0; day < 7; ++day) {
        const std::string date = utils::addDays(startDate, day);
        MenuOption chosen;
        // First avoid everything used in the last 2 days, then only yesterday's, then no limit.
        for (int window = 2; window >= 0; --window) {
            std::set<std::string> excluded;
            for (int back = 1; back <= window && back <= static_cast<int>(usedPerDay.size()); ++back)
                for (const auto& n : usedPerDay[usedPerDay.size() - static_cast<size_t>(back)])
                    excluded.insert(n);
            if (bestMenu(meal, budget, minCalories, excluded, chosen)) break;
            chosen = MenuOption();  // nothing feasible with this restriction
        }
        // Only the "defining" dishes (main item, staple, dal, gravy) are rotated; tea, salad etc. may repeat.
        std::set<std::string> used;
        for (const auto& d : chosen.dishes)
            if (d.category == "Main" || d.category == "Staple" || d.category == "Dal" || d.category == "Gravy")
                used.insert(d.name);
        usedPerDay.push_back(used);
        plan.emplace_back(date, chosen);
    }
    return plan;
}

CookPlan Optimizer::planQuantities(const std::string& date, const std::string& meal,
                                   const std::vector<std::string>& dishNames) const {
    CookPlan plan;
    plan.date = date;
    plan.meal = meal;
    const Forecast f = Forecaster(tracker_).forecast(meal, date);
    plan.forecastHeads = static_cast<int>(std::lround(f.value));
    plan.method = f.method;

    std::map<std::string, DishStats> stats;
    for (const auto& s : Analyzer(tracker_).dishStats()) stats[s.dish] = s;

    for (const auto& name : dishNames) {
        const Dish* d = tracker_.findDish(name);
        if (!d) {
            plan.unknownDishes.push_back(name);
            continue;
        }
        CookPlanItem item;
        item.dish = d->name;
        auto it = stats.find(d->name);
        if (it != stats.end() && it->second.hasPerHead) {
            item.hasHistory = true;
            item.perHeadKg = it->second.servedPerHeadKg;  // real demand, not what was cooked
            item.plannedKg = plan.forecastHeads * item.perHeadKg * (1.0 + buffer_);
            item.baselineKg = it->second.avgCookedKg;
        } else {
            item.perHeadKg = d->portionG / 1000.0;  // no history: fall back to the standard portion
            item.plannedKg = plan.forecastHeads * item.perHeadKg * (1.0 + buffer_);
            item.baselineKg = item.plannedKg;       // nothing to compare with, so no saving claimed
        }
        item.savedKg = item.baselineKg - item.plannedKg;
        item.savedCost = item.savedKg * d->costPerKg;
        plan.totalPlannedKg += item.plannedKg;
        plan.totalBaselineKg += item.baselineKg;
        plan.totalSavedKg += item.savedKg;
        plan.totalSavedCost += item.savedCost;
        plan.items.push_back(item);
    }
    return plan;
}
