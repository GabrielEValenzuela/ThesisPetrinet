#pragma once
#include <string>

namespace logger{
    struct record{
        uint32_t transition;
        std::string type;
        std::string action;
        uint64_t timestamp;
        std::string thread_id;
    };
};