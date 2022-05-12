#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

int main(int argc, char **argv)
{
  try {
     LoggerPtr logger = Logger::Create(/*channel_name: */ TM("[test_haawks]"));
     logger->LOG_INFO("Hello FixServer");
    }
    catch(std::exception& ex)
    {
        logger->LOG_INFO("exception : %s", ex.what());
    }
    return 0;
}
