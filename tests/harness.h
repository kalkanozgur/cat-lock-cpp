#pragma once
// Küçük test altyapısı. Bilinçli olarak framework yok (CLAUDE.md §2).
#include <cstdio>
#include <iostream>
#include <vector>

namespace kk::test {

struct Case {
    const char* name;
    void (*fn)();
};

inline std::vector<Case>& cases() {
    static std::vector<Case> all;
    return all;
}

inline int failures = 0;

struct Registrar {
    Registrar(const char* name, void (*fn)()) { cases().push_back({name, fn}); }
};

inline int run_all() {
    int failed_cases = 0;
    for (const Case& c : cases()) {
        const int before = failures;
        c.fn();
        const bool ok = failures == before;
        failed_cases += ok ? 0 : 1;
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", c.name);
    }
    const int total = static_cast<int>(cases().size());
    std::printf("%d/%d test geçti\n", total - failed_cases, total);
    return failed_cases == 0 ? 0 : 1;
}

}  // namespace kk::test

#define TEST(name)                                                  \
    static void kk_test_##name();                                   \
    static const ::kk::test::Registrar kk_reg_##name{#name, kk_test_##name}; \
    static void kk_test_##name()

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            ++::kk::test::failures;                                        \
            std::printf("  %s:%d: CHECK(%s)\n", __FILE__, __LINE__, #expr); \
        }                                                                  \
    } while (0)

#define CHECK_EQ(a, b)                                                     \
    do {                                                                   \
        const auto& kk_a = (a);                                            \
        const auto& kk_b = (b);                                            \
        if (!(kk_a == kk_b)) {                                             \
            ++::kk::test::failures;                                        \
            std::printf("  %s:%d: CHECK_EQ(%s, %s): ", __FILE__, __LINE__, #a, #b); \
            std::cout << kk_a << " != " << kk_b << std::endl;              \
        }                                                                  \
    } while (0)
