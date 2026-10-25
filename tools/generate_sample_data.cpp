// Creates SAMPLE waste_log.csv and attendance.csv for demonstrating the project.
//
// It reads the REAL weekly menu and dish list (data/menu.csv and data/dishes.csv - the IIIT Bhubaneswar
// ground floor mess menu) and simulates 4 weeks of cooking, headcount and waste for exactly those dishes.
// The simulated numbers are NOT real measurements - replace waste_log.csv and attendance.csv with your
// mess's real records.
//
//   ./generate_sample_data [data_folder]      (default: data)
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include "FileHandler.h"
#include "Utils.h"

namespace {

struct Behaviour {
    double uptake;     // share of students who take this dish
    double plateRate;  // share of the served food that is thrown away from plates
};

// Typical behaviour by dish category ...
Behaviour defaultFor(const std::string& category) {
    if (category == "Staple") return {0.95, 0.05};
    if (category == "Dal") return {0.88, 0.07};
    if (category == "Dry") return {0.65, 0.10};
    if (category == "Gravy") return {0.80, 0.08};
    if (category == "Side") return {0.60, 0.10};
    if (category == "Dessert") return {0.85, 0.04};
    if (category == "Beverage") return {0.80, 0.03};
    if (category == "Milk") return {0.50, 0.06};
    return {0.85, 0.06};  // Main
}

// ... with a few dishes that are known to be loved or ignored (all invented for the simulation).
const std::map<std::string, Behaviour>& overrides() {
    static const std::map<std::string, Behaviour> m = {
        {"Chole Bhature", {0.95, 0.04}},  {"Aloo Paratha", {0.90, 0.04}}, {"Upma", {0.50, 0.18}},
        {"Idli Sambar", {0.85, 0.05}},    {"Pasta/Macaroni", {0.85, 0.05}}, {"Methi Puri", {0.70, 0.08}},
        {"Egg/Banana", {0.90, 0.02}},     {"Tomato Sauce", {0.40, 0.10}}, {"Chutney", {0.55, 0.10}},
        {"Chicken Biryani", {0.60, 0.04}}, {"Veg Biryani", {0.35, 0.08}}, {"Chicken Kasa", {0.50, 0.04}},
        {"Chicken Masala", {0.55, 0.04}}, {"Mutton Curry", {0.35, 0.03}}, {"Fish Curry", {0.40, 0.05}},
        {"Egg Curry", {0.50, 0.05}},      {"Egg Chicken Bharta", {0.45, 0.05}}, {"Mix Veg", {0.40, 0.14}},
        {"Kadhai Paneer", {0.45, 0.04}},  {"Paneer Masala", {0.45, 0.04}}, {"Paneer Bhurji", {0.55, 0.04}},
        {"Soyabean Curry", {0.55, 0.15}}, {"Soyabean Chilli", {0.60, 0.10}}, {"Parwal Curry", {0.50, 0.16}},
        {"Veg Navratna", {0.45, 0.10}},   {"Ghanta", {0.60, 0.12}},       {"Karela Chips", {0.35, 0.20}},
        {"Salad", {0.40, 0.20}},          {"Pickle", {0.50, 0.05}},       {"Curd", {0.60, 0.07}},
        {"Chips", {0.85, 0.03}},          {"Papad", {0.75, 0.05}},        {"Cone Ice-cream", {0.95, 0.02}},
        {"Gulab Jamun", {0.95, 0.03}},    {"Pahala Rasgulla", {0.95, 0.03}}, {"Jalebi", {0.80, 0.04}},
        {"Suji Halwa", {0.50, 0.10}},     {"Kheer", {0.90, 0.05}}};
    return m;
}

double round2(double v) { return std::round(v * 100.0) / 100.0; }

}  // namespace

int main(int argc, char* argv[]) {
    const std::string dir = argc > 1 ? argv[1] : "data";

    std::vector<Dish> dishes;
    std::vector<MenuEntry> menu;
    try {
        dishes = FileHandler::loadDishes(dir + "/dishes.csv");
        menu = FileHandler::loadMenu(dir + "/menu.csv");
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n(Run from the project folder, or pass the data folder as an argument.)\n";
        return 1;
    }
    std::map<std::string, const Dish*> byName;
    for (const auto& d : dishes) byName[d.name] = &d;
    for (const auto& e : menu)
        if (!byName.count(e.dish)) {
            std::cerr << "menu.csv uses '" << e.dish << "' which is not in dishes.csv\n";
            return 1;
        }

    const std::vector<std::string>& meals = utils::mealNames();           // Breakfast, Lunch, Dinner
    const int baseHeads[3] = {230, 320, 300};                             // a hostel of about 350 students
    const double weekdayFactor[3][7] = {{1.0, 1.0, 1.0, 1.0, 0.95, 0.85, 0.70},   // Sunday breakfast is late
                                        {1.0, 1.0, 1.0, 1.0, 1.12, 0.80, 0.90},   // Friday special lunch is popular
                                        {1.0, 1.0, 1.0, 1.0, 0.90, 0.75, 0.95}};  // Saturday dinner: eating out

    std::mt19937 rng(2026);
    std::normal_distribution<double> noise(0.0, 1.0);

    std::vector<WasteRecord> waste;
    std::vector<Attendance> attendance;
    const std::string startDate = "2026-09-07";  // a Monday
    for (int day = 0; day < 28; ++day) {
        const std::string date = utils::addDays(startDate, day);
        const int wd = utils::weekdayOf(date);
        for (size_t m = 0; m < meals.size(); ++m) {
            Attendance a;
            a.date = date;
            a.meal = meals[m];
            a.expected = baseHeads[m];  // the kitchen always plans for the same number
            a.actual = static_cast<int>(std::lround(baseHeads[m] * weekdayFactor[m][wd] * (1.0 + 0.05 * noise(rng))));
            attendance.push_back(a);

            for (const auto& entry : menu) {  // every dish on that day's menu is cooked
                if (entry.weekday != wd || entry.meal != meals[m]) continue;
                const Dish& d = *byName.at(entry.dish);
                const auto ov = overrides().find(d.name);
                const Behaviour b = ov != overrides().end() ? ov->second : defaultFor(d.category);
                const double portionKg = d.portionG / 1000.0;

                // The kitchen cooks for the planned headcount and only partly adjusts for popularity.
                WasteRecord r;
                r.date = date;
                r.meal = a.meal;
                r.dish = d.name;
                r.cookedKg = round2(a.expected * portionKg * (0.5 + 0.5 * b.uptake) * 1.15 * (1.0 + 0.03 * noise(rng)));
                const double demand = a.actual * b.uptake * portionKg * (1.0 + 0.05 * noise(rng));
                const double served = std::min(r.cookedKg, demand);
                r.leftoverKg = round2(r.cookedKg - served);
                const double plate = std::max(0.0, served * b.plateRate * (1.0 + 0.2 * noise(rng)));
                r.plateKg = round2(std::min(plate, r.cookedKg - r.leftoverKg));
                waste.push_back(r);
            }
        }
    }

    try {
        FileHandler::saveWaste(dir + "/waste_log.csv", waste);
        FileHandler::saveAttendance(dir + "/attendance.csv", attendance);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    std::cout << "Sample data written to '" << dir << "' using the mess menu (" << menu.size() << " menu entries, "
              << dishes.size() << " dishes): " << waste.size() << " waste records, " << attendance.size()
              << " attendance rows.\n";
    return 0;
}
