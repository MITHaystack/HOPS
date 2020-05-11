#ifndef HTokenizer_HH__
#define HTokenizer_HH__

#include <vector>
#include <string>

namespace hops
{
    
/*
*File: HTokenizer.hh
*Class: HTokenizer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/


class HTokenizer
{
    public:
        HTokenizer()
        {
            fDelim = " "; //default delim is space
            fString = nullptr;
            fIncludeEmptyTokens = false;
        };
        virtual ~HTokenizer(){;};

        void SetIncludeEmptyTokensTrue(){fIncludeEmptyTokens = true;};
        void SetIncludeEmptyTokensFalse(){fIncludeEmptyTokens = false;};

        void SetString(const std::string* aString){fString = aString;};
        void SetDelimiter(const std::string& aDelim){fDelim = aDelim;};
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
