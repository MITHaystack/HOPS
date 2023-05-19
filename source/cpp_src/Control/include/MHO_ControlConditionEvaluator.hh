#ifndef MHO_ControlConditionEvaluator_HH__
#define MHO_ControlConditionEvaluator_HH__

#include "MHO_ControlDefinitions.hh"

namespace hops
{

class MHO_ControlConditionEvaluator
{

    public:
        MHO_ControlConditionEvaluator();
        virtual ~MHO_ControlConditionEvaluator();

        void SetPassInformation(std::string baseline, std::string source, std::string fgroup, std::string scan_time);

        bool Evaluate(mho_json& control_condition);

    private:

        std::string fBaseline;
        std::string fSource;
        std::string fFGroup;
        std::string fScanTime;

};



}

#endif /* end of include guard: MHO_ControlConditionEvaluator_HH__ */
