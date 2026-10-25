#include "Tracker.h"

#include <utility>

#include "Utils.h"

Tracker::Tracker(std::vector<Dish> dishes, std::vector<WasteRecord> waste,
                 std::vector<Attendance> attendance, std::vector<MenuEntry> menu)
    : dishes_(std::move(dishes)),
      waste_(std::move(waste)),
      attendance_(std::move(attendance)),
      menu_(std::move(menu)) {}

const Dish* Tracker::findDish(const std::string& name) const {
    const std::string key = utils::toLower(utils::trim(name));
    for (const auto& d : dishes_)
        if (utils::toLower(d.name) == key) return &d;
    return nullptr;
}

bool Tracker::addDish(const Dish& dish, std::string& err) {
    if (dish.name.empty()) { err = "Dish name cannot be empty."; return false; }
    if (findDish(dish.name)) { err = "A dish with this name already exists."; return false; }
    if (dish.costPerKg <= 0 || dish.portionG <= 0) { err = "Cost and portion must be positive."; return false; }
    if (dish.meals.empty()) { err = "Choose at least one meal."; return false; }
    dishes_.push_back(dish);
    return true;
}

bool Tracker::addWaste(WasteRecord r, std::string& err) {
    if (!utils::isValidDate(r.date)) { err = "Date must be a real date in YYYY-MM-DD format."; return false; }
    r.meal = utils::canonicalMeal(r.meal);
    if (r.meal.empty()) { err = "Meal must be Breakfast, Lunch or Dinner."; return false; }
    const Dish* d = findDish(r.dish);
    if (!d) { err = "Unknown dish '" + r.dish + "'. Add it to the dish list first."; return false; }
    r.dish = d->name;  // store the official spelling
    if (r.cookedKg <= 0) { err = "Cooked quantity must be greater than zero."; return false; }
    if (r.leftoverKg < 0 || r.plateKg < 0) { err = "Quantities cannot be negative."; return false; }
    if (r.leftoverKg + r.plateKg > r.cookedKg + 1e-9) {
        err = "Leftover + plate waste cannot be more than the quantity cooked.";
        return false;
    }
    waste_.push_back(r);
    return true;
}

bool Tracker::removeWaste(size_t index) {
    if (index >= waste_.size()) return false;
    waste_.erase(waste_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool Tracker::addAttendance(Attendance a, std::string& err) {
    if (!utils::isValidDate(a.date)) { err = "Date must be a real date in YYYY-MM-DD format."; return false; }
    a.meal = utils::canonicalMeal(a.meal);
    if (a.meal.empty()) { err = "Meal must be Breakfast, Lunch or Dinner."; return false; }
    if (a.expected < 0 || a.actual < 0) { err = "Headcounts cannot be negative."; return false; }
    for (auto& existing : attendance_) {
        if (existing.date == a.date && existing.meal == a.meal) {
            existing = a;  // update instead of duplicating
            return true;
        }
    }
    attendance_.push_back(a);
    return true;
}

std::vector<size_t> Tracker::findByDate(const std::string& date) const {
    std::vector<size_t> out;
    for (size_t i = 0; i < waste_.size(); ++i)
        if (waste_[i].date == date) out.push_back(i);
    return out;
}

std::vector<size_t> Tracker::findByDish(const std::string& dish) const {
    std::vector<size_t> out;
    const std::string key = utils::toLower(utils::trim(dish));
    for (size_t i = 0; i < waste_.size(); ++i)
        if (utils::toLower(waste_[i].dish) == key) out.push_back(i);
    return out;
}

int Tracker::actualHeadcount(const std::string& date, const std::string& meal) const {
    for (const auto& a : attendance_)
        if (a.date == date && a.meal == meal) return a.actual;
    return -1;
}

std::string Tracker::lastDate() const {
    std::string last;
    for (const auto& a : attendance_) if (a.date > last) last = a.date;
    for (const auto& r : waste_) if (r.date > last) last = r.date;
    return last;
}

std::vector<MenuEntry> Tracker::menuFor(int weekday, const std::string& meal) const {
    std::vector<MenuEntry> out;
    for (const auto& e : menu_)
        if (e.weekday == weekday && e.meal == meal) out.push_back(e);
    return out;
}

std::vector<std::string> Tracker::dishesFor(int weekday, const std::string& meal) const {
    std::vector<std::string> names;
    for (const auto& e : menuFor(weekday, meal)) {
        const Dish* d = findDish(e.dish);
        const std::string name = d ? d->name : e.dish;
        bool seen = false;
        for (const auto& n : names) if (n == name) seen = true;
        if (!seen) names.push_back(name);
    }
    return names;
}

std::vector<std::string> Tracker::menuProblems() const {
    std::vector<std::string> problems;
    auto add = [&problems](const std::string& msg) {
        for (const auto& p : problems) if (p == msg) return;
        problems.push_back(msg);
    };
    for (const auto& e : menu_) {
        const Dish* d = findDish(e.dish);
        if (!d) add("Menu uses '" + e.dish + "' but it is not in dishes.csv.");
        else if (!d->servedAt(e.meal))
            add("'" + d->name + "' is on the " + e.meal + " menu but dishes.csv does not list " + e.meal + " for it.");
    }
    return problems;
}
