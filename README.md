# Mess Food Waste Tracking and Meal Optimization (C++)

A console application, written only in standard C++17, that

1. **tracks** food cooked, left over and thrown away in a mess (canteen), meal by meal,
2. **analyzes** where the waste and the money loss are (by dish, meal, weekday),
3. **forecasts** how many students will eat, and
4. **optimizes** how much to cook and which menu to serve (low waste, low cost, good nutrition).

No external libraries and no other languages are used. Data is kept in plain CSV files.

> **Menu:** `data/menu.csv` and `data/dishes.csv` hold the real weekly menu of the **IIIT Bhubaneswar Ground Floor Mess**
> (breakfast, lunch, dinner). The costs, portions and nutrition values in `dishes.csv` are estimates.
> **Sample data:** `data/waste_log.csv` and `data/attendance.csv` are **simulated** (by `tools/generate_sample_data.cpp`)
> so the project works out of the box. Replace them with your mess's real records.

## Quick start

```bash
# Option A: Makefile (Linux / macOS / MinGW)
make                       # builds mess_tracker and generate_sample_data
make test                  # builds and runs all unit tests
./mess_tracker             # interactive menu
./mess_tracker --report    # writes reports/weekly_summary.txt and exits

# Option B: CMake
mkdir build && cd build && cmake .. && cmake --build . && ctest
cd .. && ./build/mess_tracker

# Option C: one g++ command
g++ -std=c++17 -Iinclude src/*.cpp -o mess_tracker
```

**Windows `.exe`:** install a free C++ compiler (for example the WinLibs MinGW-w64 zip) and double-click `build_windows.bat`. It creates `mess_tracker.exe` and `generate_sample_data.exe` in the project folder.

Run the program **from the project folder** so it finds `data/`. To use another folder: `./mess_tracker --data my_data`.
To create fresh sample waste and attendance data for the menu: `./generate_sample_data` (writes into `data/`).

## Folders

| Folder | Purpose |
|---|---|
| `include/` | Header files: class and function declarations |
| `src/` | Implementations and `main.cpp` |
| `data/` | CSV files: the mess menu, the dishes, the waste log, the attendance |
| `tests/` | Unit tests (no external test library) |
| `tools/` | Sample-data generator |
| `reports/` | Reports written by the program |
| `docs/` | Full project explanation (`PROJECT_DETAILS.md`) |

Read **`docs/PROJECT_DETAILS.md`** for the complete explanation of every file, formula and algorithm.
