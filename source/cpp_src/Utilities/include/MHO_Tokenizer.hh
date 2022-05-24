#ifndef MHO_Tokenizer_HH__
#define MHO_Tokenizer_HH__

#include <vector>
#include <string>

namespace hops {

/**
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

        /** Set the string to be parsed 
        * @param aString string
        * @returns None 
        */
        void SetString(const std::string* aString);

        /** Set the delimeter to be used to parse the string in SetString 
        * @param aDelim string a string to be used as a delimiter
        * @returns None
        */
        void SetDelimiter(const std::string& aDelim);
        
        /** Parse the string using the tokens 
        * @param tokens pointer to a vector of strings 
        * @returns None
        */
        void GetTokens(std::vector< std::string>* tokens);

    protected:

        bool fIncludeEmptyTokens;
        bool fMultiCharDelimiter;
        std::string fDelim;
        const std::string* fString;
};

} // end of hops namespace

#endif /* end of include guard: MHO_Tokenizer */
