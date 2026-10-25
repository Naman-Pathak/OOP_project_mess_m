#pragma once
#include <string>
#include <vector>
#include "Dish.h"
#include "Menu.h"
#include "WasteRecord.h"

// Holds all data in memory and is the only place records are added/removed,
// so every record goes through validation.
class Tracker {
public:
    Tracker() = default;
    Tracker(std::vector<Dish> dishes, std::vector<WasteRecord> waste,
            std::vector<Attendance> attendance, std::vector<MenuEntry> menu = {});

    const std::vector<Dish>& dishes() const { return dishes_; }
    const std::vector<WasteRecord>& waste() const { return waste_; }
    const std::vector<Attendance>& attendance() const { return attendance_; }
    const std::vector<MenuEntry>& menu() const { return menu_; }

    const Dish* findDish(const std::string& name) const;  // case-insensitive, nullptr if missing

    bool addDish(const Dish& dish, std::string& err);
    bool addWaste(WasteRecord record, std::string& err);
    bool removeWaste(size_t index);
    bool addAttendance(Attendance a, std::string& err);  // replaces same date+meal

    std::vector<size_t> findByDate(const std::string& date) const;
    std::vector<size_t> findByDish(const std::string& dish) const;
    int actualHeadcount(const std::string& date, const std::string& meal) const;  // -1 if unknown
    std::string lastDate() const;  // latest date in the data ("" if none)

    // ---- the mess weekly menu ----
    std::vector<MenuEntry> menuFor(int weekday, const std::string& meal) const;
    std::vector<std::string> dishesFor(int weekday, const std::string& meal) const;  // unique, in menu order
    std::vector<std::string> menuProblems() const;  // unknown dishes, dishes not listed for that meal

private:
    std::vector<Dish> dishes_;
    std::vector<WasteRecord> waste_;
    std::vector<Attendance> attendance_;
    std::vector<MenuEntry> menu_;
};
