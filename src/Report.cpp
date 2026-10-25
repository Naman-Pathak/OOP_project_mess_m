#include "Report.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>

#include "Utils.h"

namespace Report {

namespace {
void line(std::ostream& os, char c, int n) { os << utils::repeat(c, n) << '\n'; }
void heading(std::ostream& os, const std::string& title) {
    os << '\n' << title << '\n';
    line(os, '=', static_cast<int>(title.size()));
}
}  // namespace

void printSummaryTable(std::ostream& os, const std::string& title, const std::string& labelHeader,
                       const std::vector<std::pair<std::string, Summary>>& rows) {
    heading(os, title);
    os << std::left << std::setw(26) << labelHeader << std::right << std::setw(11) << "Cooked kg"
       << std::setw(11) << "Leftover" << std::setw(9) << "Plate" << std::setw(11) << "Waste kg"
       << std::setw(9) << "Waste %" << std::setw(12) << "Cost (Rs)" << '\n';
    line(os, '-', 89);
    for (const auto& r : rows) {
        const Summary& s = r.second;
        os << std::left << std::setw(26) << r.first << std::right << std::fixed << std::setprecision(1)
           << std::setw(11) << s.cookedKg << std::setw(11) << s.leftoverKg << std::setw(9) << s.plateKg
           << std::setw(11) << s.wasteKg << std::setw(9) << s.wastePct << std::setprecision(0)
           << std::setw(12) << s.costOfWaste << '\n';
    }
}

void printBarChart(std::ostream& os, const std::string& title,
                   const std::vector<std::pair<std::string, double>>& rows, const std::string& unit,
                   int width) {
    heading(os, title);
    double maxValue = 0;
    for (const auto& r : rows) maxValue = std::max(maxValue, r.second);
    for (const auto& r : rows) {
        const int len = maxValue > 0 ? static_cast<int>(std::lround(r.second / maxValue * width)) : 0;
        os << std::left << std::setw(26) << r.first << ' ' << utils::repeat('#', len) << ' '
           << utils::fmt(r.second, 1) << unit << '\n';
    }
}

void printDishStats(std::ostream& os, const std::vector<DishStats>& stats) {
    heading(os, "Dish statistics (what students really eat)");
    os << std::left << std::setw(26) << "Dish" << std::right << std::setw(6) << "Days" << std::setw(11)
       << "Cooked kg" << std::setw(9) << "Waste %" << std::setw(12) << "Eaten %" << std::setw(14)
       << "Cooked g/head" << std::setw(14) << "Served g/head" << '\n';
    line(os, '-', 92);
    for (const auto& s : stats) {
        os << std::left << std::setw(26) << s.dish << std::right << std::setw(6) << s.records << std::fixed
           << std::setprecision(1) << std::setw(11) << s.cookedKg << std::setw(9) << s.wastePct
           << std::setw(12) << s.acceptance * 100.0;
        if (s.hasPerHead)
            os << std::setw(14) << s.cookedPerHeadKg * 1000.0 << std::setw(14) << s.servedPerHeadKg * 1000.0;
        else
            os << std::setw(14) << "n/a" << std::setw(14) << "n/a";
        os << '\n';
    }
}

void printForecast(std::ostream& os, const std::string& meal, const std::string& date, const Forecast& f) {
    os << meal << " on " << date << " (" << utils::weekdayName(utils::weekdayOf(date)) << "): about "
       << static_cast<long>(std::lround(f.value)) << " students  [" << f.method;
    if (f.mae >= 0) os << ", average past error " << utils::fmt(f.mae, 1) << " students";
    os << "]\n";
}

void printCookPlan(std::ostream& os, const CookPlan& p) {
    heading(os, "Cooking plan: " + p.meal + " on " + p.date);
    os << "Forecast headcount: " << p.forecastHeads << "  (" << p.method << ")\n\n";
    os << std::left << std::setw(26) << "Dish" << std::right << std::setw(10) << "g/head" << std::setw(11)
       << "Cook kg" << std::setw(12) << "Usual kg" << std::setw(10) << "Saved kg" << std::setw(11)
       << "Saved Rs" << '\n';
    line(os, '-', 80);
    for (const auto& i : p.items) {
        os << std::left << std::setw(26) << i.dish << std::right << std::fixed << std::setprecision(0)
           << std::setw(10) << i.perHeadKg * 1000.0 << std::setprecision(1) << std::setw(11) << i.plannedKg
           << std::setw(12) << i.baselineKg << std::setw(10) << i.savedKg << std::setprecision(0)
           << std::setw(11) << i.savedCost << (i.hasHistory ? "" : "  (no history)") << '\n';
    }
    line(os, '-', 80);
    os << std::left << std::setw(26) << "TOTAL" << std::right << std::fixed << std::setw(10) << ""
       << std::setprecision(1) << std::setw(11) << p.totalPlannedKg << std::setw(12) << p.totalBaselineKg
       << std::setw(10) << p.totalSavedKg << std::setprecision(0) << std::setw(11) << p.totalSavedCost << '\n';
    for (const auto& u : p.unknownDishes) os << "Note: dish '" << u << "' is not in the dish list.\n";
}

void printMenu(std::ostream& os, const std::string& title, const MenuOption& m) {
    os << title << '\n';
    if (m.dishes.empty()) {
        os << "  No menu fits the budget and calorie limits.\n";
        return;
    }
    for (const auto& d : m.dishes)
        os << "  - " << std::left << std::setw(26) << d.name << std::setw(10) << d.category << "Rs "
           << std::right << std::setw(5) << utils::fmt(d.costPerPlate(), 1) << "  " << std::setw(4)
           << d.calories << " kcal\n";
    os << "  Total: Rs " << utils::fmt(m.costPerPlate, 1) << " per plate, " << m.calories << " kcal, "
       << utils::fmt(m.protein, 1) << " g protein, score " << utils::fmt(m.score, 2) << '\n';
}

void printWeeklyPlan(std::ostream& os, const std::string& meal,
                     const std::vector<std::pair<std::string, MenuOption>>& plan) {
    heading(os, "Weekly menu plan: " + meal);
    for (const auto& day : plan) {
        os << day.first << " " << std::left << std::setw(10) << utils::weekdayName(utils::weekdayOf(day.first));
        if (day.second.dishes.empty()) {
            os << "no feasible menu\n";
            continue;
        }
        for (size_t i = 0; i < day.second.dishes.size(); ++i)
            os << (i ? ", " : "") << day.second.dishes[i].name;
        os << "   (Rs " << utils::fmt(day.second.costPerPlate, 1) << ", " << day.second.calories << " kcal)\n";
    }
}

namespace {
std::string join(const std::vector<std::string>& v, const std::string& sep) {
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) s += (i ? sep : "") + v[i];
    return s;
}

// Text for one slot: fixed dishes joined with " + ", choice dishes as "(A / B)".
std::string slotText(const std::vector<std::string>& fixed, const std::vector<std::string>& choice) {
    std::string s = join(fixed, " + ");
    if (!choice.empty()) {
        const std::string c = join(choice, " / ");
        s += (s.empty() ? "" : " + ") + (choice.size() > 1 ? "(" + c + ")" : c);
    }
    return s;
}

struct SlotItems {
    std::vector<std::string> fixed, choice;
};

// Groups one meal's entries by slot, keeping the order of the file.
void groupSlots(const std::vector<MenuEntry>& entries, std::vector<std::string>& order,
                std::map<std::string, SlotItems>& items) {
    for (const auto& e : entries) {
        if (!items.count(e.slot)) order.push_back(e.slot);
        (e.choice ? items[e.slot].choice : items[e.slot].fixed).push_back(e.dish);
    }
}
}  // namespace

