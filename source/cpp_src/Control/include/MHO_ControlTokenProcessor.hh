#ifndef MHO_ControlTokenProcessor_HH__
#define MHO_ControlTokenProcessor_HH__

#include "MHO_ControlDefinitions.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"

#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file  MHO_ControlTokenProcessor.hh
 *@class  MHO_ControlTokenProcessor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Jun 13 22:27:21 2022 -0400
 *@brief
 */

/**
 * @brief Class MHO_ControlTokenProcessor
 */
class MHO_ControlTokenProcessor
{
    public:
        MHO_ControlTokenProcessor();
        virtual ~MHO_ControlTokenProcessor();

        /**
         * @brief Processes an integer token and converts it to a json value.
         * 
         * @param token Input token of type MHO_Token&.
         * @return Json representation of the integer value from the token.
         */
        mho_json ProcessInt(const MHO_Token& token);
        /**
         * @brief Processes a given MHO_Token and returns its associated JSON data.
         * 
         * @param token Input token containing value to be processed.
         * @return mho_json representing the processed token's value.
         */
        mho_json ProcessString(const MHO_Token& token);
        /**
         * @brief Processes a real token and converts it to an mho_json value.
         * 
         * @param token Input MHO_Token containing a real number.
         * @return mho_json representation of the input token's value if conversion succeeds, otherwise empty.
         */
        mho_json ProcessReal(const MHO_Token& token);
        /**
         * @brief Processes a list of tokens as integers and returns them as an mho_json object.
         * 
         * @param tokens Input vector of MHO_Token objects to process
         * @return mho_json object containing the processed integer values
         */
        mho_json ProcessListInt(const std::vector< MHO_Token >& tokens);
        /**
         * @brief Processes a list of tokens and converts them into an mho_json object.
         * 
         * @param tokens Input vector of MHO_Token objects
         * @return mho_json object containing values from input tokens
         */
        mho_json ProcessListString(const std::vector< MHO_Token >& tokens);
        /**
         * @brief Process a fixed-length list string from tokens and return as mho_json.
         * 
         * @param tokens Input vector of MHO_Token objects.
         * @return mho_json object containing values from input tokens.
         */
        mho_json ProcessFixedLengthListString(const std::vector< MHO_Token >& tokens);
        /**
         * @brief Processes a list of tokens as real numbers and returns them as JSON.
         * 
         * @param tokens Input vector of MHO_Token objects.
         * @return JSON object containing processed real number values.
         */
        mho_json ProcessListReal(const std::vector< MHO_Token >& tokens);
        /**
         * @brief Processes a boolean token and returns its value as mho_json.
         * 
         * @param token Input boolean token to process.
         * @return mho_json representation of the boolean token's value.
         */
        mho_json ProcessBool(const MHO_Token& token);

    private:
        /**
         * @brief Converts a token's float value to double and checks for conversion success.
         * 
         * @param token Input token containing float value as string.
         * @param val (double&)
         * @return True if conversion is successful, false otherwise.
         */
        bool ConvertFloat(const MHO_Token& token, double& val);
        /**
         * @brief Converts a MHO_Token to an integer and validates it.
         * 
         * @param token Input token to convert.
         * @param val (int&)
         * @return True if conversion is successful, false otherwise.
         */
        bool ConvertInteger(const MHO_Token& token, int& val);
        /**
         * @brief Converts an MHO_Token to a boolean value and stores it in val.
         * 
         * @param token Input token to convert to boolean
         * @param val (bool&)
         * @return True if conversion was successful, false otherwise
         */
        bool ConvertBool(const MHO_Token& token, bool& val);
};

} // namespace hops

#endif /*! end of include guard: MHO_ControlTokenProcessor */
