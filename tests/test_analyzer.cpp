#include "Analyzer.h"
#include "Tracker.h"
#include "test_helpers.h"

static Dish makeDish(const char* name, const char* cat, double costKg, double portionG) {
    Dish d;
    d.name = name; d.category = cat; d.meals = {"Lunch"};
    d.costPerKg = costKg; d.portionG = portionG; d.calories = 200; d.proteinG = 5;
    return d;
}

static WasteRecord rec(const char* date, const char* dish, double cooked, double left, double plate) {
    WasteRecord r;
    r.date = date; r.meal = "Lunch"; r.dish = dish;
    r.cookedKg = cooked; r.leftoverKg = left; r.plateKg = plate;
    return r;
}

int main() {
    std::vector<Dish> dishes = {makeDish("Dal", "Main", 100, 150), makeDish("Rice", "Staple", 50, 200)};
    std::vector<WasteRecord> waste = {
        rec("2026-09-07", "Dal", 10, 2, 1),    // waste 3 kg = 30 %, Rs 300
        rec("2026-09-08", "Dal", 10, 1, 1),    // waste 2 kg = 20 %, Rs 200
        rec("2026-09-07", "Rice", 20, 6, 0)};  // waste 6 kg = 30 %, Rs 300
    std::vector<Attendance> att;
    Attendance a; a.date = "2026-09-07"; a.meal = "Lunch"; a.expected = 100; a.actual = 100;
    att.push_back(a);
    Tracker t(dishes, waste, att);
    Analyzer an(t);

    Summary all = an.overall();
    CHECK(all.records == 3);
    CHECK_NEAR(all.cookedKg, 40.0, 1e-9);
    CHECK_NEAR(all.wasteKg, 11.0, 1e-9);
    CHECK_NEAR(all.wastePct, 27.5, 1e-9);
    CHECK_NEAR(all.costOfWaste, 800.0, 1e-9);

    auto perDish = an.byDish();
    CHECK_NEAR(perDish["Dal"].wastePct, 25.0, 1e-9);   // (3+2)/(10+10)
    CHECK_NEAR(perDish["Rice"].costOfWaste, 300.0, 1e-9);

    auto top = an.topWasted(1, false);
    CHECK(top.size() == 1 && top[0].first == "Rice");   // 6 kg beats Dal's 5 kg
    auto topCost = an.topWasted(1, true);
    CHECK(topCost[0].first == "Dal");                   // Rs 500 beats Rs 300

    CHECK(an.byWeekday().size() == 2);                  // Monday and Tuesday
    CHECK(an.byMeal().count("Lunch") == 1);
    CHECK(an.byDate().size() == 2);

    // per-head statistics only use days with a known headcount (7 Sept, 100 students)
    for (const auto& s : an.dishStats()) {
        if (s.dish == "Dal") {
            CHECK(s.hasPerHead);
            CHECK_NEAR(s.servedPerHeadKg, (10.0 - 2.0) / 100.0, 1e-9);
            CHECK_NEAR(s.cookedPerHeadKg, 10.0 / 100.0, 1e-9);
            CHECK_NEAR(s.acceptance, 0.75, 1e-9);
            CHECK_NEAR(s.avgCookedKg, 10.0, 1e-9);
        }
    }

    // Tracker validation
    std::string err;
    WasteRecord bad = rec("2026-09-09", "Dal", 5, 4, 3);
    CHECK(!t.addWaste(bad, err));                       // 7 kg waste out of 5 kg cooked
    bad = rec("2026-09-09", "Unknown", 5, 1, 1);
    CHECK(!t.addWaste(bad, err));
    WasteRecord good = rec("2026-09-09", "dal", 5, 1, 1);
    CHECK(t.addWaste(good, err));
    CHECK(t.waste().back().dish == "Dal");              // spelling normalised
    CHECK(t.removeWaste(t.waste().size() - 1));
    CHECK(!t.removeWaste(999));
    CHECK(t.findByDate("2026-09-07").size() == 2);
    CHECK(t.findByDish("DAL").size() == 2);
    return finish();
}
