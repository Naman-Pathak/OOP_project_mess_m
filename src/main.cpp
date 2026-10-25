// Mess Food Waste Tracking and Meal Optimization - console application.
//
//   ./mess_tracker                     interactive menu (data read from ./data)
//   ./mess_tracker --data DIR          use another data folder
//   ./mess_tracker --report [--out F]  write the full report and exit (no menu)
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "Analyzer.h"
#include "FileHandler.h"
#include "Forecaster.h"
#include "Optimizer.h"
#include "Report.h"
#include "Tracker.h"
#include "Utils.h"

namespace {

// ---------- input helpers (all return false when input ends, e.g. Ctrl+D) ----------
bool ask(const std::string& label, std::string& out) {
    std::cout << label << ": ";
    if (!std::getline(std::cin, out)) return false;
    out = utils::trim(out);
    return true;
}

bool askNumber(const std::string& label, double& out) {
    std::string s;
    while (ask(label, s)) {
        if (utils::parseDouble(s, out)) return true;
        std::cout << "  Please type a number.\n";
    }
    return false;
}

bool askInt(const std::string& label, int& out) {
    std::string s;
    while (ask(label, s)) {
        if (utils::parseInt(s, out)) return true;
        std::cout << "  Please type a whole number.\n";
    }
    return false;
}

std::string askMeal() {
    std::string s;
    while (ask("Meal (Breakfast/Lunch/Dinner)", s)) {
        const std::string m = utils::canonicalMeal(s);
        if (!m.empty()) return m;
        std::cout << "  Not a valid meal.\n";
    }
    return "";
}

std::string askDate(const std::string& label, const std::string& fallback = "") {
    std::string s;
    while (ask(label, s)) {
        if (s.empty() && !fallback.empty()) return fallback;
        if (utils::isValidDate(s)) return s;
        std::cout << "  Use a real date in YYYY-MM-DD format.\n";
    }
    return "";
}

// ---------- menu actions ----------
void showDishes(const Tracker& t) {
    std::cout << "\n" << std::left;
    std::cout.width(18); std::cout << "Dish";
    std::cout.width(10); std::cout << "Category";
    std::cout.width(18); std::cout << "Meals";
    std::cout << "Rs/plate  kcal\n" << utils::repeat('-', 62) << "\n";
    for (const auto& d : t.dishes()) {
        std::string meals;
        for (size_t i = 0; i < d.meals.size(); ++i) meals += (i ? "/" : "") + d.meals[i];
        std::cout.width(18); std::cout << d.name;
        std::cout.width(10); std::cout << d.category;
        std::cout.width(18); std::cout << meals;
        std::cout << utils::fmt(d.costPerPlate(), 1) << "      " << d.calories << "\n";
    }
}

void addWaste(Tracker& t) {
    WasteRecord r;
    r.date = askDate("Date (YYYY-MM-DD)");
    if (r.date.empty()) return;
    r.meal = askMeal();
    if (r.meal.empty() || !ask("Dish name", r.dish)) return;
    if (!askNumber("Cooked quantity (kg)", r.cookedKg)) return;
    if (!askNumber("Leftover, never served (kg)", r.leftoverKg)) return;
    if (!askNumber("Plate waste, thrown from plates (kg)", r.plateKg)) return;
    std::string err;
    std::cout << (t.addWaste(r, err) ? "Record added.\n" : "Not saved: " + err + "\n");
}

void addAttendance(Tracker& t) {
    Attendance a;
    a.date = askDate("Date (YYYY-MM-DD)");
    if (a.date.empty()) return;
    a.meal = askMeal();
    if (a.meal.empty()) return;
    if (!askInt("Expected headcount (what the kitchen planned for)", a.expected)) return;
    if (!askInt("Actual headcount (students who ate)", a.actual)) return;
    std::string err;
    std::cout << (t.addAttendance(a, err) ? "Attendance saved.\n" : "Not saved: " + err + "\n");
}

void viewRecent(const Tracker& t) {
    int n = 15;
    askInt("How many recent records", n);
    std::vector<size_t> idx;
    const size_t total = t.waste().size();
    const size_t start = (n > 0 && static_cast<size_t>(n) < total) ? total - static_cast<size_t>(n) : 0;
    for (size_t i = start; i < total; ++i) idx.push_back(i);
    Report::printRecords(std::cout, t, idx);
}

void deleteRecord(Tracker& t) {
    int idx = 0;
    if (!askInt("Record number to delete (see the # column)", idx)) return;
    if (idx < 0 || !t.removeWaste(static_cast<size_t>(idx))) { std::cout << "No such record.\n"; return; }
    std::cout << "Record deleted.\n";
}

void showAnalysis(const Tracker& t) {
    Analyzer a(t);
    Report::printSummaryTable(std::cout, "Overall waste", "Scope", {{"All meals", a.overall()}});
    std::vector<std::pair<std::string, Summary>> rows;
    const auto meals = a.byMeal();
    for (const auto& m : utils::mealNames())
        if (meals.count(m)) rows.emplace_back(m, meals.at(m));
    Report::printSummaryTable(std::cout, "Waste by meal", "Meal", rows);
    rows.clear();
    for (const auto& kv : a.byWeekday()) rows.emplace_back(utils::weekdayName(kv.first), kv.second);
    Report::printSummaryTable(std::cout, "Waste by weekday", "Weekday", rows);
}

void showTopWasted(const Tracker& t) {
    Analyzer a(t);
    Report::printSummaryTable(std::cout, "Top 10 dishes by waste (kg)", "Dish", a.topWasted(10, false));
    Report::printSummaryTable(std::cout, "Top 10 dishes by cost of waste", "Dish", a.topWasted(10, true));
    std::vector<std::pair<std::string, double>> bars;
    for (const auto& kv : a.topWasted(10, false)) bars.emplace_back(kv.first, kv.second.wastePct);
    Report::printBarChart(std::cout, "Waste % of those dishes", bars, " %");
}

void forecastHeadcount(const Tracker& t) {
    const std::string date = askDate("Date to forecast (YYYY-MM-DD, Enter = day after last data)",
                                     t.lastDate().empty() ? "" : utils::addDays(t.lastDate(), 1));
    if (date.empty()) return;
    Forecaster f(t);
    std::cout << "\n";
    for (const auto& m : utils::mealNames()) Report::printForecast(std::cout, m, date, f.forecast(m, date));
}

void cookingPlan(const Tracker& t) {
    const std::string date = askDate("Date (YYYY-MM-DD, Enter = day after last data)",
                                     t.lastDate().empty() ? "" : utils::addDays(t.lastDate(), 1));
    if (date.empty()) return;
    const std::string meal = askMeal();
    if (meal.empty()) return;
    std::vector<std::string> names = t.dishesFor(utils::weekdayOf(date), meal);
    std::string list;
    if (!names.empty())
        std::cout << "Scheduled menu: " << Report::describeMeal(t.menuFor(utils::weekdayOf(date), meal)) << "\n";
    if (!ask(names.empty() ? "Dishes on the menu, separated by commas"
                           : "Dishes, separated by commas (Enter = the scheduled menu above)", list)) return;
    if (!list.empty()) names = utils::split(list, ',');
    if (names.empty()) { std::cout << "No dishes given and no scheduled menu for that day.\n"; return; }
    Optimizer opt(t);
    Report::printCookPlan(std::cout, opt.planQuantities(date, meal, names));
}

void suggestMenu(const Tracker& t) {
    const std::string meal = askMeal();
    if (meal.empty()) return;
    double budget = Optimizer::defaultBudget(meal);
    int minCal = Optimizer::defaultMinCalories(meal);
    std::string s;
    if (ask("Budget per plate in Rs (Enter = " + utils::fmt(budget, 0) + ")", s) && !s.empty())
        utils::parseDouble(s, budget);
    if (ask("Minimum calories (Enter = " + std::to_string(minCal) + ")", s) && !s.empty())
        utils::parseInt(s, minCal);
    Optimizer opt(t);
    MenuOption menu;
    if (opt.bestMenu(meal, budget, minCal, {}, menu)) Report::printMenu(std::cout, "\nBest menu for " + meal, menu);
    else std::cout << "No combination of dishes fits that budget and calorie minimum.\n";
}

void weeklyMenu(const Tracker& t) {
    const std::string meal = askMeal();
    if (meal.empty()) return;
    const std::string start = askDate("Start date (YYYY-MM-DD, Enter = day after last data)",
                                      t.lastDate().empty() ? "" : utils::addDays(t.lastDate(), 1));
    if (start.empty()) return;
    Optimizer opt(t);
    Report::printWeeklyPlan(std::cout, meal,
                            opt.weeklyPlan(meal, start, Optimizer::defaultBudget(meal), Optimizer::defaultMinCalories(meal)));
}

void showMenu(const Tracker& t) {
    std::string s;
    if (!ask("Weekday (e.g. Monday, Enter = whole week)", s)) return;
    if (s.empty()) { Report::printWeeklyMenu(std::cout, t); return; }
    const int wd = utils::weekdayFromName(s);
    if (wd < 0) { std::cout << "Not a weekday.\n"; return; }
    Report::printMenuForDay(std::cout, t, wd);
}

// Only the two files the program can change are saved. dishes.csv and menu.csv are edited by hand,
// so they are left untouched (this also keeps the comments written in them).
void saveAll(const Tracker& t, const std::string& dir) {
    FileHandler::saveWaste(dir + "/waste_log.csv", t.waste());
    FileHandler::saveAttendance(dir + "/attendance.csv", t.attendance());
}

void printMenuScreen() {
    std::cout << "\n===== MESS FOOD WASTE TRACKER =====\n"
                 " 1  Show dishes\n"
                 " 2  Add a waste record\n"
                 " 3  Add attendance (headcount)\n"
                 " 4  View recent records\n"
                 " 5  Search records by date\n"
                 " 6  Search records by dish\n"
                 " 7  Delete a record\n"
                 " 8  Waste analysis (overall / meal / weekday)\n"
                 " 9  Top wasted dishes\n"
                 "10  Dish statistics\n"
                 "11  Forecast headcount\n"
                 "12  Cooking quantity plan\n"
                 "13  Suggest best menu (optimizer)\n"
                 "14  Suggested weekly menu plan (optimizer)\n"
                 "15  Generate full report file\n"
                 "16  Show the mess menu\n"
                 " 0  Save and exit\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string dataDir = "data";
    std::string reportPath = "reports/weekly_summary.txt";
    bool reportOnly = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--data" && i + 1 < argc) dataDir = argv[++i];
        else if (arg == "--out" && i + 1 < argc) reportPath = argv[++i];
        else if (arg == "--report") reportOnly = true;
        else {
            std::cerr << "Usage: mess_tracker [--data DIR] [--report] [--out FILE]\n";
            return 1;
        }
    }

