#include "Analyzer.h"

#include <algorithm>

#include "Utils.h"

namespace {
void finalize(Summary& s) { s.wastePct = s.cookedKg > 0 ? s.wasteKg / s.cookedKg * 100.0 : 0.0; }
}  // namespace

void Analyzer::accumulate(Summary& s, const WasteRecord& r) const {
    s.records += 1;
    s.cookedKg += r.cookedKg;
    s.leftoverKg += r.leftoverKg;
    s.plateKg += r.plateKg;
    s.wasteKg += r.wasteKg();
    if (const Dish* d = tracker_.findDish(r.dish)) s.costOfWaste += r.wasteKg() * d->costPerKg;
}

Summary Analyzer::overall() const {
    Summary s;
    for (const auto& r : tracker_.waste()) accumulate(s, r);
    finalize(s);
    return s;
}

std::map<std::string, Summary> Analyzer::byDish() const {
    std::map<std::string, Summary> m;
    for (const auto& r : tracker_.waste()) accumulate(m[r.dish], r);
    for (auto& kv : m) finalize(kv.second);
    return m;
}

std::map<std::string, Summary> Analyzer::byMeal() const {
    std::map<std::string, Summary> m;
    for (const auto& r : tracker_.waste()) accumulate(m[r.meal], r);
    for (auto& kv : m) finalize(kv.second);
    return m;
}

std::map<int, Summary> Analyzer::byWeekday() const {
    std::map<int, Summary> m;
    for (const auto& r : tracker_.waste()) accumulate(m[utils::weekdayOf(r.date)], r);
    for (auto& kv : m) finalize(kv.second);
    return m;
}

std::map<std::string, Summary> Analyzer::byDate() const {
    std::map<std::string, Summary> m;
    for (const auto& r : tracker_.waste()) accumulate(m[r.date], r);
    for (auto& kv : m) finalize(kv.second);
    return m;
}

std::vector<std::pair<std::string, Summary>> Analyzer::topWasted(size_t n, bool byCost) const {
    std::vector<std::pair<std::string, Summary>> v;
    for (const auto& kv : byDish()) v.push_back(kv);
    std::sort(v.begin(), v.end(), [byCost](const auto& a, const auto& b) {
        return byCost ? a.second.costOfWaste > b.second.costOfWaste
                      : a.second.wasteKg > b.second.wasteKg;
    });
    if (v.size() > n) v.resize(n);
    return v;
}

std::vector<DishStats> Analyzer::dishStats() const {
    struct Acc {
        double cooked = 0, leftover = 0, plate = 0, cookedKnown = 0, servedKnown = 0;
        long heads = 0;
        int records = 0;
    };
    std::map<std::string, Acc> acc;
    for (const auto& r : tracker_.waste()) {
        Acc& a = acc[r.dish];
        a.cooked += r.cookedKg;
        a.leftover += r.leftoverKg;
        a.plate += r.plateKg;
        a.records += 1;
        const int heads = tracker_.actualHeadcount(r.date, r.meal);
        if (heads > 0) {  // per-head figures only use days where the headcount is known
            a.cookedKnown += r.cookedKg;
            a.servedKnown += r.servedKg();
            a.heads += heads;
        }
    }
    std::vector<DishStats> out;
    for (const auto& kv : acc) {
        const Acc& a = kv.second;
        DishStats d;
        d.dish = kv.first;
        d.records = a.records;
        d.cookedKg = a.cooked;
        d.wasteKg = a.leftover + a.plate;
        d.wastePct = a.cooked > 0 ? d.wasteKg / a.cooked * 100.0 : 0.0;
        d.acceptance = a.cooked > 0 ? (a.cooked - d.wasteKg) / a.cooked : 0.0;
        d.avgCookedKg = a.records > 0 ? a.cooked / a.records : 0.0;
        if (a.heads > 0) {
            d.hasPerHead = true;
            d.cookedPerHeadKg = a.cookedKnown / static_cast<double>(a.heads);
            d.servedPerHeadKg = a.servedKnown / static_cast<double>(a.heads);
        }
        out.push_back(d);
    }
    return out;
}
