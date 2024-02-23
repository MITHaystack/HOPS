#ifndef MHO_Tokenizer_HH__
#define MHO_Tokenizer_HH__

#include <vector>
#include <string>

#include "MHO_Message.hh"

namespace hops {

/*!
*@file MHO_Tokenizer.hh
*@class MHO_Tokenizer
*@author J. Barret - barrettj@mit.edu
* A class reponsible for parsing a string on a given delimiter where the default is a space
*/

class MHO_Tokenizer{
    public:
        MHO_Tokenizer();
        ~MHO_Tokenizer();

        /** Include empty tokens set fIncludeEmptyTokens to true for situations where two delimiters surround a substring which would result in an empty string
        * @param None
        * @returns None
        */
        void SetIncludeEmptyTokensTrue();

        /** Do not include empty tokens and set fincludeEmptyTokens to false
        * @param None
        * @returns None
        */
        void SetIncludeEmptyTokensFalse();

        void SetUseMulticharacterDelimiterTrue();
        void SetUseMulticharacterDelimiterFalse();

        void SetPreserveQuotesTrue();
        void SetPreserveQuotesFalse();

        void SetRemoveLeadingTrailingWhitespaceTrue();
        void SetRemoveLeadingTrailingWhitespaceFalse();

        /** Set the string to be parsed
        * @param aString string
        * @returns None
        */
        void SetString(const std::string* aString);

        /** Set the delimiter to be used to parse the string in SetString
        * @param aDelim string a string to be used as a delimiter
        * @returns None
        */
        void SetDelimiter(const std::string& aDelim);

        /** Parse the string using the tokens
        * @param tokens pointer to a vector of strings
        * @returns None
        */
        void GetTokens(std::vector< std::string>* tokens);

        static std::string
        TrimLeadingAndTrailingWhitespace(const std::string& value);

    protected:

        void MultiCharTokenize(std::vector< std::string>* tokens);
        void SingleCharTokenize(std::vector< std::string>* tokens);

        std::size_t IndexQuoteInstances(const std::string* aString, std::vector< std::pair< std::size_t, std::size_t>  >* quotes);


        bool fIncludeEmptyTokens;
        bool fMultiCharDelimiter;
        bool fPreserveQuotes;
        bool fRemoveLeadingTrailingWhitespace;
        std::string fDelim;
        const std::string* fString;
        const std::string* fCurrentString;
        std::vector< std::pair< std::size_t, std::size_t>  > fQuotePairIndexes;
};



//fuction which splits a single string into a vector of tokens
//if the default (no delimiter) is used, then each character is split into a new token
//otherwise the string is split on the specified delimiter
std::vector< std::string > SplitString(const std::string& input, std::string delim = "");




} // end of hops namespace

#endif /* end of include guard: MHO_Tokenizer */
