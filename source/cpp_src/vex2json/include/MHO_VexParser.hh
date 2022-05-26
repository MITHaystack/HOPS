#ifndef MHO_VexParser_HH__
#define MHO_VexParser_HH__

/*
*@file: MHO_VexParser.hh
*@class: MHO_VexParser
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <vector>
#include <list>
#include <string>
#include <sstream>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{

class MHO_VexParser
{
    public:
        MHO_VexParser();
        virtual ~MHO_VexParser();



        void SetVexFile(std::string filename);
        void ReadFile();
        void RemoveComments();
        void ParseVex();

        void SetVexVersion(std::string version);
        void SetVexVersion(const char* version);
        std::string GetFormatDirectory();

        //testing only!! TODO REMOVE ME
        std::list< std::string >* GetLines(){return &fLines;};
        
    private:

        std::string fVexFileName;

        //tokenizer
        MHO_Tokenizer fTokenizer;
        std::string fVexDelim;
        std::string fWhitespace;
        std::string fCommentFlag;

        //workspace
        std::string fLine; //the line from the input vex file 
        std::list< std::string > fLines;
        std::vector< std::string > fTokens;

        //format definition 
        std::string fFormatDirectory;
        std::string fVexVersion;

};

}//end namespace

#endif /* end of include guard: MHO_VexParser */