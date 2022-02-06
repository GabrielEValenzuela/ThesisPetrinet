#pragma once
#include <string>

namespace logger{
    struct record{
        uint32_t transition;
        std::string type;
        std::string action;
        std::string timestamp;
        std::string thread_id;
    };
};