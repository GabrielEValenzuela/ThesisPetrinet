#pragma once
//#include "../lib/date/tz.h"
//#include "../lib/spdlog/spdlog.h"
//#include "../lib/spdlog/sinks/basic_file_sink.h"
#include <iostream>
#include <fstream>
#include <chrono>

class Logger{
    private:
    std::fstream log_file;
    std::string fire_record;
    //std::shared_ptr<spdlog::logger> log;
    public:
        Logger() {
            log_file.open("record_file.txt",std::ios::out);
        };
        ~Logger() {
            log_file.close();
        }
    void record(std::string data);
    void addFire(uint32_t transition);
    void error(std::string data);
    std::string getFireHistory();
};