#include <thread>

#include "Logger.h"
#include "TopicHandler.h"

int main(void) {
    TopicHandler topic("FRED/TCM/TCM0/PARAMETERS");

    while (true) {
        auto res = topic.handle_command("DELAY_A,WRITE,13", 1., true);
        if (res) {
            Logger::info("TEST", "{}", *res);
        }
        std::this_thread::sleep_for(std::chrono::duration<double>(1.0));
    }
}
