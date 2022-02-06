#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include <execution>
#include <limits>

#ifdef __unix__

#endif
namespace MathEngine{
    /*
        Calculate the new mark
        @params
        old_mark: Previuos mark
        column: Fron incidence matrix
        new_mark: Vector that will contain the result mark
    */
    static void fire(std::vector<uint32_t>* old_mark, const std::vector<int>& column, std::vector<uint32_t>* new_mark) {
        std::transform(std::execution::par_unseq,old_mark->begin(), old_mark->end(), column.begin(), new_mark->begin(), std::plus<>());
    }

    static void fireSafe(const std::vector<uint32_t>* old_mark, const std::vector<int>* column, std::vector<uint32_t>* new_mark){
        std::transform(std::execution::par_unseq,old_mark->begin(), old_mark->end(), column->begin(), new_mark->begin(),[](uint32_t a,int b){
        if(b==std::numeric_limits<int32_t>::max()||a==std::numeric_limits<int32_t>::max()){
            return std::numeric_limits<int32_t>::max();
        } else {
            return (int)a+b;
        }});
    }
    /*static bool fire(std::vector<uint32_t> old_mark, std::vector<int> column, std::unique_ptr<std::vector<uint32_t>>& new_mark) {
    bool isValidFire = true;
        for(auto p = 0; p < old_mark.size();++p){
            new_mark->at(p) = old_mark.at(p) + column.at(p);
            isValidFire &= (old_mark.at(p) + column.at(p) >= 0);
        }
    return isValidFire;
    }*/
};