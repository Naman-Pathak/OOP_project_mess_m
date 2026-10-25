#include <cstdio>
#include <fstream>
#include <stdexcept>

#include "FileHandler.h"
#include "test_helpers.h"

static void writeText(const char* path, const char* text) {
    std::ofstream(path) << text;
}

static bool loadFails(const char* path, int which) {
    try {
        if (which == 0) FileHandler::loadDishes(path);
        if (which == 1) FileHandler::loadWaste(path);
        if (which == 2) FileHandler::loadAttendance(path);
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

int main() {
    const char* dishesFile = "tmp_test_dishes.csv";
    const char* wasteFile = "tmp_test_waste.csv";
    const char* attFile = "tmp_test_att.csv";

    writeText(dishesFile,
              "name,category,meal,cost_per_kg,portion_g,calories,protein_g\n"
              "# a comment line\n"
              "Dal,Main,Lunch;Dinner,80,150,180,10\n"
              "\n"
              "Tea,Beverage,Any,40,150,60,2\r\n");
    auto dishes = FileHandler::loadDishes(dishesFile);
    CHECK(dishes.size() == 2);
    CHECK(dishes[0].meals.size() == 2 && dishes[0].servedAt("Dinner") && !dishes[0].servedAt("Breakfast"));
    CHECK(dishes[1].servedAt("Breakfast"));  // "Any"
    CHECK_NEAR(dishes[0].costPerPlate(), 12.0, 1e-9);

    // save -> load gives the same data
    FileHandler::saveDishes(dishesFile, dishes);
    auto again = FileHandler::loadDishes(dishesFile);
    CHECK(again.size() == 2 && again[0].name == "Dal" && again[0].calories == 180);

    std::vector<WasteRecord> waste(1);
    waste[0].date = "2026-09-07"; waste[0].meal = "Lunch"; waste[0].dish = "Dal";
    waste[0].cookedKg = 10; waste[0].leftoverKg = 2.5; waste[0].plateKg = 1.25;
    FileHandler::saveWaste(wasteFile, waste);
    auto w2 = FileHandler::loadWaste(wasteFile);
    CHECK(w2.size() == 1 && w2[0].dish == "Dal");
    CHECK_NEAR(w2[0].wasteKg(), 3.75, 1e-9);
    CHECK_NEAR(w2[0].wastePercent(), 37.5, 1e-9);

    std::vector<Attendance> att(1);
    att[0].date = "2026-09-07"; att[0].meal = "Lunch"; att[0].expected = 300; att[0].actual = 280;
    FileHandler::saveAttendance(attFile, att);
    auto a2 = FileHandler::loadAttendance(attFile);
    CHECK(a2.size() == 1 && a2[0].actual == 280);

    // bad input must be rejected with an exception
    CHECK(loadFails("does_not_exist.csv", 0));
    writeText(wasteFile, "date,meal,dish,cooked_kg,leftover_kg,plate_waste_kg\n2026-02-30,Lunch,Dal,10,1,1\n");
    CHECK(loadFails(wasteFile, 1));  // impossible date
    writeText(wasteFile, "date,meal,dish,cooked_kg,leftover_kg,plate_waste_kg\n2026-09-07,Lunch,Dal,10,8,5\n");
    CHECK(loadFails(wasteFile, 1));  // waste is more than cooked
    writeText(wasteFile, "date,meal,dish,cooked_kg,leftover_kg,plate_waste_kg\n2026-09-07,Lunch,Dal,10,1\n");
    CHECK(loadFails(wasteFile, 1));  // missing column
    writeText(attFile, "date,meal,expected,actual\n2026-09-07,Brunch,300,280\n");
    CHECK(loadFails(attFile, 2));    // invalid meal

    std::remove(dishesFile);
    std::remove(wasteFile);
    std::remove(attFile);
    return finish();
}
