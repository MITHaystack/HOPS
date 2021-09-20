#ifndef MHO_Tokenizer_HH__
#define MHO_Tokenizer_HH__

#include <vector>
#include <string>

namespace hops
{

/**
*@file MHO_Tokenizer.hh 
*@class MHO_Tokenizer
*@author J. Barret - barrettj@mit.edu
* A class reponsible for parsing a string on a given delimiter where the default is a space 
*/

class MHO_Tokenizer
{
    public:
        MHO_Tokenizer()
        {
            fDelim = " "; //default delim is space
            fString = nullptr;
            fIncludeEmptyTokens = false;
        };
        virtual ~MHO_Tokenizer(){;};

        /** Include empty tokens set fIncludeEmptyTokens to true for situations where two delimiters surround a substring which would result in an empty string
        * @param None
        * @returns None
        */
        void SetIncludeEmptyTokensTrue(){fIncludeEmptyTokens = true;};

        /** Do not include empty tokens and set fincludeEmptyTokens to false 
        * @param None
        * @returns None
        */
        void SetIncludeEmptyTokensFalse(){fIncludeEmptyTokens = false;};

      
        /** Set the string to be parsed 
        * @param aString string
        * @returns None 
        */
        void SetString(const std::string* aString){fString = aString;};
        
        
        /** Set the delimeter to be used to parse the string in SetString 
        * @param aDelim string a string to be used as a delimiter
        * @returns None
        */
        void SetDelimiter(const std::string& aDelim){fDelim = aDelim;};

        /** Parse the string using the tokens 
        * @param tokens pointer to a vector of strings 
        * @returns None
        */
        void GetTokens(std::vector< std::string>* tokens) const
        {
            if(tokens != NULL && fString != NULL)
            {
                tokens->clear();
                if(fDelim.size() > 0)
                {

                    size_t start = 0;
                    size_t end = 0;
                    size_t length = 0;
                    while( end != std::string::npos )
                    {
                        end = fString->find(fDelim, start);

                        if(end == std::string::npos)
                        {
                            length = std::string::npos;
                        }
                        else
                        {
                            length = end - start;
                        }


                        if( fIncludeEmptyTokens || ( (length > 0 ) && ( start < fString->size() ) ) )
                        {
                            tokens->push_back( fString->substr(start,length) );
                        }

                        if( end > std::string::npos - fDelim.size() )
                        {
                            start = std::string::npos;
                        }
                        else
                        {
                            start = end + fDelim.size();
                        }
                    }
                }
            }
        }


    protected:

        bool fIncludeEmptyTokens;
        std::string fDelim;
        const std::string* fString;

};

}

#endif /* __HTokenizer_H__ */
