#include <thread>
#include "tests/Parameters.h"
#include "tests/Status.h"

int main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
        Logger::setDebug(true);
    }
    tests::Status status;
    status.start();

    std::this_thread::sleep_for(std::chrono::duration<double>(3.));

    tests::TcmParameters parameters;
    parameters.run();

    std::this_thread::sleep_for(std::chrono::duration<double>(3.));

    status.stop();
}