    Tracker tracker;
    try {
        std::vector<MenuEntry> menu;
        if (std::filesystem::exists(dataDir + "/menu.csv")) menu = FileHandler::loadMenu(dataDir + "/menu.csv");
        tracker = Tracker(FileHandler::loadDishes(dataDir + "/dishes.csv"),
                          FileHandler::loadWaste(dataDir + "/waste_log.csv"),
                          FileHandler::loadAttendance(dataDir + "/attendance.csv"), menu);
    } catch (const std::exception& e) {
        std::cerr << "Could not load data: " << e.what() << "\n"
                  << "Tip: run from the project folder, or create sample data with ./generate_sample_data\n";
        return 1;
    }

    if (reportOnly) {
        for (const auto& problem : tracker.menuProblems()) std::cerr << "Menu warning: " << problem << "\n";
        std::string err;
        if (!Report::writeFullReport(reportPath, tracker, err)) { std::cerr << err << "\n"; return 1; }
        std::cout << "Report written to " << reportPath << "\n";
        return 0;
    }

    std::cout << "Loaded " << tracker.dishes().size() << " dishes, " << tracker.menu().size() << " menu entries, "
              << tracker.waste().size() << " waste records, " << tracker.attendance().size() << " attendance rows.\n";
    for (const auto& problem : tracker.menuProblems()) std::cout << "Menu warning: " << problem << "\n";
    std::string choice;
    while (true) {
        printMenuScreen();
        if (!ask("Choose", choice)) break;  // input closed
        int c = -1;
        if (!utils::parseInt(choice, c)) { std::cout << "Please type a number from the menu.\n"; continue; }
        if (c == 0) break;
        std::string text;
        switch (c) {
            case 1: showDishes(tracker); break;
            case 2: addWaste(tracker); break;
            case 3: addAttendance(tracker); break;
            case 4: viewRecent(tracker); break;
            case 5: {
                const std::string d = askDate("Date (YYYY-MM-DD)");
                if (!d.empty()) Report::printRecords(std::cout, tracker, tracker.findByDate(d));
                break;
            }
            case 6:
                if (ask("Dish name", text)) Report::printRecords(std::cout, tracker, tracker.findByDish(text));
                break;
            case 7: deleteRecord(tracker); break;
            case 8: showAnalysis(tracker); break;
            case 9: showTopWasted(tracker); break;
            case 10: Report::printDishStats(std::cout, Analyzer(tracker).dishStats()); break;
            case 11: forecastHeadcount(tracker); break;
            case 12: cookingPlan(tracker); break;
            case 13: suggestMenu(tracker); break;
            case 14: weeklyMenu(tracker); break;
            case 16: showMenu(tracker); break;
            case 15: {
                std::string err;
                if (Report::writeFullReport(reportPath, tracker, err)) std::cout << "Report written to " << reportPath << "\n";
                else std::cout << err << "\n";
                break;
            }
            default: std::cout << "Please choose a number from the menu.\n";
        }
    }
    try {
        saveAll(tracker, dataDir);
        std::cout << "\nData saved. Goodbye!\n";
    } catch (const std::exception& e) {
        std::cerr << "Could not save: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
