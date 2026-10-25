#pragma once
#include <string>
#include <vector>
#include "Dish.h"
#include "Menu.h"
#include "WasteRecord.h"

// Reads and writes the three CSV files. All functions throw std::runtime_error
// with a helpful message (file name + line number) when something is wrong.
class FileHandler {
public:
    static std::vector<Dish> loadDishes(const std::string& path);
    static std::vector<WasteRecord> loadWaste(const std::string& path);
    static std::vector<Attendance> loadAttendance(const std::string& path);
    static std::vector<MenuEntry> loadMenu(const std::string& path);

    static void saveDishes(const std::string& path, const std::vector<Dish>& dishes);
    static void saveWaste(const std::string& path, const std::vector<WasteRecord>& records);
    static void saveAttendance(const std::string& path, const std::vector<Attendance>& rows);
    static void saveMenu(const std::string& path, const std::vector<MenuEntry>& menu);
};
