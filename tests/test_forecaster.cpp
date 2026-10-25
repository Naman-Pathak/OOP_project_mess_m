#include "Forecaster.h"
#include "Tracker.h"
#include "Utils.h"
#include "test_helpers.h"

int main() {
    // Three weeks where headcount depends only on the weekday: 100, 110, 120 ... 160.
    std::vector<Attendance> att;
    for (int i = 0; i < 21; ++i) {
        Attendance a;
        a.date = utils::addDays("2026-09-07", i);  // starts on a Monday
        a.meal = "Lunch";
        a.expected = 150;
        a.actual = 100 + 10 * utils::weekdayOf(a.date);
        att.push_back(a);
    }
    Tracker t({}, {}, att);
    Forecaster f(t);

    auto pts = f.series("Lunch");
    CHECK(pts.size() == 21);
    CHECK(pts.front().date == "2026-09-07" && pts.back().date == "2026-09-27");
    CHECK(f.series("Dinner").empty());

    CHECK_NEAR(Forecaster::weekdayAverage(pts, 0), 100.0, 1e-9);   // Monday
    CHECK_NEAR(Forecaster::weekdayAverage(pts, 6), 160.0, 1e-9);   // Sunday
    CHECK_NEAR(Forecaster::movingAverage(pts, 0), 130.0, 1e-9);    // mean of one full week

    // The weekday pattern is exact, so that method must win and be right.
    Forecast next = f.forecast("Lunch", "2026-09-28");              // Monday
    CHECK_NEAR(next.value, 100.0, 1e-9);
    CHECK(next.method == "Weekday average");
    CHECK_NEAR(next.mae, 0.0, 1e-9);
    Forecast sat = f.forecast("Lunch", "2026-10-03");               // Saturday
    CHECK_NEAR(sat.value, 150.0, 1e-9);

    Forecast none = f.forecast("Dinner", "2026-09-28");
    CHECK(none.value == 0.0);

    // Too little history falls back to a plain average.
    Tracker small({}, {}, std::vector<Attendance>(att.begin(), att.begin() + 3));
    Forecast fb = Forecaster(small).forecast("Lunch", "2026-09-28");
    CHECK_NEAR(fb.value, 110.0, 1e-9);
    return finish();
}
