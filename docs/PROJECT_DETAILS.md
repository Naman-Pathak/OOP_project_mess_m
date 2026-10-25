# Mess Food Waste Tracking and Meal Optimization
## Complete Project Details and Explanation (C++ project)

---

## Table of contents

1. Project summary
2. Problem statement and objectives
3. Features
4. Technology and requirements
5. How to build and run
6. Folder structure and purpose
7. File-by-file explanation
8. Data files (data dictionary and validation rules)
9. Key measurements and formulas
10. Algorithms in detail
11. The sample data (important: it is simulated)
12. Sample results
13. Testing
14. C++ concepts used
15. Using real mess data
16. Limitations and future work
17. Possible viva / presentation questions
18. Glossary

---

## 1. Project summary

In a mess (hostel or college canteen) food is usually cooked for a fixed headcount, but the real number of students who eat changes with the day and the meal, and students do not like every dish equally. The result is food that is cooked but never eaten: money, effort and food wasted.

This project is a **console application written completely in C++17** that:

- records how much of each dish was cooked, how much was left in the vessels (*leftover*) and how much was thrown from plates (*plate waste*);
- records the expected and actual headcount of each meal;
- shows where the waste and the money loss are concentrated;
- forecasts the headcount of the next meal;
- recommends **how much of each dish to cook** (meal optimization, quantity side);
- recommends **which dishes to serve** within a budget and nutrition limit (meal optimization, menu side);
- writes a full text report.

Nothing outside the C++ standard library is used. Data is stored in simple CSV text files that the program reads and writes itself.

**The food menu used by the project is the real weekly menu of the IIIT Bhubaneswar Ground Floor Mess** (breakfast, lunch and dinner, with the Common / Dry item / Gravy / Dessert slots and the veg / non-veg choices). It is stored in `data/menu.csv` and drives the cooking plans. The mess serves three meals a day, so the project works with **Breakfast, Lunch and Dinner** (there is no snacks meal).

---

## 2. Problem statement and objectives

**Problem.** A mess wastes a large share of the food it cooks. Without measurements nobody knows which dishes, meals or days are responsible, and cooking quantities are decided by habit.

**Objectives.**

1. Build a simple, reliable way to **track** food waste for every dish and meal.
2. **Measure** waste in kilograms, in percent of the food cooked, and in rupees.
3. **Find the pattern**: worst dishes, worst meals, worst weekdays.
4. **Predict demand** (headcount) so the kitchen can cook for the number who will really come.
5. **Plan quantities** per dish from real per-student consumption.
6. **Choose menus** that students actually eat, that cost less and that are nutritious.
7. Show the **saving** achieved by following the plan.

---

## 3. Features

| # | Feature | Where in the menu |
|---|---|---|
| 1 | List dishes with cost per plate and calories | 1 |
| 2 | Add a waste record (with validation) | 2 |
| 3 | Add attendance (expected and actual headcount) | 3 |
| 4 | View recent records, search by date, search by dish | 4, 5, 6 |
| 5 | Delete a wrong record | 7 |
| 6 | Waste analysis: overall, by meal, by weekday | 8 |
| 7 | Top wasted dishes by kg and by cost, with a bar chart | 9 |
| 8 | Dish statistics (how much is really eaten per student) | 10 |
| 9 | Headcount forecast with automatic method selection | 11 |
| 10 | Cooking quantity plan, **taken automatically from the scheduled mess menu** of that weekday, with the estimated saving | 12 |
| 11 | Best-menu suggestion under budget and calorie limits (optimizer) | 13 |
| 12 | 7-day menu plan without monotonous repetition (optimizer) | 14 |
| 13 | Full report written to a text file | 15 or `--report` |
| 14 | Show the mess menu for one day or the whole week | 16 |
| 15 | Automatic saving of waste and attendance data on exit | 0 |

---

## 4. Technology and requirements

- Language: **C++17** only.
- Libraries: only the standard library (`<vector>`, `<map>`, `<set>`, `<string>`, `<fstream>`, `<iostream>`, `<iomanip>`, `<algorithm>`, `<cmath>`, `<random>`, `<filesystem>` ...).
- Compiler: any C++17 compiler (g++ 8+, clang 7+, MSVC 2017+).
- Build systems provided: a `Makefile`, a `CMakeLists.txt`, or a plain `g++` command.
- Platform: Windows, Linux or macOS (console).

---

## 5. How to build and run

### 5.1 Build

**Makefile**

```bash
make            # builds mess_tracker and generate_sample_data
make test       # builds and runs the 6 unit-test programs
make clean      # removes build files
```

**CMake**

```bash
mkdir build
cd build
cmake ..
cmake --build .
ctest           # runs the tests
```

**Windows executable (.exe)**

Install a free C++ compiler (for example the WinLibs MinGW-w64 zip, with its `bin` folder on PATH, or Visual Studio Build Tools) and double-click `build_windows.bat` in the project folder. It creates `mess_tracker.exe` and `generate_sample_data.exe` (statically linked with MinGW, so they run on other Windows PCs without extra files). Keep the `data` folder next to the `.exe`.

