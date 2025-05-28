#include "FredTester.h"

#include <format>
#include <iostream>

void FredTester::run() {
    for (auto s : m_tests) {
        std::cout << s << '\n';
    }
}
