#include "MHO_Tokenizer.hh"

#include <vector>
#include <string>

namespace hops
{

MHO_Tokenizer::MHO_Tokenizer()
{
    fDelim = " "; //default delim is space
    fString = nullptr;
    fIncludeEmptyTokens = false;
    fMultiCharDelimiter = false;
};
MHO_Tokenizer::~MHO_Tokenizer(){;};

void MHO_Tokenizer::SetIncludeEmptyTokensTrue(){fIncludeEmptyTokens = true;};

void MHO_Tokenizer::SetIncludeEmptyTokensFalse(){fIncludeEmptyTokens = false;};

void MHO_Tokenizer::SetUseMulticharacterDelimiterTrue(){fMultiCharDelimiter = true;};

void MHO_Tokenizer::SetUseMulticharacterDelimiterFalse(){fMultiCharDelimiter = false;};

void MHO_Tokenizer::SetString(const std::string* aString){fString = aString;};

void MHO_Tokenizer::SetDelimiter(const std::string& aDelim){fDelim = aDelim;};

void MHO_Tokenizer::GetTokens(std::vector< std::string>* tokens)
{
    if(tokens != NULL && fString != NULL)
    {
        tokens->clear();
        if(!fMultiCharDelimiter) //delimiters are single-characters only
        {
            if(fDelim.size() > 0)
            {
                size_t start = 0;
                size_t end = 0;
                size_t length = 0;
                while( end != std::string::npos )
                {
                    end = fString->find_first_of(fDelim, start);
                    if(end == std::string::npos){ length = std::string::npos;}
                    else{length = end - start;}

                    if( fIncludeEmptyTokens || ( (length > 0 ) && ( start < fString->size() ) ) )
                    {
                        tokens->push_back( fString->substr(start,length) );
                    }

                    if( end > std::string::npos - 1 ){ start = std::string::npos;}
                    else{start = end + 1;}
                }
            }
        }
        else //the delimiter is a multi-character string 
        {
            size_t start = 0;
            size_t end = 0;
            size_t length = 0;
            while( end != std::string::npos )
            {
                end = fString->find(fDelim, start);

                if(end == std::string::npos){ length = std::string::npos;}
                else{length = end - start;}

                if( fIncludeEmptyTokens || ( (length > 0 ) && ( start < fString->size() ) ) )
                {
                    tokens->push_back( fString->substr(start,length) );
                }

                if( end > std::string::npos - fDelim.size() ){ start = std::string::npos;}
                else{start = end + fDelim.size();}
            }
        }
    }

}

}//end namespace 

