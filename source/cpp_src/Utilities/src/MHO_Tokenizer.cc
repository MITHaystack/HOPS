#include "MHO_Tokenizer.hh"

#include <vector>
#include <string>

namespace hops
{

MHO_Tokenizer::MHO_Tokenizer()
{
    fDelim = " "; //default delim is space
    fString = nullptr;
    fCurrentString = nullptr;
    fIncludeEmptyTokens = false;
    fMultiCharDelimiter = false;
    fPreserveQuotes = false;
    fRemoveLeadingTrailingWhitespace = false;
};
MHO_Tokenizer::~MHO_Tokenizer(){;};

void MHO_Tokenizer::SetIncludeEmptyTokensTrue(){fIncludeEmptyTokens = true;};
void MHO_Tokenizer::SetIncludeEmptyTokensFalse(){fIncludeEmptyTokens = false;};

void MHO_Tokenizer::SetUseMulticharacterDelimiterTrue(){fMultiCharDelimiter = true;};
void MHO_Tokenizer::SetUseMulticharacterDelimiterFalse(){fMultiCharDelimiter = false;};

void MHO_Tokenizer::SetPreserveQuotesTrue(){fPreserveQuotes = true;};
void MHO_Tokenizer::SetPreserveQuotesFalse(){fPreserveQuotes = false;};

void MHO_Tokenizer::SetRemoveLeadingTrailingWhitespaceTrue(){fRemoveLeadingTrailingWhitespace = true;};
void MHO_Tokenizer::SetRemoveLeadingTrailingWhitespaceFalse(){fRemoveLeadingTrailingWhitespace = false;};

void MHO_Tokenizer::SetString(const std::string* aString){fString = aString;};

void MHO_Tokenizer::SetDelimiter(const std::string& aDelim){fDelim = aDelim;};

void 
MHO_Tokenizer::GetTokens(std::vector< std::string>* tokens)
{
    if(tokens != NULL && fString != NULL)
    {
        tokens->clear();
        fQuotePairIndexes.clear();
        std::size_t n_quote = IndexQuoteInstances(fString, &fQuotePairIndexes);
        if(!fPreserveQuotes || n_quote == 0)
        {
            fCurrentString = fString;
            if(!fMultiCharDelimiter){SingleCharTokenize(tokens);}
            else{MultiCharTokenize(tokens);}
        }
        else 
        {
            //locate quotes, and only tokenize the portions which are outside of a closed pair 
            std::vector< std::pair< bool, std::string > > sections;
            std::size_t prev = 0;
            for(std::size_t i=0; i<n_quote; i++)
            {
                std::string sec = fString->substr(prev, fQuotePairIndexes[i].first - prev);
                std::string quote = fString->substr(fQuotePairIndexes[i].first, fQuotePairIndexes[i].second - fQuotePairIndexes[i].first );
                prev = fQuotePairIndexes[i].second;
                sections.push_back( std::make_pair(false, sec) );
                sections.push_back( std::make_pair(true, quote) );
            }
            sections.push_back( std::make_pair(false, fString->substr(prev) ) ); //catch any trailing portion

            for(std::size_t i=0; i<sections.size(); i++)
            {
                if(sections[i].first == true)
                {
                    tokens->push_back(sections[i].second); //treat quoted section as a single token
                }
                else 
                {
                    fCurrentString = &(sections[i].second);
                    if(!fMultiCharDelimiter){SingleCharTokenize(tokens);}
                    else{MultiCharTokenize(tokens);}
                }
            }
        }
    }

    if(fRemoveLeadingTrailingWhitespace)
    {
        for(std::size_t  i=0; i<tokens->size(); i++)
        {
            (*tokens)[i] = TrimLeadingAndTrailingWhitespace( (*tokens)[i]);
        }
    }
}


void 
MHO_Tokenizer::MultiCharTokenize(std::vector< std::string>* tokens)
{
    //the delimiter is a multi-character string 
    size_t start = 0;
    size_t end = 0;
    size_t length = 0;
    while( end != std::string::npos )
    {
        end = fCurrentString->find(fDelim, start);

        if(end == std::string::npos){ length = std::string::npos;}
        else{length = end - start;}

        if( fIncludeEmptyTokens || ( (length > 0 ) && ( start < fCurrentString->size() ) ) )
        {
            tokens->push_back( fCurrentString->substr(start,length) );
        }

        if( end > std::string::npos - fDelim.size() ){ start = std::string::npos;}
        else{start = end + fDelim.size();}
    }
}

void 
MHO_Tokenizer::SingleCharTokenize(std::vector< std::string>* tokens)
{
    //delimiters are single-characters only
    if(fDelim.size() > 0)
    {
        size_t start = 0;
        size_t end = 0;
        size_t length = 0;
        while( end != std::string::npos )
        {
            end = fCurrentString->find_first_of(fDelim, start);
            if(end == std::string::npos){ length = std::string::npos;}
            else{length = end - start;}

            if( fIncludeEmptyTokens || ( (length > 0 ) && ( start < fCurrentString->size() ) ) )
            {
                tokens->push_back( fCurrentString->substr(start,length) );
            }

            if( end > std::string::npos - 1 ){ start = std::string::npos;}
            else{start = end + 1;}
        }
    }
}

std::size_t 
MHO_Tokenizer::IndexQuoteInstances(const std::string* aString, std::vector< std::pair< std::size_t, std::size_t> >* quotes)
{   
    quotes->clear();
    std::vector< std::size_t > positions;
    for(std::size_t i=0; i<aString->size(); i++)
    {
        if( (*aString)[i] == '\"'){positions.push_back(i);}
    }
    
    if(positions.size() % 2 == 0 )
    {
        std::size_t n_quotes = positions.size()/2;
        for(std::size_t j=0; j<n_quotes; j++)
        {
            std::pair< std::size_t, std::size_t > q;
            q.first = positions[2*j];
            q.second = positions[2*j + 1];
            quotes->push_back(q);
        }
    }
    else 
    {
        //error we have an unmatched quote
        msg_warn("utility", "tokenizer unable to reliably parse a string with un-matched quotes, treating as unquoted text." << eom);
    }
    return quotes->size();
}

std::string 
MHO_Tokenizer::TrimLeadingAndTrailingWhitespace(const std::string& value) const
{
    std::string ret_val = "";
    std::string whitespace = " \t";
    auto start = value.find_first_not_of(whitespace);
    if(start == std::string::npos){return ret_val;}
    auto stop = value.find_last_not_of(whitespace);
    std::size_t length = stop - start + 1;
    ret_val = value.substr(start, length);
    return ret_val;
}

}//end namespace 

