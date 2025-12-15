#pragma once

#include <string>
#include <iostream>
#include <functional>
#include <vector>

namespace equilibria {
namespace test {

/**
 * @brief Lightweight test framework for controller unit tests
 * 
 * Simple assertion-based testing with minimal overhead.
 * Designed to be fast and runnable in CI.
 */
class TestRunner {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string message;
    };

    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void add_test(const std::string& name, std::function<void()> test_func) {
        tests_.push_back({name, test_func});
    }

    int run_all() {
        int passed = 0;
        int failed = 0;

        std::cout << "\n=== Running Tests ===\n" << std::endl;

        for (const auto& test : tests_) {
            try {
                std::cout << "[ RUN  ] " << test.name << std::endl;
                test.func();
                std::cout << "[ PASS ] " << test.name << std::endl;
                passed++;
            } catch (const std::exception& e) {
                std::cerr << "[ FAIL ] " << test.name << std::endl;
                std::cerr << "         " << e.what() << std::endl;
                failed++;
            }
        }

        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;

        return failed;
    }

private:
    struct Test {
        std::string name;
        std::function<void()> func;
    };

    std::vector<Test> tests_;
};

// Assertion macros
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + #condition); \
        } \
    } while (0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            throw std::runtime_error(std::string("Assertion failed: !") + #condition); \
        } \
    } while (0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + #expected + " == " + #actual); \
        } \
    } while (0)

#define ASSERT_CONTAINS(haystack, needle) \
    do { \
        if ((haystack).find(needle) == std::string::npos) { \
            throw std::runtime_error(std::string("Assertion failed: '") + haystack + "' contains '" + needle + "'"); \
        } \
    } while (0)

// Test registration macro
#define TEST(test_name) \
    void test_##test_name(); \
    struct test_##test_name##_registrar { \
        test_##test_name##_registrar() { \
            equilibria::test::TestRunner::instance().add_test(#test_name, test_##test_name); \
        } \
    }; \
    static test_##test_name##_registrar test_##test_name##_registrar_instance; \
    void test_##test_name()

} // namespace test
} // namespace equilibria
