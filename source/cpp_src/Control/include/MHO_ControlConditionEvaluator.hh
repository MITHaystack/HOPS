#ifndef MHO_ControlConditionEvaluator_HH__
#define MHO_ControlConditionEvaluator_HH__

#include "MHO_Message.hh"
#include "MHO_ControlDefinitions.hh"

#include <string>
#include <vector>
#include <list>
#include <stack>

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

        int EvaluateStation(std::string station);
        int EvaluateBaseline(std::string baseline);
        int EvaluateSource(std::string src);
        int EvaluateFrequencyGroup(std::string fgroup);

        int EvaluateBooleanOps(std::list< int > states);

        std::string fWildcard;

        std::string fBaseline;
        std::string fRefStation;
        std::string fRemStation;
        std::string fSource;
        std::string fFGroup;
        std::string fScanTime;

        //std::vector< std::string > PreprocessTokens( std::vector< std::string>& tokens);

};



}

#endif /* end of include guard: MHO_ControlConditionEvaluator_HH__ */