std::string describeMeal(const std::vector<MenuEntry>& entries) {
    std::vector<std::string> order;
    std::map<std::string, SlotItems> items;
    groupSlots(entries, order, items);
    std::string s;
    for (size_t i = 0; i < order.size(); ++i) {
        const SlotItems& it = items[order[i]];
        s += (i ? " | " : "") + order[i] + ": " + slotText(it.fixed, it.choice);
    }
    return s;
}

void printMenuForDay(std::ostream& os, const Tracker& t, int weekday) {
    os << '\n' << utils::weekdayName(weekday) << '\n';
    for (const auto& meal : utils::mealNames()) {
        std::vector<std::string> order;
        std::map<std::string, SlotItems> items;
        groupSlots(t.menuFor(weekday, meal), order, items);
        if (order.empty()) {
            os << "  " << std::left << std::setw(10) << meal << "(nothing on the menu)\n";
            continue;
        }
        for (size_t i = 0; i < order.size(); ++i) {
            const SlotItems& it = items[order[i]];
            os << "  " << std::left << std::setw(10) << (i == 0 ? meal : "") << std::setw(11) << order[i] << ": "
               << slotText(it.fixed, it.choice) << '\n';
        }
    }
}

void printWeeklyMenu(std::ostream& os, const Tracker& t) {
    heading(os, "Mess weekly menu");
    if (t.menu().empty()) {
        os << "No menu loaded (data/menu.csv is missing or empty).\n";
        return;
    }
    os << "(choice dishes are written as (A / B): students pick one)\n";
    for (int d = 0; d < 7; ++d) printMenuForDay(os, t, d);
}

void printRecords(std::ostream& os, const Tracker& t, const std::vector<size_t>& indices) {
    os << std::left << std::setw(5) << "#" << std::setw(12) << "Date" << std::setw(11) << "Meal"
       << std::setw(26) << "Dish" << std::right << std::setw(9) << "Cooked" << std::setw(10) << "Leftover"
       << std::setw(8) << "Plate" << std::setw(9) << "Waste %" << '\n';
    line(os, '-', 90);
    for (size_t idx : indices) {
        const WasteRecord& r = t.waste()[idx];
        os << std::left << std::setw(5) << idx << std::setw(12) << r.date << std::setw(11) << r.meal
           << std::setw(26) << r.dish << std::right << std::fixed << std::setprecision(2) << std::setw(9)
           << r.cookedKg << std::setw(10) << r.leftoverKg << std::setw(8) << r.plateKg << std::setprecision(1)
           << std::setw(9) << r.wastePercent() << '\n';
    }
    if (indices.empty()) os << "(no records)\n";
}

