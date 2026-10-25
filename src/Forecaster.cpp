#include "Forecaster.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "Utils.h"

std::vector<Point> Forecaster::series(const std::string& meal) const {
    std::vector<Point> pts;
    for (const auto& a : tracker_.attendance()) {
        if (a.meal != meal) continue;
        Point p;
        p.date = a.date;
        p.weekday = utils::weekdayOf(a.date);
        p.value = a.actual;
        pts.push_back(p);
    }
    std::sort(pts.begin(), pts.end(), [](const Point& x, const Point& y) { return x.date < y.date; });
    return pts;
}

// Average of the last 7 days.
double Forecaster::movingAverage(const std::vector<Point>& h, int /*weekday*/) {
    if (h.empty()) return 0.0;
    const size_t n = std::min<size_t>(7, h.size());
    double sum = 0;
    for (size_t i = h.size() - n; i < h.size(); ++i) sum += h[i].value;
    return sum / static_cast<double>(n);
}

// Recent days count more than old days (alpha = 0.4).
double Forecaster::exponentialSmoothing(const std::vector<Point>& h, int /*weekday*/) {
    if (h.empty()) return 0.0;
    const double alpha = 0.4;
    double level = h[0].value;
    for (size_t i = 1; i < h.size(); ++i) level = alpha * h[i].value + (1.0 - alpha) * level;
    return level;
}

// Average of the last 4 same weekdays (e.g. the last 4 Fridays).
double Forecaster::weekdayAverage(const std::vector<Point>& h, int weekday) {
    double sum = 0;
    int count = 0;
    for (size_t i = h.size(); i-- > 0 && count < 4;) {
        if (h[i].weekday == weekday) {
            sum += h[i].value;
            ++count;
        }
    }
    return count > 0 ? sum / count : movingAverage(h, weekday);
}

namespace {
using Predictor = double (*)(const std::vector<Point>&, int);

// Walk-forward test: predict each day using only the days before it.
double meanAbsError(Predictor f, const std::vector<Point>& pts, size_t minHistory) {
    double total = 0;
    int n = 0;
    for (size_t i = minHistory; i < pts.size(); ++i) {
        std::vector<Point> history(pts.begin(), pts.begin() + static_cast<std::ptrdiff_t>(i));
        total += std::fabs(f(history, pts[i].weekday) - pts[i].value);
        ++n;
    }
    return n > 0 ? total / n : std::numeric_limits<double>::infinity();
}
}  // namespace

Forecast Forecaster::forecast(const std::string& meal, const std::string& targetDate) const {
    Forecast result;
    const std::vector<Point> pts = series(meal);
    if (pts.empty()) {
        result.method = "no attendance data";
        return result;
    }
    const int wd = utils::weekdayOf(targetDate);
    const size_t minHistory = 7;
    if (pts.size() <= minHistory) {
        result.value = movingAverage(pts, wd);
        result.method = "Moving average (not enough history to compare methods)";
        return result;
    }
    struct Candidate { const char* name; Predictor fn; };
    const Candidate candidates[] = {{"Moving average (7 days)", &Forecaster::movingAverage},
                                    {"Exponential smoothing", &Forecaster::exponentialSmoothing},
                                    {"Weekday average", &Forecaster::weekdayAverage}};
    double bestMae = std::numeric_limits<double>::infinity();
    const Candidate* best = &candidates[0];
    for (const auto& c : candidates) {
        const double mae = meanAbsError(c.fn, pts, minHistory);
        if (mae < bestMae) { bestMae = mae; best = &c; }
    }
    result.value = best->fn(pts, wd);
    result.method = best->name;
    result.mae = bestMae;
    return result;
}
