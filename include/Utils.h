#pragma once
// Small helper functions used everywhere: string handling, date maths, validation.
#include <string>
#include <vector>

namespace utils {

// ---- strings ----
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s, char delim);
std::string toLower(std::string s);
std::string fmt(double value, int precision = 2);
std::string repeat(char c, int count);

// ---- numbers ----
bool parseDouble(const std::string& s, double& out);
bool parseInt(const std::string& s, int& out);

// ---- dates (format YYYY-MM-DD) ----
bool isValidDate(const std::string& date);
long daysFromCivil(int y, int m, int d);          // days since 1970-01-01
void civilFromDays(long z, int& y, int& m, int& d);
long dateToDays(const std::string& date);
std::string daysToDate(long days);
std::string addDays(const std::string& date, int n);
int weekdayOf(const std::string& date);           // 0 = Monday ... 6 = Sunday
std::string weekdayName(int weekday);
int weekdayFromName(const std::string& name);    // "Monday" or "mon" -> 0; -1 if invalid

// ---- meals ----
const std::vector<std::string>& mealNames();      // Breakfast, Lunch, Dinner
std::string canonicalMeal(const std::string& input);  // "" if not a valid meal

}  // namespace utils
