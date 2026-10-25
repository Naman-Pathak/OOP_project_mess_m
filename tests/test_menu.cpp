#include <cstdio>
#include <fstream>
#include <stdexcept>

#include "FileHandler.h"
#include "Tracker.h"
#include "Utils.h"
#include "test_helpers.h"

static void writeText(const char* path, const char* text) { std::ofstream(path) << text; }

static bool menuLoadFails(const char* path) {
    try {
        FileHandler::loadMenu(path);
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

static Dish mk(const char* name, const char* meals) {
    Dish d;
    d.name = name; d.category = "Main"; d.meals = utils::split(meals, ';');
    d.costPerKg = 100; d.portionG = 100; d.calories = 100; d.proteinG = 5;
    return d;
}

int main() {
    const char* path = "tmp_test_menu.csv";
    writeText(path,
              "day,meal,slot,dish,type\n"
              "# comment\n"
              "Monday,Lunch,Common,Rice,fixed\n"
              "monday,lunch,Gravy,Egg Curry,choice\n"
              "MON,Lunch,Gravy,Mix Veg,choice\n"
              "Tuesday,Dinner,Common,Rice,fixed\n");
    auto menu = FileHandler::loadMenu(path);
    CHECK(menu.size() == 4);
    CHECK(menu[0].weekday == 0 && menu[0].meal == "Lunch" && !menu[0].choice);
    CHECK(menu[1].choice && menu[2].weekday == 0);       // day names are case-insensitive, "MON" works
    CHECK(menu[3].weekday == 1 && menu[3].meal == "Dinner");

    // save -> load round trip
    FileHandler::saveMenu(path, menu);
    auto again = FileHandler::loadMenu(path);
    CHECK(again.size() == 4 && again[1].dish == "Egg Curry" && again[1].choice);

    // bad menu files are rejected
    writeText(path, "day,meal,slot,dish,type\nFunday,Lunch,Common,Rice,fixed\n");
    CHECK(menuLoadFails(path));                           // invalid day
    writeText(path, "day,meal,slot,dish,type\nMonday,Brunch,Common,Rice,fixed\n");
    CHECK(menuLoadFails(path));                           // invalid meal
    writeText(path, "day,meal,slot,dish,type\nMonday,Lunch,Common,Rice,maybe\n");
    CHECK(menuLoadFails(path));                           // invalid type
    writeText(path, "day,meal,slot,dish,type\nMonday,Lunch,Common,Rice\n");
    CHECK(menuLoadFails(path));                           // missing column
    std::remove(path);

    // Tracker menu queries
    std::vector<Dish> dishes = {mk("Rice", "Lunch;Dinner"), mk("Egg Curry", "Lunch"), mk("Mix Veg", "Dinner")};
    Tracker t(dishes, {}, {}, menu);
    CHECK(t.menuFor(0, "Lunch").size() == 3);
    CHECK(t.menuFor(0, "Dinner").empty());
    auto names = t.dishesFor(0, "Lunch");
    CHECK(names.size() == 3 && names[0] == "Rice" && names[1] == "Egg Curry");
    CHECK(t.dishesFor(1, "Dinner").size() == 1);

    // menu problems: Mix Veg is on the Monday Lunch menu but dishes list it for Dinner only
    auto problems = t.menuProblems();
    CHECK(problems.size() == 1);
    CHECK(!problems.empty() && problems[0].find("Mix Veg") != std::string::npos);
    Tracker unknown({}, {}, {}, menu);
    CHECK(!unknown.menuProblems().empty());               // dishes missing entirely
    CHECK(Tracker().menu().empty());                      // no menu is allowed
    return finish();
}
