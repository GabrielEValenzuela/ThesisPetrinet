#pragma once
#include <variant>
#include <vector>
#include <algorithm>
#include <memory>
#include "../lib/json.hpp"
#include "../lib/pugixml.hpp"
#include "PetriBuilder.hpp"

/*
    The Concrete Builders provide different implementations of the construction steps.
    Concrete builders may produce products that don’t follow the common interface.
    This return a resulting object, a product.
    Products constructed by different builders don’t have to belong to the same class hierarchy or interface.
*/
class PlaceTransitionNet : public PetriBuilder{
    public:
        void buildStepPTMatrix() override;
        void auxTransitionsDisc();
        void buildStepHMatrix() override;
        void buildStepWMatrix() override;
        void buildStepAVector() override;
        void buildStepPTMark() override;
        void buildStepSensitized() override;
        void auxStepLVector(std::vector<uint8_t>* l_vec);
        void auxStepBVector(std::vector<uint8_t>* b_vec);
        void auxStepSensitized(PetriMatrix<int32_t>* pre_incidence, std::vector<uint32_t>& mark);
        void buildStepGuardAndEvent() override;
        void setFile(std::unique_ptr<ReadFile> load_file) override;
        void setDependencyInjection() override;
        std::shared_ptr<PetriNetwork> getPetrinet() override;
};