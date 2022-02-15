#pragma once
#include <cstdint>
#include <string>
#include <utility>
/*
#include <regex>
constexpr int group = 2;
namespace tools{
    static std::pair<uint32_t,uint32_t> getRows(std::string source,std::string destiny){
        std::regex pattern("(\w)(\d+)");
        std::smatch sm_source;
        std::smatch sm_destiny;
        std::regex_search(source,sm_source,pattern);
        std::regex_search(destiny,sm_destiny,pattern);
        uint32_t isource = std::stoi(sm_source[group]);
        uint32_t idestiny = std::stoi(sm_destiny[group]);
        return std::make_pair(isource,idestiny);
    }
}*/

namespace Petri_Choice{
    enum class type{
        DISCRETE,
        CONTINUOS,
        HYBRID,
        TEMPORAL
    };
};

namespace Distribution_event {
    enum class type {
        NORMAL,
        TSTUDENT,
        CHISQUARE,
        POISSON
    };
}

namespace Agent_Choice {
    enum class type {
        Immediate,
        Temporal
    };
}

namespace Network_Choice {
    enum class type {
        DISCRETE_IMMEDIATE,
        DISCRETE_TEMPORAL,
        CONTINUOS_IMMEDIATE,
        CONTINUOS_TEMPORAL,
        COLORED_IMMEDIATE,
        COLORED_TEMPORAL,
        ERR = -1
    };
};

namespace Timescale_Choice {
    enum class unit {
        NANOSECOND,
        MICROSECOND,
        MILLISECOND,
        SECOND,
        MINUTE,
        HOUR,
        ERR = -1
    };
};

namespace util_reader{
    static const std::string TRANSITION_TEMPORAL        = "temporal";
    static const std::string TRANSITION_IMMEDIATE       = "immediate";
    static const std::string TEMPORAL_DETERMINISTIC     = "deterministic";
    static const std::string TEMPORAL_STOCHASTIC        = "stochastic";

    static const std::string PLACE_DISCRETE             = "discrete";
    static const std::string PLACE_CONTINUOS            = "continuos";
    static const std::string PLACE_COLORED              = "colored";

    static const std::string FUNCTION_EQ                = "==";
    static const std::string FUNCTION_NEQ               = "!=";
    static const std::string FUNCTION_LEQ               = "<=";
    static const std::string FUNCTION_GEQ               = ">=";
    static const std::string FUNCTION_GET               = ">";
    static const std::string FUNCTION_LET               = "<";

    static const std::string ARC_REGULAR                = "regular";
    static const std::string ARC_INHIBITOR              = "inhibitor";
    static const std::string ARC_READER                 = "reader";
    static const std::string ARC_RESET                  = "reset";

    struct distribution{
    std::string name;
    float first_parameter;
    float second_parameter;
    };
};