**Plain g++ (no build tool)**

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o mess_tracker
g++ -std=c++17 -Iinclude tools/generate_sample_data.cpp $(ls src/*.cpp | grep -v main.cpp) -o generate_sample_data
```

### 5.2 Run

Always run from the **project folder** so the program finds `data/`.

```bash
./mess_tracker                      # interactive menu
./mess_tracker --report             # write reports/weekly_summary.txt and exit
./mess_tracker --report --out my_report.txt
./mess_tracker --data other_folder  # use another data folder
./generate_sample_data              # (re)create the SAMPLE waste_log.csv and attendance.csv for the menu in data/
./generate_sample_data other_folder
```

On Windows use `mess_tracker.exe` instead of `./mess_tracker`.

### 5.3 Typical workflow

1. Run `./generate_sample_data` once to try the project (it simulates waste and headcount for the real mess menu), or put your own waste and attendance records in `data/`.
2. Run `./mess_tracker` and explore menu items 8 to 14.
3. Each day add the day's waste records (menu 2) and headcounts (menu 3).
4. Use menu 11 to 14 to plan tomorrow's cooking and the next week's menu.
5. Choose 0 to save; data is written back to the CSV files.
6. Use menu 15 (or `--report`) to produce the report for your project submission.

---

## 6. Folder structure and purpose

```
mess-food-waste-tracker/
├── CMakeLists.txt
├── Makefile
├── README.md
├── .gitignore
├── include/        (10 header files)
├── src/            (8 source files)
├── data/           (4 CSV files)
├── tests/          (6 test programs + 1 helper header)
├── tools/          (1 sample-data generator)
├── reports/        (report written by the program)
└── docs/           (this file)
```

| Folder | What it contains | Why it exists |
|---|---|---|
| `include/` | `.h` header files | Declares the classes and functions. Splitting declaration (`.h`) from implementation (`.cpp`) is standard C++ practice and lets many files share the same interface. |
| `src/` | `.cpp` files and `main.cpp` | The actual code. `main.cpp` is the user interface; every other file is one module. |
| `data/` | `menu.csv`, `dishes.csv`, `waste_log.csv`, `attendance.csv` | The program's database. `menu.csv` and `dishes.csv` describe the mess (edited by hand); `waste_log.csv` and `attendance.csv` are the daily records. Plain text so it can be opened in any editor. |
| `tests/` | `test_*.cpp`, `test_helpers.h` | Automatic checks that each module gives correct answers. |
| `tools/` | `generate_sample_data.cpp` | Reads the real menu and dish list and simulates waste and headcount for them, so the project can be shown without waiting for real records. |
| `reports/` | `weekly_summary.txt` | Output of the program (generated, can be deleted and recreated). |
| `docs/` | `PROJECT_DETAILS.md` | Documentation. |

`build/` (created by `make` or CMake) and the compiled programs are generated files, not part of the source.

---

## 7. File-by-file explanation

The program is split into **modules**, each with one job. They depend on each other only in one direction:

```
Utils  <-  FileHandler
Utils  <-  Tracker (holds dishes, records and the menu)  <-  Analyzer  <-  Forecaster / Optimizer  <-  Report  <-  main
```

### 7.1 Header files (`include/`)

| File | Purpose |
|---|---|
| `Utils.h` | Helper functions: trimming and splitting strings, converting text to numbers safely, date arithmetic and validation, weekday calculation and weekday-name parsing, meal-name checking (Breakfast, Lunch, Dinner). |
| `Dish.h` | `struct Dish`: name, category (Main, Side, Beverage, Milk for breakfast; Staple, Dal, Dry, Gravy, Side, Dessert for lunch and dinner), the meals it can be served at, cost per kg, portion size in grams, calories and protein per portion. Method `costPerPlate()` = cost per kg x portion / 1000. Method `servedAt(meal)`. |
| `WasteRecord.h` | `struct WasteRecord` (one dish, one meal, one day: cooked, leftover and plate waste in kg) with helper methods `wasteKg()`, `servedKg()`, `consumedKg()`, `wastePercent()`; and `struct Attendance` (date, meal, expected and actual headcount). |
| `Menu.h` | `struct MenuEntry`: one line of the weekly menu (weekday, meal, slot, dish, and whether it is a *choice* dish). |
| `FileHandler.h` | `class FileHandler`: static functions to load and save the four CSV files. |
| `Tracker.h` | `class Tracker`: keeps all dishes, waste records, attendance rows and the weekly menu in memory. Adding or deleting records goes through it, so every record is validated. Also answers "what is on the menu on this weekday at this meal?" and checks the menu for mistakes. |
| `Analyzer.h` | `struct Summary` (totals for any group), `struct DishStats` (per-dish behaviour), `class Analyzer` (grouping and ranking). |
| `Forecaster.h` | `struct Point`, `struct Forecast`, `class Forecaster` (three predictors and automatic method selection). |
| `Optimizer.h` | `CookPlan`, `MenuWeights`, `MenuOption`, `class Optimizer` (quantity planning, dish scoring, menu search, weekly plan). |
| `Report.h` | Printing functions in `namespace Report` (tables, bar chart, forecasts, cooking plans, menus, the weekly mess menu); they write to any `std::ostream`, so the same code prints on the screen and into a file. |

### 7.2 Source files (`src/`)

**`Utils.cpp`**
- `trim`, `split`, `toLower`: string tools. `split` keeps a trailing empty field so that a missing last column is detected.
- `parseDouble`, `parseInt`: use `strtod`/`strtol` and reject text such as `12abc`, so wrong input never silently becomes 0.
- `isValidDate`: checks the pattern `YYYY-MM-DD`, the month range and the number of days in the month, including leap years (so `2026-02-29` is rejected).
- `daysFromCivil` / `civilFromDays`: convert a date to a day number and back (public-domain algorithm by Howard Hinnant). This makes `addDays` and `weekdayOf` easy and correct without any library.
- `weekdayOf`: 1970-01-01 was a Thursday, so `weekday = (days + 3) mod 7` with Monday = 0.
- `canonicalMeal`: accepts "lunch", " LUNCH " and returns "Lunch"; returns an empty string for an invalid meal.

**`FileHandler.cpp`**
- Reads a CSV file line by line, skips the header line, blank lines and lines beginning with `#`.
- Checks the number of columns, that numbers really are numbers, dates are real dates, meals are valid, and that *leftover + plate waste is not more than cooked*.
- Throws `std::runtime_error` with the file name and line number when something is wrong (for example `waste_log.csv (line 12): invalid date '2026-02-30'`).
- Saving writes the header and the rows with fixed decimal places.
- `loadMenu` / `saveMenu` handle `menu.csv`: day names such as `Monday` or `mon` are accepted, and `type` must be `fixed` or `choice`.

**`Tracker.cpp`**
- `findDish`: case-insensitive search ("dalma" finds "Dalma").
- `addWaste`: checks date, meal, that the dish exists, quantities are not negative, cooked quantity is above zero and that waste cannot exceed what was cooked. It stores the dish under its official spelling.
- `addAttendance`: if a row for the same date and meal exists it is **updated**, not duplicated.
- `findByDate`, `findByDish`, `removeWaste`, `actualHeadcount`, `lastDate`.
- `menuFor(weekday, meal)` returns the menu lines; `dishesFor(weekday, meal)` returns the distinct dish names (used for cooking plans); `menuProblems()` reports menu dishes that are missing from `dishes.csv` or not listed for that meal.

**`Analyzer.cpp`**
- `overall`, `byDish`, `byMeal`, `byWeekday`, `byDate`: each loops over all records and adds them into a `Summary`, using `std::map` as a "dictionary" from group name to totals.
- `topWasted(n, byCost)`: copies the per-dish map into a vector and sorts it with `std::sort` and a lambda.
- `dishStats`: for every dish computes *acceptance* (share of cooked food actually eaten), *average cooked quantity per serving day*, and *cooked/served kg per student*. The per-student figures use only days whose headcount is known.

**`Forecaster.cpp`**
- `series(meal)`: the actual headcounts of one meal, sorted by date.
- Three predictors (see section 10.1) and `forecast`, which tests all three on the past and picks the most accurate.

**`Optimizer.cpp`**
- `dishScores`: one score between 0 and 1 for every dish.
- `bestMenu`: exhaustive search for the best combination of dishes.
- `weeklyPlan`: seven-day plan with rotation.
- `planQuantities`: kilograms to cook for each dish.

**`Report.cpp`**
- Formatted tables (`std::setw`, `std::left`, `std::right`, `std::fixed`), a text bar chart made of `#` characters, forecast lines, cooking plans, menus, and `writeFullReport` which assembles the whole report.
- `describeMeal`, `printMenuForDay`, `printWeeklyMenu` print the mess menu grouped by slot (choice dishes are written as `(A / B)`).

**`main.cpp`**
- Reads command-line options (`--data`, `--report`, `--out`).
- Loads the CSV files (shows a friendly error if something is wrong) and prints a warning for every menu mistake.
- Menu 12 (cooking plan) fetches the dishes of the scheduled menu automatically; menu 16 shows the mess menu.
- Shows the menu in a loop. All typed input is validated: invalid numbers and dates are asked again; if input ends (Ctrl+D / Ctrl+Z) the program saves and exits.
- On exit (menu 0) saves `waste_log.csv` and `attendance.csv`. `dishes.csv` and `menu.csv` are edited by hand and are left untouched, so the notes written in them are kept.

### 7.3 Test files (`tests/`)

| File | What it checks |
|---|---|
| `test_helpers.h` | Tiny `CHECK` and `CHECK_NEAR` macros and a `finish()` function (no external test library needed). |
| `test_utils.cpp` | Date validation (leap years), weekday of known dates, `addDays` across month and year ends, splitting, trimming, number parsing, meal names. |
| `test_filehandler.cpp` | Loading, saving and reloading give the same data; comments and blank lines are skipped; bad files (missing file, impossible date, waste larger than cooked, missing column, invalid meal) are rejected. |
| `test_analyzer.cpp` | Totals, waste percent, cost of waste, rankings, per-student statistics, and the Tracker's validation rules. |
| `test_forecaster.cpp` | With a perfect weekday pattern the weekday method wins with zero error; fallback with too little history; empty series. |
| `test_menu.cpp` | Loading and saving `menu.csv` (case-insensitive day names, choice dishes), rejection of bad menu files, `menuFor`, `dishesFor`, and detection of menu mistakes. |
| `test_optimizer.cpp` | Cooking quantity and saving arithmetic, no saving claimed without history, budget limit, calorie minimum, exclusion of dishes, score range, weekly plan variety. |

### 7.4 Tool (`tools/`)

**`generate_sample_data.cpp`** reads `data/dishes.csv` and `data/menu.csv` (the real mess menu) and simulates four weeks of cooking, headcount and waste for exactly those dishes. It writes `waste_log.csv` and `attendance.csv` (see section 11).

### 7.5 Build files

- `Makefile`: simple build, `make`, `make test`, `make clean`.
- `CMakeLists.txt`: builds a library `messlib` (everything except `main`), the program, the generator and the six tests (registered with `ctest`).

---

## 8. Data files

All files are CSV (comma separated). The first line is the header. Blank lines and lines starting with `#` are ignored (they can hold notes).

### 8.1 `data/menu.csv`: the mess weekly menu (real data)

This file is the **IIIT Bhubaneswar Ground Floor Mess menu**, typed in from the menu photograph. One row = one dish on one slot of one meal on one weekday.

| Column | Meaning | Rule |
|---|---|---|
| `day` | Monday ... Sunday (3-letter forms such as `Mon` also work) | valid weekday |
| `meal` | Breakfast, Lunch or Dinner | valid meal |
| `slot` | Where the dish sits on the menu: `Items` (breakfast), `Common`, `Dry`, `Gravy`, `Dessert`, `Compulsory` | not empty |
| `dish` | Dish name | must exist in `dishes.csv` |
| `type` | `fixed` = everyone gets it; `choice` = students pick **one** of the choice dishes of that slot | `fixed` or `choice` |

Example: `Tuesday,Dinner,Gravy,Egg Curry,choice` and `Tuesday,Dinner,Gravy,Mix Veg,choice` mean "Egg Curry / Mix Veg".

**How the photograph was converted**

- "RICE & ROTI + DALMA + PAPAD + CURD" became five rows (Rice, Roti, Dalma, Papad, Curd), because each of them is cooked, served and wasted separately.
- "CHICKEN / VEG BIRYANI", "EGG CURRY (2 PC) / MIX VEG", "MUTTON / KADHAI PANEER", "FISH CURRY / VEG NAVRATNA", "JALEBI / SUJI HALWA" and similar alternatives became `choice` rows.
- Quantity notes on the menu (2 PC, 75 GM paneer, 100 GM chicken, 70 ML tea, 150 ML milk) were used to choose the portion sizes in `dishes.csv`. "(ALOO LESS)" remarks were ignored.
- "TEA / COFFEE" and "BREAD WITH JAM / BUTTER" and "PASTA / MACARONI" are each kept as one dish.
- **Assumption:** the note under the menu ("Each are compulsory: 1 egg / 2 banana, tea/coffee, milk, pickle") does not say at which meal. Tea/Coffee and Milk are on the breakfast cells of the menu. Egg/Banana was attached to **Breakfast** and Pickle to **Lunch and Dinner** (slot `Compulsory`). Edit `menu.csv` if your mess does it differently.

The weekly menu as stored:

| Day | Breakfast (all with Tea/Coffee + Milk) |
|---|---|
| Monday | Chole Bhature (2 pc) |
| Tuesday | Aloo Paratha (1 pc) + Tomato Sauce |
| Wednesday | Pasta/Macaroni + Tomato Sauce + Bread with Jam/Butter |
| Thursday | Onion Uttapam + Chutney + Bread with Jam/Butter |
| Friday | Methi Puri + Sabji |
| Saturday | Idli Sambar (3 pc) + Chutney + Bread with Jam/Butter |
| Sunday | Upma + Ghugni + Bread with Jam/Butter |

| Day | Lunch: Common | Dry item | Gravy | Dessert |
|---|---|---|---|---|
| Monday | Rice, Roti, Dalma, Papad, Curd | Tomato Khatta | Soyabean Curry | none |
| Tuesday | Jeera Rice, Roti, Arhar Dal, Chips | Paneer Bhurji | Ghanta | none |
| Wednesday | Rice, Roti, Masoor Dal, Salad | Beans-Aloo Fry | Parwal Curry | none |
| Thursday | Rice, Roti, Dalma, Curd | Vegetable Fried Chips | Ghanta | none |
| Friday | Special lunch: (Chicken Biryani / Veg Biryani), Roti, Chips | Raita, Biryani Gravy (non-veg) | Kadhai Paneer (veg only) | none |
| Saturday | Rice, Roti, Moong Dal, Papad, Salad | Dahi Baigan (2 pc) | (Fish Curry / Veg Navratna) | none |
| Sunday | Fried Rice, Roti, Toor Dal, Curd | Karela Chips | (Chicken Masala / Paneer Masala) | Cone Ice-cream |

| Day | Dinner: Common | Dry item | Gravy | Dessert |
|---|---|---|---|---|
| Monday | Rice, Roti, Masoor Dal, Salad | Vegetable Chips | Veg Tadka | Pahala Rasgulla |
| Tuesday | Rice, Roti, Chana Dal, Chips | Aloo Banana Chips | (Egg Curry / Mix Veg) | none |
| Wednesday | Rice, Roti, Arhar Dal | Potato Fry | (Mutton Curry / Kadhai Paneer) | Gulab Jamun |
| Thursday | Rice, Roti, Moong Dal, Chips | Bhindi-Aloo | (Egg Chicken Bharta / Paneer Bhurji) | Kheer |
| Friday | Rice, Paratha, Arhar Dal | Dry Mix Veg | (Chicken Kasa / Paneer Bhurji) | none |
| Saturday | Jeera Rice, Roti, Dal Fry, Chips | Gobi Pakoda | Chhole Curry | (Jalebi / Suji Halwa) |
| Sunday | Rice, Roti, Arhar Dal, Salad, Chips | Aloo-Baigan-Tomato Bharta | Soyabean Chilli | none |

(Every lunch and dinner also has Pickle, and every breakfast has Egg/Banana, as explained above.)

### 8.2 `data/dishes.csv`: the dishes (menu items)

One row for each distinct dish used in `menu.csv` (71 dishes).

| Column | Meaning | Rule |
|---|---|---|
| `name` | Dish name | not empty, unique |
| `category` | Role of the dish: `Main` (breakfast item), `Side`, `Beverage`, `Milk` (breakfast); `Staple`, `Dal`, `Dry`, `Gravy`, `Side`, `Dessert` (lunch and dinner) | text |
| `meal` | Meals where the dish is served, separated by `;` (e.g. `Lunch;Dinner`); `Any` = every meal | at least one |
| `cost_per_kg` | Cost of one kg of cooked dish, in rupees | above zero |
| `portion_g` | Standard portion for one student, grams (ml for drinks) | above zero |
| `calories` | Calories in one portion | zero or more |
| `protein_g` | Protein in one portion, grams | zero or more |

Example: `Dalma,Dal,Lunch,80,150,170,9`

**Important: the dish names and the meals are real (from the menu), but `cost_per_kg`, `portion_g`, `calories` and `protein_g` are my ESTIMATES.** The menu does not give prices or nutrition. Replace them with the mess's real figures (purchase cost per kg of cooked food, the real serving size, and nutrition values) before you trust the rupee figures.

`Tracker::menuProblems()` checks that every dish on the menu exists here and is listed for the meal where it is served; the program prints a warning at start-up if not.

### 8.3 `data/waste_log.csv`: the daily waste log (sample data is simulated)

| Column | Meaning | Rule |
|---|---|---|
| `date` | Day of the meal | real date, `YYYY-MM-DD` |
| `meal` | Breakfast, Lunch or Dinner | valid meal |
| `dish` | Dish name | must exist in `dishes.csv` |
| `cooked_kg` | Total food prepared | above zero |
| `leftover_kg` | Food never served (left in vessels) | zero or more |
| `plate_waste_kg` | Food served but thrown away from plates | zero or more |

Rule: `leftover_kg + plate_waste_kg <= cooked_kg`.

Example: `2026-09-07,Lunch,Dalma,38.50,9.20,2.10`

### 8.4 `data/attendance.csv`: headcount per meal (sample data is simulated)

| Column | Meaning |
|---|---|
| `date` | Day |
| `meal` | Meal |
| `expected` | Number the kitchen planned for |
| `actual` | Number of students who actually ate |

Example: `2026-09-07,Lunch,320,316`

Adding a row for a date and meal that already exists **replaces** it.

---

## 9. Key measurements and formulas

For one waste record:

```
waste        = leftover + plate waste
served       = cooked - leftover
consumed     = served - plate waste
waste %      = waste / cooked x 100
cost of waste = waste x cost_per_kg of the dish        (rupees)
cost per plate = cost_per_kg x portion_g / 1000        (rupees)
```

For a dish over many days (`DishStats`):

```
acceptance        = (cooked - waste) / cooked              (0 to 1, "how much of it is eaten")
avg cooked        = total cooked / number of serving days
served per head   = total served kg / total students who ate     (real demand per student)
cooked per head   = total cooked kg / total students who ate
```

Why *leftover* and *plate waste* are tracked separately: leftover (never served) is the part the kitchen can avoid by cooking less. Plate waste is the part students leave; it is reduced by better dishes, smaller first servings and awareness. The cooking plan uses **served per head** (real demand), not cooked per head, so it removes avoidable leftover without under-feeding anyone.

---

## 10. Algorithms in detail

### 10.1 Headcount forecast (`Forecaster`)

Goal: predict how many students will eat a given meal on a given date.

Three simple methods are implemented:

1. **Moving average (7 days)**: the average of the last 7 observed headcounts. Good when the headcount is stable.
2. **Exponential smoothing** (alpha = 0.4): `level = 0.4 x new + 0.6 x old level`. Recent days count more than old days. Good when the level drifts.
3. **Weekday average**: the average of the last 4 observations of the **same weekday** (for example the last 4 Fridays). Good when the weekday matters (Saturday dinners and Sunday breakfasts are always smaller).

**Choosing the best method automatically.** For each method the program does a *walk-forward test*: starting from day 8, it predicts each day using only the days before it and measures the absolute error. The average of these errors is the **MAE** (mean absolute error). The method with the smallest MAE is used for the real prediction and the report shows its MAE in students.

If fewer than 8 days of data exist, the methods cannot be compared, so the plain moving average is used.

Complexity: O(n^2) for n days (it copies the history for each test), which is instant for months of data.

### 10.2 Cooking quantity plan (`Optimizer::planQuantities`)

Input: date, meal, list of dishes on the menu.

```
heads        = forecast headcount (rounded)
per_head     = served per head of that dish (from history)
cook_kg      = heads x per_head x (1 + safety buffer)        (buffer = 5 %)
usual_kg     = average quantity normally cooked for that dish
saved_kg     = usual_kg - cook_kg
saved_rupees = saved_kg x cost_per_kg
```

- If a dish has **no history**, the standard portion is used and **no saving is claimed**.
- The safety buffer protects against a shortage; it can be changed in the code (`Optimizer` constructor).
- In the application (menu 12) and in the report, the list of dishes is taken **from the scheduled mess menu** for the weekday of the chosen date. For a `choice` slot every alternative is planned: each dish's demand per student (`per_head`) already reflects the share of students who pick it, because it is learned from history.

### 10.3 Dish score (`Optimizer::dishScores`)

Each dish gets a score between 0 and 1:

```
score = 0.45 x acceptance + 0.25 x cost score + 0.30 x nutrition score
```

- **Acceptance**: share of the cooked food that is eaten (from history). A dish without history gets a neutral 0.5.
- **Cost score**: 1 for the cheapest plate in its category and 0 for the most expensive one in the category (min-max scaling).
- **Nutrition score**: protein per portion, scaled the same way inside the category.

Dishes are compared **only with dishes of the same category** (a dessert is not compared with a curry). The weights are in `MenuWeights` and can be changed.

### 10.4 Best menu (`Optimizer::bestMenu`)

The optimizer answers: *"if the mess could choose its dishes freely from the dishes it already cooks, which combination would be best?"* It follows the slot structure of the mess menu. Each meal has a structure:

| Meal | Must have one of | May have one of |
|---|---|---|
| Breakfast | Main, Beverage (tea/coffee), Milk | Side |
| Lunch and Dinner | Staple, Dal, Dry, Gravy | Side, Dessert |

(Staple = rice, roti, biryani...; Dal = the dal of the Common slot; Dry = the dry item; Gravy = the gravy slot. Special accompaniments such as Biryani Gravy and Raita are `Side` dishes.)

The program lists all dishes that are served at the meal for each category and tries **every combination** (an exhaustive search; the budget limit removes most branches early, so it runs in a few milliseconds). A combination is accepted only if

- its total **cost per plate is within the budget**, and
- its total **calories reach the minimum**.

Among accepted combinations, the one with the **highest total dish score** wins. Branches that already exceed the budget are skipped early (pruning).

Defaults (rupees per plate / minimum kcal): Breakfast 45 / 450, Lunch and Dinner 75 / 650. In menu 13 the user can type different limits.

Note: the search picks one staple (for example Roti), whereas the real mess serves Rice **and** Roti together; and it does not know special-day rules. Treat it as a suggestion to compare with the scheduled menu, not as a replacement for it.

### 10.5 Weekly menu plan (`Optimizer::weeklyPlan`)

Choosing the best menu every day would give the same menu every day. So for each of the 7 days:

1. try to avoid the main item, staple, dal and gravy used on the **previous 2 days**;
2. if no menu is possible, avoid only **yesterday's**;
3. if still impossible, allow anything.

Tea, milk, sides and desserts may repeat. The result is a rotation of about three different menus. (This is the optimizer's suggestion, menu 14; the real weekly menu is shown with menu 16.)

### 10.6 Cooking plan for the scheduled menu

`Tracker::dishesFor(weekday, meal)` returns the distinct dishes of the scheduled menu (for example Tuesday Dinner: Rice, Roti, Chana Dal, Chips, Aloo Banana Chips, Egg Curry, Mix Veg, Pickle). The report (section 8) prints, for each meal of the next day, the scheduled menu and the cooking plan of those dishes. This is how the real mess menu feeds the optimization.

### 10.7 How the sample "saving" is estimated

The report adds up the `saved_rupees` of the dishes of the scheduled menu for the next day. This is an **estimate**: it assumes the forecast is right, that the dish is eaten at its historical rate, and that the "usual quantity" is the average quantity cooked for that dish in the past (for a dish served at both lunch and dinner, such as Rice, it is the average of both).

---

## 11. The sample data (important: part of it is simulated)

| File | Source |
|---|---|
| `menu.csv` | **Real**: typed from the IIIT Bhubaneswar Ground Floor Mess menu photograph |
| `dishes.csv` | Dish names and meals are **real** (from the menu); costs, portions, calories and protein are **estimates** |
| `waste_log.csv` | **Simulated** by `tools/generate_sample_data.cpp` |
| `attendance.csv` | **Simulated** by `tools/generate_sample_data.cpp` |

The generator reads `menu.csv` and `dishes.csv` and, with a fixed random seed (2026), produces the same numbers every time. **The waste and headcount numbers are not real measurements.** They exist so that every feature can be demonstrated on the real menu.

What the simulation assumes:

- 28 days (2026-09-07 to 2026-10-04), 3 meals a day, a hostel of about 350 students.
- Every dish on a day's menu is cooked, including both alternatives of a `choice` slot.
- The kitchen always plans for the **same headcount** (230 breakfast, 320 lunch, 300 dinner).
- The real headcount depends on the weekday (Sunday breakfast about 70 %, Saturday lunch about 80 %, Saturday dinner about 75 %, Friday special lunch about 112 %) plus random variation of about 5 %.
- Each dish has an *uptake* (share of students who take it) and a *plate waste rate*. They come from the dish category (for example Staple 95 %, Dry item 65 %, Side 60 %), with some invented exceptions (Chicken Biryani 60 %, Veg Biryani 35 %, Mutton Curry 35 %, Karela Chips 35 %, Cone Ice-cream 95 %, Upma 50 % with high plate waste).
- The kitchen cooks for the planned headcount and only partly adjusts for how popular a dish is.

Result: 600 waste records and 84 attendance rows (28 days x 3 meals).

**In your project report, clearly say that the waste and attendance data are simulated and that the costs and nutrition values are estimates**, and that the method works with real records once they are collected.

---

## 12. Sample results

These figures come from the simulated waste data on the real menu, so they only illustrate the output format and the method.

**Overall (28 days):** 18,209.5 kg cooked; 6,201.7 kg wasted (5,411.3 kg leftover + 790.4 kg plate waste) = **34.1 %** waste, worth about **Rs 6,95,295** (using the estimated costs).

**By meal:** Breakfast 36.7 %, Lunch 34.9 %, Dinner 31.8 %.

**By weekday:** Saturday 44.9 % and Sunday 39.3 % are the worst (fewer students eat, but the same quantity is cooked); Monday 27.7 % and Thursday 29.0 % are the best.

**Highest waste percentages:** Karela Chips 67.8 %, Upma 66.6 %, Veg Navratna 62.5 %, Fish Curry 62.0 %, Salad 62.0 %.

**Most waste in kg:** Rice 632.6 kg, Roti 424.8 kg, Milk 406.8 kg (these are cooked in the largest quantities).

**Highest cost of waste:** Paneer Bhurji Rs 46,451, Mutton Curry Rs 38,682, Kadhai Paneer Rs 37,764. Expensive non-veg and paneer dishes cost the most when they are wasted.

**Forecast for Monday 2026-10-05:** Breakfast about 227 students (weekday average, average error 8.7), Lunch about 309 (weekday average, error 13.8), Dinner about 278 (moving average, error 19.6). The method with the smallest past error was chosen automatically for each meal.

**Cooking plan for the scheduled menu on Monday 2026-10-05:**

| Meal | Scheduled menu | Estimated saving |
|---|---|---|
| Breakfast | Chole Bhature + Tea/Coffee + Milk + Egg/Banana | Rs 1,850 |
| Lunch | Rice + Roti + Dalma + Papad + Curd + Tomato Khatta + Soyabean Curry + Pickle | Rs 3,843 |
| Dinner | Rice + Roti + Masoor Dal + Salad + Vegetable Chips + Veg Tadka + Pahala Rasgulla + Pickle | Rs 4,844 |

Total estimated saving for that day: **Rs 10,538** compared with the usual cooking quantities of those dishes (for example the Lunch plan cooks 197 kg instead of the usual 250 kg).

**Optimizer suggestion (best menu within budget):** Breakfast: Chole Bhature, Tea/Coffee, Milk, Ghugni (Rs 44.9). Lunch: Roti, Masoor Dal, Tomato Khatta, Soyabean Curry, Chips, Cone Ice-cream (Rs 51.0). Dinner: Roti, Chana Dal, Vegetable Chips, Soyabean Chilli, Chips, Kheer (Rs 52.5).

The full printout, including the weekly menu, is in `reports/weekly_summary.txt`.

---

## 13. Testing

Run `make test` (or `ctest`). Six test programs run automatically and print `All checks passed` when everything is correct.

- Tests use small data sets made by hand, so the expected answers can be calculated by hand (for example a dish with 10 kg cooked, 2 kg leftover and 1 kg plate waste must give 30 % waste).
- Bad input is tested deliberately (impossible dates, waste larger than cooked, missing columns).
- The tests do not depend on `data/`, so they never change your real data. The file-handling test writes temporary files and deletes them.

Manual test checklist: run the program, add a record with a wrong date (it must be refused), add a valid record, view it, delete it, forecast, plan, generate the report.

---

## 14. C++ concepts used

- **Classes and structs**, encapsulation (`Tracker` controls how data is changed), const-correct methods.
- **Header / source separation** and include guards (`#pragma once`).
- **STL containers**: `std::vector`, `std::map`, `std::set`, `std::pair`.
- **STL algorithms**: `std::sort`, `std::transform`, `std::min`, `std::max`.
- **Lambda functions** (sorting rules, category filters).
- **File streams** (`std::ifstream`, `std::ofstream`) and `std::stringstream`.
- **Exceptions** (`std::runtime_error`) for bad files.
- **Function pointers** (the forecasting methods are stored in an array and tested in one loop).
- **Recursion** (the menu search).
- **Namespaces** (`utils`, `Report`).
- **Output formatting** with `<iomanip>`.
- **`std::filesystem`** to create the report folder; **`<random>`** in the generator.
- **Unit testing** with a small hand-made macro framework.

---

## 15. Using real mess data

The menu is already the real IIIT Bhubaneswar Ground Floor Mess menu. To make the whole project real:

1. **Edit `data/dishes.csv`**: replace the estimated cost per kg, portion size, calories and protein with the mess's real figures (ask the mess committee for purchase costs and serving sizes).
2. **Keep `data/menu.csv` up to date**: if the mess changes its weekly menu, edit the rows (a dish used there must exist in `dishes.csv`; the program warns about mistakes at start-up).
3. **Delete the simulated records**: replace `waste_log.csv` and `attendance.csv` with only the header line, then collect real data.
4. **Collect daily records.** After each meal, someone weighs (a) the food cooked, (b) the food left in the vessels, (c) the food thrown from plates (a weighed bin or a sampling method). Enter them with menu 2.
5. **Count the students** at each meal (the mess register or card swipes) and enter them with menu 3.
6. **Wait for enough data**: at least 2 weeks for a first forecast and cooking plan, 4 weeks or more for reliable results.
7. Use menus 8 to 14 and the report.
8. After following the plan for a few weeks, **compare the waste % before and after**. That comparison is the proof that the optimization works.

Tip: you can type records into a spreadsheet and save as CSV with the same columns; the program reads them directly.

---

## 16. Limitations and future work

**Limitations**

- The waste and headcount in the sample results are simulated; the costs, portions and nutrition values of the dishes are estimates.
- The menu is one weekly cycle repeated every week; the program does not model different menus in different weeks, or festival and special-event menus.
- The weekday of the menu is used as it is; the "(ALOO LESS)" style remarks and the veg / non-veg split of students are not modelled separately.
- The meal to which the compulsory items (Egg/Banana, Pickle) belong is an assumption.
- The optimizer does not know that Rice and Roti are served together, that some dishes are special-day items, or which dishes go well together; its output is a suggestion to compare with the scheduled menu.
- The mess serves only breakfast, lunch and dinner; there is no snacks meal in the project.
- Forecasting uses simple methods; it does not know about exams, festivals, holidays or weather.
- Demand per student is an average; it does not model individual students.
- The menu search uses protein and calories only; it does not check variety of vitamins, allergies or religious and regional preferences.
- Plate waste measurement in real life needs effort; if it is not measured, set it to 0.
- Console interface only.

**Possible extensions**

- Add a holiday or event calendar to the forecast.
- Add a feedback or rating file and include student ratings in the dish score.
- Use linear programming to optimize a whole week's menu under a weekly budget and nutrition goals.
- Add a simple graphical interface (for example with Qt) or a web front-end written in C++.
- Export reports as CSV for spreadsheets.
- Add inventory and purchasing so that quantities cooked feed into quantities bought.

---

## 17. Possible viva / presentation questions

**Q1. Why did you split the code into so many files?**
Each file has one responsibility (reading files, storing data, analysis, forecasting, optimization, printing). This makes the code easier to test, understand and extend, and the dependency flows one way.

**Q2. What is the difference between leftover and plate waste?**
Leftover is food cooked but never served; the kitchen can avoid it by cooking less. Plate waste is food served but thrown away; it depends on food quality and portion size.

**Q3. How does the program decide how much to cook?**
It forecasts the headcount, multiplies it by the real served quantity per student of each dish (learned from history) and adds a 5 % safety buffer.

**Q4. How do you know your forecast is any good?**
Each method is tested on the past with a walk-forward test, and the mean absolute error is shown. The method with the lowest error is used.

**Q5. Why compare dishes only inside their category in the score?**
A dessert and a staple are not comparable on protein or price; comparing within the category gives a fair score.

**Q6. Is the menu search optimal?**
Yes, within its rules: it tries every combination that satisfies the budget and calorie limits and picks the highest score. It is exhaustive and the number of combinations is small.

**Q7. How do you stop wrong data from entering?**
Dates must be real, meals valid, the dish must exist, quantities cannot be negative and waste cannot exceed the quantity cooked. The CSV loader checks the same rules and reports the file and line number of an error.

**Q8. Is your data real?**
The demonstration data is simulated by `tools/generate_sample_data.cpp` with a fixed seed. The program is designed for real records, which can be put in the same CSV format.

**Q9. Why only the C++ standard library?**
It makes the project portable and easy to build with no installation; it also shows core C++ skills (STL, file streams, classes).

**Q10. How does the project use the real mess menu?**
The weekly menu (IIIT Bhubaneswar Ground Floor Mess) is stored in `menu.csv`. For a chosen date the program looks up the weekday and meal, takes the scheduled dishes (including both alternatives of a veg / non-veg choice), and plans how much of each to cook from the forecast headcount and the per-student demand learned from history.

**Q11. What did you assume that the menu did not say?**
The cost, portion size, calories and protein of every dish, and the meal at which the compulsory items (egg/banana, pickle) are served. These are listed in section 8 and are easy to change in the CSV files.

**Q12. What would you improve next?**
A holiday calendar in the forecast, student ratings in the dish score, real costs and nutrition values, and linear programming for weekly menu optimization.

---

## 18. Glossary

| Term | Meaning |
|---|---|
| Mess | A hostel or institutional canteen |
| Leftover | Cooked food that was never served |
| Plate waste | Served food that was thrown away |
| Waste % | Waste divided by cooked quantity, times 100 |
| Headcount | Number of students who eat a meal |
| Acceptance | Share of cooked food that is actually eaten |
| MAE | Mean absolute error: average size of the forecast mistakes |
| Walk-forward test | Predicting each past day using only the days before it |
| Safety buffer | Small extra quantity cooked to avoid running short |
| Exhaustive search | Trying all possible combinations and keeping the best |
| Pruning | Skipping combinations early when they already break a limit |
| CSV | Comma-separated values, a plain text table |
