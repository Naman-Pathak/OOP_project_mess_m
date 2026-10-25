#pragma once
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "Analyzer.h"
#include "Forecaster.h"
#include "Optimizer.h"
#include "Tracker.h"

// Printing helpers. Every function takes an std::ostream, so the same code
// prints to the screen (std::cout) and to a report file (std::ofstream).
namespace Report {

void printSummaryTable(std::ostream& os, const std::string& title, const std::string& labelHeader,
                       const std::vector<std::pair<std::string, Summary>>& rows);
void printBarChart(std::ostream& os, const std::string& title,
                   const std::vector<std::pair<std::string, double>>& rows,
                   const std::string& unit, int width = 40);
void printDishStats(std::ostream& os, const std::vector<DishStats>& stats);
void printForecast(std::ostream& os, const std::string& meal, const std::string& date,
                   const Forecast& f);
void printCookPlan(std::ostream& os, const CookPlan& plan);
void printMenu(std::ostream& os, const std::string& title, const MenuOption& menu);
void printWeeklyPlan(std::ostream& os, const std::string& meal,
                     const std::vector<std::pair<std::string, MenuOption>>& plan);
// The mess weekly menu (from menu.csv), grouped by slot.
void printMenuForDay(std::ostream& os, const Tracker& t, int weekday);
void printWeeklyMenu(std::ostream& os, const Tracker& t);
// One-line description of what is served at a meal, e.g. "Common: Rice + Roti | Gravy: (Egg Curry / Mix Veg)".
std::string describeMeal(const std::vector<MenuEntry>& entries);
void printRecords(std::ostream& os, const Tracker& t, const std::vector<size_t>& indices);

// Builds the complete analysis report and saves it to 'path'.
bool writeFullReport(const std::string& path, const Tracker& tracker, std::string& err);

}  // namespace Report
