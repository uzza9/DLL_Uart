#include "cmn_lib.h"


std::chrono::high_resolution_clock::time_point  timeStart;
std::chrono::high_resolution_clock::time_point  timeEnd;

void current_time_point(std::chrono::system_clock::time_point timePt)
{
    time_t  timeStamp = std::chrono::system_clock::to_time_t(timePt);
    std::cout << ctime(&timeStamp) << std::endl;
}

void timestamp_start(const std::string &msg)
{
    std::cout << msg;
    current_time_point(std::chrono::system_clock::now());
    timeStart = std::chrono::high_resolution_clock::now();       
}

void timestamp_end(const std::string &msg)
{
    std::cout << msg;
    current_time_point(std::chrono::system_clock::now());
    timeEnd = std::chrono::high_resolution_clock::now();    
}

void timestamp_elapsed(const std::string &msg)
{
    std::chrono::duration<double, std::milli> elapsed = timeEnd - timeStart;
    std::cout << msg << elapsed.count() << " m.seconds" << std::endl;       
}
