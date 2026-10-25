#include "Utils.h"
#include "test_helpers.h"

int main() {
    using namespace utils;
    CHECK(isValidDate("2024-02-29"));
    CHECK(!isValidDate("2026-02-29"));      // 2026 is not a leap year
    CHECK(!isValidDate("2026-13-01"));
    CHECK(!isValidDate("26-01-01"));
    CHECK(!isValidDate("2026/01/01"));

    CHECK(weekdayOf("2026-10-07") == 2);    // Wednesday
    CHECK(weekdayOf("2026-09-07") == 0);    // Monday
    CHECK(weekdayName(6) == "Sunday");
    CHECK(weekdayFromName("Monday") == 0 && weekdayFromName(" sunday ") == 6 && weekdayFromName("fri") == 4);
    CHECK(weekdayFromName("Funday") == -1 && weekdayFromName("") == -1);

    CHECK(addDays("2026-12-31", 1) == "2027-01-01");
    CHECK(addDays("2026-03-01", -1) == "2026-02-28");
    CHECK(addDays("2024-02-28", 1) == "2024-02-29");
    CHECK(dateToDays("2026-10-08") - dateToDays("2026-10-01") == 7);

    auto parts = split("a, b,,c", ',');
    CHECK(parts.size() == 4 && parts[1] == "b" && parts[2].empty());
    CHECK(trim("  hi \r\n") == "hi");
    CHECK(toLower("PaNeEr") == "paneer");

    double d = 0;
    int i = 0;
    CHECK(parseDouble(" 12.5 ", d) && d == 12.5);
    CHECK(!parseDouble("12abc", d));
    CHECK(!parseDouble("", d));
    CHECK(parseInt("42", i) && i == 42);
    CHECK(!parseInt("4.2", i));

    CHECK(canonicalMeal(" lunch ") == "Lunch");
    CHECK(canonicalMeal("brunch").empty());
    CHECK(canonicalMeal("snacks").empty());   // this mess serves only 3 meals
    CHECK(mealNames().size() == 3);
    return finish();
}
