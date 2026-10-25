#include "FileHandler.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "Utils.h"

namespace {

using Row = std::pair<int, std::vector<std::string>>;  // (line number, fields)

[[noreturn]] void fail(const std::string& path, int line, const std::string& msg) {
    std::ostringstream os;
    os << path;
    if (line > 0) os << " (line " << line << ")";
    os << ": " << msg;
    throw std::runtime_error(os.str());
}

// Reads a CSV file, skipping the header, blank lines and '#' comments.
std::vector<Row> readRows(const std::string& path, size_t expectedColumns) {
    std::ifstream in(path);
    if (!in) fail(path, 0, "cannot open file");
    std::vector<Row> rows;
    std::string line;
    int lineNo = 0;
    bool headerSeen = false;
    while (std::getline(in, line)) {
        ++lineNo;
        line = utils::trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (!headerSeen) { headerSeen = true; continue; }
        auto fields = utils::split(line, ',');
        if (fields.size() != expectedColumns)
            fail(path, lineNo, "expected " + std::to_string(expectedColumns) + " columns but found " +
                                   std::to_string(fields.size()));
        rows.emplace_back(lineNo, std::move(fields));
    }
    return rows;
}

double num(const std::string& path, int line, const std::string& s, const char* what, bool allowZero) {
    double v = 0;
    if (!utils::parseDouble(s, v)) fail(path, line, std::string("'") + s + "' is not a number (" + what + ")");
    if (v < 0 || (!allowZero && v == 0))
        fail(path, line, std::string(what) + " must be " + (allowZero ? "zero or more" : "greater than zero"));
    return v;
}

std::ofstream openForWrite(const std::string& path) {
    std::ofstream out(path);
    if (!out) fail(path, 0, "cannot open file for writing");
    return out;
}

}  // namespace

std::vector<Dish> FileHandler::loadDishes(const std::string& path) {
    std::vector<Dish> dishes;
    for (const auto& row : readRows(path, 7)) {
        const int ln = row.first;
        const auto& f = row.second;
        Dish d;
        d.name = f[0];
        d.category = f[1];
        d.meals = utils::split(f[2], ';');
        if (d.name.empty()) fail(path, ln, "dish name is empty");
        if (d.meals.empty()) fail(path, ln, "meal list is empty");
        d.costPerKg = num(path, ln, f[3], "cost_per_kg", false);
        d.portionG = num(path, ln, f[4], "portion_g", false);
        d.calories = static_cast<int>(num(path, ln, f[5], "calories", true));
        d.proteinG = num(path, ln, f[6], "protein_g", true);
        dishes.push_back(d);
    }
    return dishes;
}

std::vector<WasteRecord> FileHandler::loadWaste(const std::string& path) {
    std::vector<WasteRecord> out;
    for (const auto& row : readRows(path, 6)) {
        const int ln = row.first;
        const auto& f = row.second;
        WasteRecord r;
        r.date = f[0];
        if (!utils::isValidDate(r.date)) fail(path, ln, "invalid date '" + r.date + "' (use YYYY-MM-DD)");
        r.meal = utils::canonicalMeal(f[1]);
        if (r.meal.empty()) fail(path, ln, "invalid meal '" + f[1] + "'");
        r.dish = f[2];
        r.cookedKg = num(path, ln, f[3], "cooked_kg", false);
        r.leftoverKg = num(path, ln, f[4], "leftover_kg", true);
        r.plateKg = num(path, ln, f[5], "plate_waste_kg", true);
        if (r.leftoverKg + r.plateKg > r.cookedKg + 1e-6)
            fail(path, ln, "leftover + plate waste is more than the quantity cooked");
        out.push_back(r);
    }
    return out;
}

std::vector<Attendance> FileHandler::loadAttendance(const std::string& path) {
    std::vector<Attendance> out;
    for (const auto& row : readRows(path, 4)) {
        const int ln = row.first;
        const auto& f = row.second;
        Attendance a;
        a.date = f[0];
        if (!utils::isValidDate(a.date)) fail(path, ln, "invalid date '" + a.date + "'");
        a.meal = utils::canonicalMeal(f[1]);
        if (a.meal.empty()) fail(path, ln, "invalid meal '" + f[1] + "'");
        a.expected = static_cast<int>(num(path, ln, f[2], "expected", true));
        a.actual = static_cast<int>(num(path, ln, f[3], "actual", true));
        out.push_back(a);
    }
    return out;
}

std::vector<MenuEntry> FileHandler::loadMenu(const std::string& path) {
    std::vector<MenuEntry> out;
    for (const auto& row : readRows(path, 5)) {
        const int ln = row.first;
        const auto& f = row.second;
        MenuEntry e;
        e.weekday = utils::weekdayFromName(f[0]);
        if (e.weekday < 0) fail(path, ln, "invalid day '" + f[0] + "' (use Monday ... Sunday)");
        e.meal = utils::canonicalMeal(f[1]);
        if (e.meal.empty()) fail(path, ln, "invalid meal '" + f[1] + "'");
        e.slot = f[2];
        e.dish = f[3];
        if (e.slot.empty() || e.dish.empty()) fail(path, ln, "slot and dish must not be empty");
        const std::string type = utils::toLower(f[4]);
        if (type != "fixed" && type != "choice") fail(path, ln, "type must be 'fixed' or 'choice'");
        e.choice = (type == "choice");
        out.push_back(e);
    }
    return out;
}

void FileHandler::saveDishes(const std::string& path, const std::vector<Dish>& dishes) {
    std::ofstream out = openForWrite(path);
    out << "name,category,meal,cost_per_kg,portion_g,calories,protein_g\n";
    for (const auto& d : dishes) {
        std::string meals;
        for (size_t i = 0; i < d.meals.size(); ++i) meals += (i ? ";" : "") + d.meals[i];
        out << d.name << ',' << d.category << ',' << meals << ',' << utils::fmt(d.costPerKg) << ','
            << utils::fmt(d.portionG, 0) << ',' << d.calories << ',' << utils::fmt(d.proteinG, 1) << '\n';
    }
}

void FileHandler::saveWaste(const std::string& path, const std::vector<WasteRecord>& records) {
    std::ofstream out = openForWrite(path);
    out << "date,meal,dish,cooked_kg,leftover_kg,plate_waste_kg\n";
    for (const auto& r : records)
        out << r.date << ',' << r.meal << ',' << r.dish << ',' << utils::fmt(r.cookedKg) << ','
            << utils::fmt(r.leftoverKg) << ',' << utils::fmt(r.plateKg) << '\n';
}

void FileHandler::saveMenu(const std::string& path, const std::vector<MenuEntry>& menu) {
    std::ofstream out = openForWrite(path);
    out << "day,meal,slot,dish,type\n";
    for (const auto& e : menu)
        out << utils::weekdayName(e.weekday) << ',' << e.meal << ',' << e.slot << ',' << e.dish << ','
            << (e.choice ? "choice" : "fixed") << '\n';
}

void FileHandler::saveAttendance(const std::string& path, const std::vector<Attendance>& rows) {
    std::ofstream out = openForWrite(path);
    out << "date,meal,expected,actual\n";
    for (const auto& a : rows) out << a.date << ',' << a.meal << ',' << a.expected << ',' << a.actual << '\n';
}
