#pragma once
#include <string>

// One row of the daily waste log: one dish, at one meal, on one day.
struct WasteRecord {
    std::string date;      // YYYY-MM-DD
    std::string meal;      // Breakfast / Lunch / Dinner
    std::string dish;
    double cookedKg = 0;   // total food prepared
    double leftoverKg = 0; // never served (left in the vessels)
    double plateKg = 0;    // served but thrown away from plates

    double wasteKg() const { return leftoverKg + plateKg; }
    double servedKg() const { return cookedKg - leftoverKg; }
    double consumedKg() const { return servedKg() - plateKg; }
    double wastePercent() const { return cookedKg > 0 ? wasteKg() / cookedKg * 100.0 : 0.0; }
};

// Headcount for one meal on one day.
struct Attendance {
    std::string date;
    std::string meal;
    int expected = 0;  // number the kitchen planned for
    int actual = 0;    // number who actually ate
};
