#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "Tracker.h"

// Totals for any group of records (overall, one dish, one meal, one weekday...).
struct Summary {
    int records = 0;
    double cookedKg = 0;
    double leftoverKg = 0;
    double plateKg = 0;
    double wasteKg = 0;
    double wastePct = 0;     // wasteKg / cookedKg * 100
    double costOfWaste = 0;  // Rs, wasteKg * cost per kg of the dish
};

// Per-dish behaviour learned from history (used by the optimizer).
struct DishStats {
    std::string dish;
    int records = 0;
    double cookedKg = 0;
    double wasteKg = 0;
    double wastePct = 0;
    double acceptance = 0;        // share of cooked food that was actually eaten (0..1)
    double avgCookedKg = 0;       // average quantity cooked per serving day
    bool hasPerHead = false;
    double cookedPerHeadKg = 0;   // cooked kg per student who ate
    double servedPerHeadKg = 0;   // served kg per student who ate (real demand)
};

class Analyzer {
public:
    explicit Analyzer(const Tracker& tracker) : tracker_(tracker) {}

    Summary overall() const;
    std::map<std::string, Summary> byDish() const;
    std::map<std::string, Summary> byMeal() const;
    std::map<int, Summary> byWeekday() const;   // 0 = Monday
    std::map<std::string, Summary> byDate() const;

    // Dishes sorted by total waste (kg) or by cost of waste, highest first.
    std::vector<std::pair<std::string, Summary>> topWasted(size_t n, bool byCost = false) const;
    std::vector<DishStats> dishStats() const;

private:
    void accumulate(Summary& s, const WasteRecord& r) const;
    const Tracker& tracker_;
};
