#include "../include/Logger.hpp"

void Logger::record(std::string data){
    //using date::operator<<;
    //log_file << date::current_zone()->
      //  to_local(std::chrono::system_clock::now()) << "\t" << data << "\n";
    //std::cout << "\t" << data << "\n";
}

void Logger::error(std::string data) {
    log_file << "***ERROR*** \t" << data << "\n";
}

void Logger::addFire(uint32_t transition){
    fire_record+="T"+std::to_string(transition);
}

std::string Logger::getFireHistory() {
    return fire_record;
}