bool writeFullReport(const std::string& path, const Tracker& tracker, std::string& err) {
    if (tracker.waste().empty()) {
        err = "There are no waste records yet, so there is nothing to report.";
        return false;
    }
    std::error_code ec;
    const std::filesystem::path p(path);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path(), ec);
    std::ofstream out(path);
    if (!out) {
        err = "Cannot write to " + path;
        return false;
    }

    Analyzer analyzer(tracker);
    Forecaster forecaster(tracker);
    Optimizer optimizer(tracker);

    out << "MESS FOOD WASTE TRACKING AND MEAL OPTIMIZATION - REPORT\n";
    line(out, '=', 55);
    std::string first = tracker.waste().front().date;
    for (const auto& r : tracker.waste()) first = std::min(first, r.date);
    out << "Data covers " << first << " to " << tracker.lastDate() << ", " << tracker.waste().size()
        << " waste records, " << tracker.attendance().size() << " attendance rows.\n";

    // ---- 1. Where is the waste? ----
    printSummaryTable(out, "1. Overall waste", "Scope", {{"All meals", analyzer.overall()}});

    std::vector<std::pair<std::string, Summary>> rows;
    const auto meals = analyzer.byMeal();
    for (const auto& m : utils::mealNames())
        if (meals.count(m)) rows.emplace_back(m, meals.at(m));
    printSummaryTable(out, "2. Waste by meal", "Meal", rows);

    rows.clear();
    for (const auto& kv : analyzer.byWeekday()) rows.emplace_back(utils::weekdayName(kv.first), kv.second);
    printSummaryTable(out, "3. Waste by weekday", "Weekday", rows);

    printSummaryTable(out, "4. Ten dishes with the most waste (kg)", "Dish", analyzer.topWasted(10, false));
    printSummaryTable(out, "5. Five dishes with the highest cost of waste", "Dish", analyzer.topWasted(5, true));

    auto stats = analyzer.dishStats();
    std::sort(stats.begin(), stats.end(), [](const DishStats& a, const DishStats& b) { return a.wastePct > b.wastePct; });
    std::vector<std::pair<std::string, double>> bars;
    for (size_t i = 0; i < stats.size() && i < 10; ++i) bars.emplace_back(stats[i].dish, stats[i].wastePct);
    printBarChart(out, "6. Ten highest waste percentages", bars, " %");
    std::sort(stats.begin(), stats.end(), [](const DishStats& a, const DishStats& b) { return a.dish < b.dish; });
    printDishStats(out, stats);

    // ---- 2. What to do tomorrow? ----
    const std::string next = utils::addDays(tracker.lastDate(), 1);
    heading(out, "7. Headcount forecast for " + next);
    for (const auto& m : utils::mealNames()) printForecast(out, m, next, forecaster.forecast(m, next));

    const int nextWeekday = utils::weekdayOf(next);
    heading(out, "8. Cooking plan for the scheduled menu on " + next + " (" + utils::weekdayName(nextWeekday) + ")");
    double totalSaved = 0;
    for (const auto& m : utils::mealNames()) {
        std::vector<std::string> names = tracker.dishesFor(nextWeekday, m);
        out << '\n' << m << ": ";
        if (names.empty()) {
            // No weekly menu loaded: fall back to the optimizer's best menu.
            MenuOption fallback;
            if (!optimizer.bestMenu(m, Optimizer::defaultBudget(m), Optimizer::defaultMinCalories(m), {}, fallback)) {
                out << "no menu available.\n";
                continue;
            }
            for (const auto& d : fallback.dishes) names.push_back(d.name);
            out << "(no weekly menu loaded - using the optimizer's menu)\n";
        } else {
            out << describeMeal(tracker.menuFor(nextWeekday, m)) << '\n';
        }
        const CookPlan plan = optimizer.planQuantities(next, m, names);
        printCookPlan(out, plan);
        totalSaved += plan.totalSavedCost;
    }
    out << "\nEstimated saving for this day if the plans are followed: Rs " << utils::fmt(totalSaved, 0) << '\n';

    heading(out, "9. Optimizer suggestion: best menu within budget (compare with the scheduled menu)");
    for (const auto& m : utils::mealNames()) {
        MenuOption menu;
        const bool ok = optimizer.bestMenu(m, Optimizer::defaultBudget(m), Optimizer::defaultMinCalories(m), {}, menu);
        out << '\n';
        printMenu(out, m + " (budget Rs " + utils::fmt(Optimizer::defaultBudget(m), 0) + " per plate, at least " +
                           std::to_string(Optimizer::defaultMinCalories(m)) + " kcal)", ok ? menu : MenuOption());
    }

    printWeeklyMenu(out, tracker);
    return true;
}

}  // namespace Report
