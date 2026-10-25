#pragma once
#include <string>
#include <vector>

// One dish on the mess menu.
struct Dish {
    std::string name;
    std::string category;            // Staple, Main, Side, Dessert, Beverage
    std::vector<std::string> meals;  // meals it can be served at ("Any" = all)
    double costPerKg = 0;            // Rs per kg of cooked food
    double portionG = 0;             // standard portion per student, grams
    int calories = 0;                // per portion
    double proteinG = 0;             // per portion

    double costPerPlate() const { return costPerKg * portionG / 1000.0; }

    bool servedAt(const std::string& meal) const {
        for (const auto& m : meals)
            if (m == "Any" || m == meal) return true;
        return false;
    }
};
