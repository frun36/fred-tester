#include <cstring>

#include "tests/FredTester.h"
#include "Logger.h"

int main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
        Logger::setDebug(true);
    }

    tests::FredTester tester;
    tester.run();
}
