#pragma once
// A tiny test helper so the project needs no external test library.
#include <cmath>
#include <iostream>

inline int g_failures = 0;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n";   \
            ++g_failures;                                                             \
        }                                                                             \
    } while (0)

#define CHECK_NEAR(a, b, eps) CHECK(std::fabs((a) - (b)) <= (eps))

inline int finish() {
    if (g_failures == 0) {
        std::cout << "All checks passed\n";
        return 0;
    }
    std::cout << g_failures << " check(s) FAILED\n";
    return 1;
}
