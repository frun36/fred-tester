#include "Logger.h"
#include "TopicHandler.h"

int main(void) {
    TopicHandler topic("FRED/TCM/TCM0/PARAMETERS");
    
    while (true) {
        auto res = topic.handle_command("COUNTER_READ_INTERVAL,READ", 1.);
        if (res) {
            Logger::info("TEST", "{}", *res);
        }
    }
}
