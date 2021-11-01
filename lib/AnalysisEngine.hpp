#pragma once
#include <regex>
#include <iostream>

namespace AnalysisEngine{
    static void findInvariantsTransitions(std::string fire_history) {
        if(fire_history==""){
            std::cout<<"Invalid fire history. You should run a simulation first\n";
        } else {
            std::regex pattern("(T\\d)(.*)(\\1)");
            std::smatch strmatches;
            std::regex_match(fire_history,strmatches,pattern);
            std::cout<<"Invalid transitions\n===================\n";
            for(auto match: strmatches){
                std::cout<<match<<"\n";
            }
        }
    }
};