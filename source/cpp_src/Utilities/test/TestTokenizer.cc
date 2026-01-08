#include <iostream>
#include <string>
#include <vector>

#include "MHO_Tokenizer.hh"
#include "MHO_StationIdentity.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{

    MHO_Tokenizer tokenizer;
    tokenizer.SetIncludeEmptyTokensFalse();

    std::string test1 = "This is a string separated by spaces.";
    std::string test2 = "This|is|a|string|separated|by|pipes.";
    std::string test3 = "This \t is a\tstring|separated by a\nmix.";
    std::string test4 = "This<d>is<d>a<d>string<d>separated<d>by<d>a<d>multi-character<d>delimiter.";

    std::string test5 = "This is a string that \"contains \t quoted \t text\".";

    std::string delim1 = " ";
    std::string delim2 = "|";
    std::string delim3 = "| \t\r\n";
    std::string delim4 = "<d>";

    std::vector< std::string > tok1;
    std::vector< std::string > tok2;
    std::vector< std::string > tok3;
    std::vector< std::string > tok4;
    std::vector< std::string > tok5;
    std::vector< std::string > tok6;

    tokenizer.SetDelimiter(delim1);
    tokenizer.SetString(&test1);
    tokenizer.GetTokens(&tok1);

    tokenizer.SetDelimiter(delim2);
    tokenizer.SetString(&test2);
    tokenizer.GetTokens(&tok2);

    tokenizer.SetDelimiter(delim3);
    tokenizer.SetString(&test3);
    tokenizer.GetTokens(&tok3);

    tokenizer.SetDelimiter(delim4);
    tokenizer.SetUseMulticharacterDelimiterTrue();
    tokenizer.SetString(&test4);
    tokenizer.GetTokens(&tok4);

    tokenizer.SetDelimiter(delim1);
    tokenizer.SetUseMulticharacterDelimiterFalse();
    tokenizer.SetPreserveQuotesTrue();
    tokenizer.SetString(&test5);
    tokenizer.GetTokens(&tok5);

    tokenizer.SetDelimiter(delim3);
    tokenizer.SetPreserveQuotesTrue();
    tokenizer.SetString(&test5);
    tokenizer.GetTokens(&tok6);

    for(std::size_t i = 0; i < tok1.size(); i++)
    {
        std::cout << tok1[i] << " ";
    }
    std::cout << std::endl;
    for(std::size_t i = 0; i < tok2.size(); i++)
    {
        std::cout << tok2[i] << " ";
    }
    std::cout << std::endl;
    for(std::size_t i = 0; i < tok3.size(); i++)
    {
        std::cout << tok3[i] << " ";
    }
    std::cout << std::endl;
    for(std::size_t i = 0; i < tok4.size(); i++)
    {
        std::cout << tok4[i] << " ";
    }
    std::cout << std::endl;
    for(std::size_t i = 0; i < tok5.size(); i++)
    {
        std::cout << tok5[i] << " ";
    }
    std::cout << std::endl;
    for(std::size_t i = 0; i < tok6.size(); i++)
    {
        std::cout << tok6[i] << " ";
    }
    std::cout << std::endl;

    std::string val = "    mean_motion = 0.; ";
    std::string fAssignmentDelim = "=;";
    tokenizer.SetDelimiter(fAssignmentDelim);
    tokenizer.SetUseMulticharacterDelimiterFalse();
    tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    tokenizer.SetIncludeEmptyTokensFalse();

    std::vector< std::string > tok7;
    tokenizer.SetString(&val);
    tokenizer.GetTokens(&tok7);

    std::string polprod = "XX+YY";
    std::vector< std::string > tok9;
    //we have a pol-product summation like (RR+LL) or XX+YY, or RX+RY
    //so split on all '+' symbols (currently we only support '+' not '-')
    tokenizer.SetDelimiter("+");
    tokenizer.SetUseMulticharacterDelimiterFalse();
    tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    tokenizer.SetIncludeEmptyTokensFalse();
    tokenizer.SetString(&polprod);
    tokenizer.GetTokens(&tok9);
    for(std::size_t i = 0; i < tok9.size(); i++)
    {
        std::cout << tok9[i] << " ";
    }
    std::cout << std::endl;

    for(std::size_t i = 0; i < tok7.size(); i++)
    {
        std::cout << tok7[i] << "|";
    }
    std::cout << std::endl;
    
    
    //minor test of the station identifier class
    MHO_StationIdentity west;
    west.SetAll("WESTFORD", "Wf", "E");
    
    std::cout<<"station: "<<west.as_string()<<std::endl;
    
    MHO_StationIdentity ggao;
    ggao.SetAll("GGAO12M", "Gs", "G");
    
    std::cout<<"station: "<<ggao.as_string()<<std::endl;
    
    bool ok = (ggao == west);
    std::cout<<"equivalent? "<< ok <<std::endl;

    std::cout<<"Westford matches 'E'? "<<west.matches("E")<<std::endl;
    std::cout<<"Westford matches 'G'? "<<west.matches("G")<<std::endl;
    std::cout<<"GGAO matches 'G'? "<<ggao.matches("G")<<std::endl;

    return 0;
}
