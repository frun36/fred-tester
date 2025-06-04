#include <list>

#include "Test.h"

class TestSuite {
  private:
    std::list<Test> m_tests;

    void run() {
        for (auto& test : m_tests) {
            test.run();
        }
    }
};
