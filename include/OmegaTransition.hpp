#pragma once
#include <memory>
#include <vector>
#include <sstream>
#include "PetriNetwork.hpp"
#include "../lib/MathEngine.hpp"

class OmegaTransition{
    private:
    const int32_t OMEGA = std::numeric_limits<int32_t>::max();
    std::unique_ptr<std::vector<int32_t>> row_incidence;
    std::unique_ptr<std::vector<int32_t>> row_preincidence;
    std::shared_ptr<PetriNetwork> petri_instance;
    std::shared_ptr<std::vector<uint8_t>> sensitized_vec;
    public:
    OmegaTransition(std::unique_ptr<std::vector<int32_t>> r_inc,std::unique_ptr<std::vector<int32_t>> r_pre,std::shared_ptr<PetriNetwork> petri_net)
    :row_incidence{std::move(r_inc)},
     row_preincidence{std::move(r_pre)},
     petri_instance{petri_net}
     {};
    uint8_t isSensitizedMark(std::vector<uint32_t>* mark){
        sensitized_vec = petri_instance->applyExtendedEquation(mark,row_preincidence.get());
        return std::any_of(sensitized_vec->begin(),sensitized_vec->end(),[](uint8_t e){return e;});
    }

    void copySensitized(std::vector<uint8_t>* new_sensitized,std::vector<uint32_t>* mark){
        sensitized_vec = petri_instance->applyExtendedEquation(mark,row_preincidence.get());
        std::copy(sensitized_vec->begin(),sensitized_vec->end(),new_sensitized->begin());
    }

    void fireFromMark(std::vector<uint32_t>* old_mark, std::vector<uint32_t>* new_mark){
        MathEngine::fireSafe(old_mark, row_incidence.get(), new_mark);
    }

    std::vector<int32_t>* getRowIncidence(){
        return row_incidence.get();
    }

    void print(){
        std::stringstream str_builder;
        str_builder << "[";
        for (auto m : *row_incidence) {
            if (m == OMEGA){
                str_builder << "W ";
            } else {
                str_builder << m << ". ";
            }
        }
        str_builder << "]";
        std::cout<<"Incidence: "<<str_builder.str()<<"\n\n";
    }

    std::vector<int32_t>* getRowPreincidence(){
        return row_preincidence.get();
    }

    bool operator>=(const OmegaTransition& rhs_omega){
        auto pre_comp = std::make_unique<std::vector<uint8_t>>(row_preincidence->size());
        auto inc_comp = std::make_unique<std::vector<uint8_t>>(row_incidence->size());
        std::transform(row_preincidence->begin(),row_preincidence->end(),rhs_omega.row_preincidence->begin(),pre_comp->begin(),[](uint32_t lhs,uint32_t rhs){return lhs<rhs;});
        std::transform(row_incidence->begin(),row_incidence->end(),rhs_omega.row_incidence->begin(),inc_comp->begin(),[](uint32_t lhs,uint32_t rhs){return lhs>rhs;});
        auto first = std::all_of(pre_comp->begin(),pre_comp->end(),[](uint8_t r){return r;});
        auto second = std::all_of(inc_comp->begin(),inc_comp->end(),[](uint8_t r){return r;});
        return first && second;
    }
};