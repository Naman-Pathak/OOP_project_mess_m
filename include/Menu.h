#pragma once
#include <string>

// One line of the mess weekly menu: "on this weekday, at this meal, this slot has this dish".
struct MenuEntry {
    int weekday = 0;     // 0 = Monday ... 6 = Sunday
    std::string meal;    // Breakfast / Lunch / Dinner
    std::string slot;    // Items (breakfast), Common, Dry, Gravy, Dessert or Compulsory
    std::string dish;    // must exist in dishes.csv
    bool choice = false; // true: students pick ONE of the choice dishes of this slot (e.g. Egg Curry / Mix Veg)
};
