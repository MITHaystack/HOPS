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

        using token_iter = std::vector< std::string >::iterator;

        int ProcessToken(token_iter& it, token_iter& it_end);

        int EvaluateStation(token_iter& it);
        int EvaluateBaseline(token_iter& it);
        int EvaluateSource(token_iter& it);
        int EvaluateFrequencyGroup(token_iter& it);
        int EvaluateScan(token_iter& it, token_iter& it_end);

        int ScanLessThan(std::string scan);
        int ScanGreaterThan(std::string scan);
        int ScanInBetween(std::string scan_low, std::string scan_high);

        int EvaluateBooleanOps(std::list< int > states);

        std::string fWildcard;

        std::string fBaseline;
        std::string fRefStation;
        std::string fRemStation;
        std::string fSource;
        std::string fFGroup;
        std::string fScanTime;


        std::size_t fStartLineNumber;
};



}

#endif /* end of include guard: MHO_ControlConditionEvaluator_HH__ */
