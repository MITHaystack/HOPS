#ifndef MHO_ControlConditionEvaluator_HH__
#define MHO_ControlConditionEvaluator_HH__

#include "MHO_ControlDefinitions.hh"
#include "MHO_Message.hh"

#include <list>
#include <stack>
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file MHO_ControlConditionEvaluator.hh
 *@class MHO_ControlConditionEvaluator
 *@date Fri May 19 13:08:22 2023 -0400
 *@brief evaluates conditional statements encounterd in control file syntax
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_ControlConditionEvaluator
{

    public:
        MHO_ControlConditionEvaluator();
        virtual ~MHO_ControlConditionEvaluator();

        /**
         * @brief Setter for pass (baseline, source, frequency group, scan) information
         *
         * @param baseline Baseline string
         * @param source Source string
         * @param fgroup Frequency Group string
         * @param scan_time Scan-time/name string
         */
        void SetPassInformation(std::string baseline, std::string source, std::string fgroup, std::string scan_time);

        /**
         * @brief reduces the contents of a control file to only those statements which are applicable for this pass
         *
         * @param control_contents Reference to input mho_json containing control conditions
         * @return mho_json containing applicable conditional statements
         */
        mho_json GetApplicableStatements(mho_json& control_contents);

        /**
         * @brief Evaluates a single condition from an mho_json object and returns the result as a boolean.
         *
         * @param control_condition Reference to an mho_json object containing the condition to evaluate.
         * @return Boolean indicating whether the evaluated condition is true or false.
         */
        bool Evaluate(mho_json& control_condition);

    private:
        using token_iter = std::vector< std::string >::iterator;

        /**
         * @brief Processes a token and returns its corresponding state or operation.
         *
         * @param it Input/output iterator to the current token.
         * @param it_end Iterator marking the end of the tokens sequence.
         * @return Integer representing the token's state or operation.
         */
        int ProcessToken(token_iter& it, token_iter& it_end);

        /**
         * @brief Evaluates station condition and returns TRUE_STATE if it matches reference or remote stations or wildcard.
         *
         * @param it Input iterator to token sequence.
         * @return TRUE_STATE if station matches reference/remove/wildcard, FALSE_STATE otherwise.
         */
        int EvaluateStation(token_iter& it);
        /**
         * @brief Evaluates baseline condition and returns TRUE_STATE if met.
         *
         * @param it Input iterator to evaluate baseline condition.
         * @return TRUE_STATE if baseline condition is met, FALSE_STATE otherwise.
         */
        int EvaluateBaseline(token_iter& it);
        /**
         * @brief Evaluates source token and returns TRUE_STATE if it's a wildcard or source, FALSE_STATE otherwise.
         *
         * @param it Input iterator to current token
         * @return TRUE_STATE if token is wildcard or source, FALSE_STATE otherwise
         */
        int EvaluateSource(token_iter& it);
        /**
         * @brief Evaluates frequency group and returns TRUE_STATE if it matches fFGroup or fWildcard, otherwise FALSE_STATE.
         *
         * @param it Input iterator to evaluate frequency group.
         * @return TRUE_STATE if frequency group matches fFGroup or fWildcard, otherwise FALSE_STATE.
         */
        int EvaluateFrequencyGroup(token_iter& it);
        /**
         * @brief Evaluates scan statements and returns state based on token iterators.
         *
         * @param it Input iterator to current token
         * @param it_end Iterator to end of tokens
         * @return State (TRUE_STATE or FALSE_STATE) based on evaluation
         */
        int EvaluateScan(token_iter& it, token_iter& it_end);

        /**
         * @brief Checks if fScanTime is less than input scan string and returns TRUE_STATE if true.
         *
         * @param scan Input scan string to compare with fScanTime
         * @return TRUE_STATE if fScanTime < scan, FALSE_STATE otherwise
         */
        int ScanLessThan(std::string scan);
        /**
         * @brief Checks if fScanTime is greater than scan and returns TRUE_STATE if true.
         *
         * @param scan Input string to compare with fScanTime
         * @return TRUE_STATE if fScanTime  scan, FALSE_STATE otherwise
         */
        int ScanGreaterThan(std::string scan);
        /**
         * @brief Checks if fScanTime is between scan_low and scan_high.
         *
         * @param scan_low Lower bound for time scan
         * @param scan_high Upper bound for time scan
         * @return TRUE_STATE (1) if fScanTime is in range, FALSE_STATE (0) otherwise
         */
        int ScanInBetween(std::string scan_low, std::string scan_high);

        /**
         * @brief Evaluates boolean operations (NOT, AND, OR) in a list of states.
         *
         * @param states List of integer states representing boolean operations and values
         * @return Resulting state after evaluating all boolean operations
         */
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

} // namespace hops

#endif /*! end of include guard: MHO_ControlConditionEvaluator_HH__ */
