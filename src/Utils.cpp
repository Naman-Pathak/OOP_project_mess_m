#include "Utils.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace utils {

std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    std::stringstream ss(s);
    while (std::getline(ss, cur, delim)) out.push_back(trim(cur));
    if (!s.empty() && s.back() == delim) out.push_back("");  // keep a trailing empty field
    return out;
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string fmt(double value, int precision) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(precision) << value;
    return os.str();
}

std::string repeat(char c, int count) {
    return count > 0 ? std::string(static_cast<size_t>(count), c) : std::string();
}

bool parseDouble(const std::string& s, double& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    char* end = nullptr;
    double v = std::strtod(t.c_str(), &end);
    if (end == nullptr || *end != '\0' || !std::isfinite(v)) return false;
    out = v;
    return true;
}

bool parseInt(const std::string& s, int& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    char* end = nullptr;
    long v = std::strtol(t.c_str(), &end, 10);
    if (end == nullptr || *end != '\0') return false;
    out = static_cast<int>(v);
    return true;
}

static bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

static int daysInMonth(int y, int m) {
    static const int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeap(y)) ? 29 : d[m - 1];
}

bool isValidDate(const std::string& s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    int y = std::stoi(s.substr(0, 4));
    int m = std::stoi(s.substr(5, 2));
    int d = std::stoi(s.substr(8, 2));
    if (y < 1900 || y > 2200 || m < 1 || m > 12) return false;
    return d >= 1 && d <= daysInMonth(y, m);
}

// Civil-calendar <-> day-number conversion (Howard Hinnant's public-domain algorithms).
long daysFromCivil(int y, int m, int d) {
    y -= (m <= 2) ? 1 : 0;
    const long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = static_cast<unsigned>((153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1);
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long>(doe) - 719468;
}

void civilFromDays(long z, int& y, int& m, int& d) {
    z += 719468;
    const long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long yy = static_cast<long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    y = static_cast<int>(yy + (m <= 2 ? 1 : 0));
}

long dateToDays(const std::string& date) {
    return daysFromCivil(std::stoi(date.substr(0, 4)), std::stoi(date.substr(5, 2)),
                         std::stoi(date.substr(8, 2)));
}

std::string daysToDate(long days) {
    int y, m, d;
    civilFromDays(days, y, m, d);
    std::ostringstream os;
    os << std::setfill('0') << std::setw(4) << y << '-' << std::setw(2) << m << '-'
       << std::setw(2) << d;
    return os.str();
}

std::string addDays(const std::string& date, int n) { return daysToDate(dateToDays(date) + n); }

int weekdayOf(const std::string& date) {
    // 1970-01-01 was a Thursday; with Monday = 0 that is index 3.
    return static_cast<int>(((dateToDays(date) + 3) % 7 + 7) % 7);
}

std::string weekdayName(int weekday) {
    static const char* names[] = {"Monday", "Tuesday", "Wednesday", "Thursday",
                                  "Friday", "Saturday", "Sunday"};
    return (weekday >= 0 && weekday < 7) ? names[weekday] : "?";
}

int weekdayFromName(const std::string& name) {
    const std::string key = toLower(trim(name));
    if (key.size() < 3) return -1;
    for (int i = 0; i < 7; ++i) {
        const std::string full = toLower(weekdayName(i));
        if (key == full || key == full.substr(0, 3)) return i;
    }
    return -1;
}

const std::vector<std::string>& mealNames() {
    static const std::vector<std::string> meals = {"Breakfast", "Lunch", "Dinner"};
    return meals;
}

std::string canonicalMeal(const std::string& input) {
    const std::string key = toLower(trim(input));
    for (const auto& m : mealNames())
        if (toLower(m) == key) return m;
    return "";
}

}  // namespace utils
