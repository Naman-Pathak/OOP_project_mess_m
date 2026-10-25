#pragma once
#include <string>
#include <vector>
#include "Tracker.h"

// One observation of the headcount time series.
struct Point {
    std::string date;
    int weekday = 0;
    double value = 0;
};

struct Forecast {
    double value = 0;       // predicted headcount
    std::string method;     // method that won the comparison
    double mae = -1;        // its average error in past tests (-1 = not measured)
};

class Forecaster {
public:
    explicit Forecaster(const Tracker& tracker) : tracker_(tracker) {}

    std::vector<Point> series(const std::string& meal) const;  // actual headcounts, sorted by date

    // Three simple predictors. 'history' is sorted by date; 'weekday' is the target day.
    static double movingAverage(const std::vector<Point>& history, int weekday);
    static double exponentialSmoothing(const std::vector<Point>& history, int weekday);
    static double weekdayAverage(const std::vector<Point>& history, int weekday);

    // Tests all three on the past (walk-forward) and uses the most accurate one.
    Forecast forecast(const std::string& meal, const std::string& targetDate) const;

private:
    const Tracker& tracker_;
};
