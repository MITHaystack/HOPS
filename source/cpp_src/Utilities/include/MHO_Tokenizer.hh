#ifndef MHO_Tokenizer_HH__
#define MHO_Tokenizer_HH__

#include <string>
#include <vector>

#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file MHO_Tokenizer.hh
 *@class MHO_Tokenizer
 *@date Mon May 11 15:51:26 2020 -0400
 *@author J. Barret - barrettj@mit.edu
 * A configurable class reponsible for parsing a string on a given delimiter (default delimiter) is a space
 */

/**
 * @brief Class MHO_Tokenizer
 */
class MHO_Tokenizer
{
    public:
        MHO_Tokenizer();
        ~MHO_Tokenizer();

        /*!* Include empty tokens set fIncludeEmptyTokens to true for situations where two delimiters surround a substring which would result in an empty string
         * @param None
         * @returns None
         */
        void SetIncludeEmptyTokensTrue();

        /*!* Do not include empty tokens and set fincludeEmptyTokens to false
         * @param None
         * @returns None
         */
        void SetIncludeEmptyTokensFalse();

        void SetUseMulticharacterDelimiterTrue();
        void SetUseMulticharacterDelimiterFalse();

        void SetPreserveQuotesTrue();
        void SetPreserveQuotesFalse();

        /**
         * @brief Setter for remove leading trailing whitespace true
         */
        void SetRemoveLeadingTrailingWhitespaceTrue();
        /**
         * @brief Setter for remove leading trailing whitespace false
         */
        void SetRemoveLeadingTrailingWhitespaceFalse();

        /*!* Set the string to be parsed
         * @param aString string
         * @returns None
         */
        void SetString(const std::string* aString);

        /*!* Set the delimiter to be used to parse the string in SetString
         * @param aDelim string a string to be used as a delimiter
         * @returns None
         */
        void SetDelimiter(const std::string& aDelim);

        /*!* Parse the string using the tokens
         * @param tokens pointer to a vector of strings
         * @returns None
         */
        void GetTokens(std::vector< std::string >* tokens);

        static std::string TrimLeadingAndTrailingWhitespace(const std::string& value);

    protected:
        void MultiCharTokenize(std::vector< std::string >* tokens);
        void SingleCharTokenize(std::vector< std::string >* tokens);

        /**
         * @brief Finds and indexes quote instances in a given string.
         *
         * @param aString Input string to search for quotes.
         * @param quotes (std::vector< std::pair< std::size_t, std::size_t ) *
         * @return Number of indexed quote pairs found.
         */
        std::size_t IndexQuoteInstances(const std::string* aString,
                                        std::vector< std::pair< std::size_t, std::size_t > >* quotes);

        bool fIncludeEmptyTokens;
        bool fMultiCharDelimiter;
        bool fPreserveQuotes;
        bool fRemoveLeadingTrailingWhitespace;
        std::string fDelim;
        const std::string* fString;
        const std::string* fCurrentString;
        std::vector< std::pair< std::size_t, std::size_t > > fQuotePairIndexes;
};

//fuction which splits a single string into a vector of tokens
//if the default (no delimiter) is used, then each character is split into a new token
//otherwise the string is split on the specified delimiter
/**
 * @brief Function SplitString
 *
 * @param input (const std::string&)
 * @param delim (std::string)
 * @return Return value (std::string >)
 */
std::vector< std::string > SplitString(const std::string& input, std::string delim = "");

} // namespace hops

#endif /*! end of include guard: MHO_Tokenizer */
