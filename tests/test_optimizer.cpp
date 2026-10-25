#include <set>

#include "Optimizer.h"
#include "Tracker.h"
#include "Utils.h"
#include "test_helpers.h"

static Dish mk(const char* name, const char* cat, double costKg, double portionG, int kcal, double protein) {
    Dish d;
    d.name = name; d.category = cat; d.meals = {"Lunch"};
    d.costPerKg = costKg; d.portionG = portionG; d.calories = kcal; d.proteinG = protein;
    return d;
}

int main() {
    // Lunch needs one Staple, one Dal, one Dry item and one Gravy.
    // Plate costs: RiceA 10, RiceB 20, DalA 10, DalB 25, FryA 5, FryB 5, CurryA 8, CurryB 20
    std::vector<Dish> dishes = {
        mk("RiceA", "Staple", 50, 200, 300, 5),  mk("RiceB", "Staple", 100, 200, 350, 6),
        mk("DalA", "Dal", 100, 100, 150, 8),     mk("DalB", "Dal", 250, 100, 250, 15),
        mk("FryA", "Dry", 100, 50, 30, 1),       mk("FryB", "Dry", 100, 50, 40, 2),
        mk("CurryA", "Gravy", 80, 100, 100, 4),  mk("CurryB", "Gravy", 200, 100, 200, 9)};

    // 14 days of history for the Lunch headcount (always 100 students).
    std::vector<Attendance> att;
    std::vector<WasteRecord> waste;
    for (int i = 0; i < 14; ++i) {
        const std::string date = utils::addDays("2026-09-07", i);
        Attendance a; a.date = date; a.meal = "Lunch"; a.expected = 100; a.actual = 100;
        att.push_back(a);
        WasteRecord r;
        r.date = date; r.meal = "Lunch"; r.dish = "DalA";
        r.cookedKg = 20;      // cooks 20 kg ...
        r.leftoverKg = 8;     // ... but only 12 kg is ever served (120 g per student)
        r.plateKg = 0;
        waste.push_back(r);
    }
    Tracker t(dishes, waste, att);
    Optimizer opt(t, 0.05);

    // ---- cooking plan ----
    CookPlan plan = opt.planQuantities("2026-09-21", "Lunch", {"DalA", "RiceA", "Ghost"});
    CHECK(plan.forecastHeads == 100);
    CHECK(plan.unknownDishes.size() == 1 && plan.unknownDishes[0] == "Ghost");
    CHECK(plan.items.size() == 2);
    CHECK(plan.items[0].hasHistory);
    CHECK_NEAR(plan.items[0].perHeadKg, 0.12, 1e-9);
    CHECK_NEAR(plan.items[0].plannedKg, 100 * 0.12 * 1.05, 1e-9);   // 12.6 kg instead of 20 kg
    CHECK_NEAR(plan.items[0].savedKg, 20.0 - 12.6, 1e-9);
    CHECK_NEAR(plan.items[0].savedCost, (20.0 - 12.6) * 100.0, 1e-6);
    CHECK(!plan.items[1].hasHistory);                               // RiceA never logged
    CHECK_NEAR(plan.items[1].savedKg, 0.0, 1e-9);                   // no saving is claimed without data

    // ---- menu search ----
    std::set<std::string> none;
    MenuOption m;
    CHECK(opt.bestMenu("Lunch", 100.0, 0, none, m));
    CHECK(m.dishes.size() == 4);                                    // staple + dal + dry + gravy
    CHECK(m.costPerPlate <= 100.0);

    // a tight budget must exclude the expensive dishes: cheapest plate = 10 + 10 + 5 + 8 = 33
    CHECK(opt.bestMenu("Lunch", 33.0, 0, none, m));
    CHECK_NEAR(m.costPerPlate, 33.0, 1e-9);
    CHECK(!opt.bestMenu("Lunch", 32.0, 0, none, m));                // nothing is that cheap

    // calorie minimum: the cheapest plate has at most 300+150+40+100 = 590 kcal
    CHECK(!opt.bestMenu("Lunch", 33.0, 600, none, m));
    CHECK(opt.bestMenu("Lunch", 100.0, 600, none, m));
    CHECK(m.calories >= 600);

    CHECK(!opt.bestMenu("Breakfast", 100.0, 0, none, m));           // no breakfast dishes exist

    // exclusion: asking to avoid DalA forces DalB
    std::set<std::string> avoid = {"DalA"};
    CHECK(opt.bestMenu("Lunch", 100.0, 0, avoid, m));
    bool hasDalA = false;
    for (const auto& d : m.dishes) if (d.name == "DalA") hasDalA = true;
    CHECK(!hasDalA);

    // dish scores stay between 0 and 1
    for (const auto& kv : opt.dishScores()) CHECK(kv.second >= 0.0 && kv.second <= 1.0);

    // weekly plan: 7 entries and the dal changes from one day to the next
    auto week = opt.weeklyPlan("Lunch", "2026-09-21", 100.0, 0);
    CHECK(week.size() == 7);
    CHECK(week[0].first == "2026-09-21" && week[6].first == "2026-09-27");
    auto mainOf = [](const MenuOption& o) {
        for (const auto& d : o.dishes) if (d.category == "Dal") return d.name;
        return std::string();
    };
    CHECK(mainOf(week[0].second) != mainOf(week[1].second));
    return finish();
}
