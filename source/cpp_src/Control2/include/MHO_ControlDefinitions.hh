#ifndef MHO_ControlElements_HH__
#define MHO_ControlElements_HH__

/*
*@file: MHO_ControlElements.hh
*@class: MHO_ControlElements
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include <string>
#include <vector>

namespace hops
{

struct MHO_ControlLine
{
    std::size_t fLineNumber;
    std::string fContents;
    std::vector< std::string > fTokens;
};

struct MHO_ControlStatement
{
    std::string fKeyword;
    std::vector< std::string > fTokens;
};

enum control_element_type
{
    control_int_type,
    control_list_int_type,
    control_real_type,
    control_string_type,
    control_list_string_type,
    control_list_real_type,
    control_conditional_type,
    control_compound_type,
    control_unknown_type
};



class MHO_ControlDefinitions
{
    public:
        MHO_ControlDefinitions();
        virtual ~MHO_ControlDefinitions();

        std::string GetFormatDirectory() const;
        std::vector< std::string > GetBlockNames() const {return fBlockNames;}

        static std::string DetermineFileVersion(std::string filename);

        // static std::string BlockStartFlag() {return std::string("$");};
        // static std::string RefTag() {return std::string("ref");}
        // static std::string WhitespaceDelim() {return std::string(" \t\r\n");};
        // static std::string AssignmentOp() {return std::string("=");};
        // static std::string AssignmentDelim() {return std::string("=;");};
        // static std::string StartTagDelim() {return std::string(" \t\r\n;");};
        // static std::string ElementDelim() {return ":";};
        // 
        // static std::string VexRevisionFlag() {return std::string("VEX_rev");};
        // static std::string OVexRevisionFlag() {return std::string("$OVEX_REV");};
        // static std::string StartLiteralFlag() {return std::string("start_literal");};
        // static std::string EndLiteralFlag() {return std::string("end_literal");};
        // static std::string CommentFlag() {return std::string("*");};
        // static std::string StatementEndFlag() {return std::string(";");};
        // static std::string StatementLineEnd() {return std::string(";\n");};
        // 
        // static std::string OptionalFlag() {return std::string("#");};
        // 
        // static vex_element_type DetermineType(std::string etype);
        // 
        // static bool IsOptionalField(std::string& field_name);
        
        
        static control_element_type 
        DetermineControlType(std::string etype)
        {
            if(etype == "int"){return control_int_type;}
            if(etype == "list_int"){return control_list_int_type;}
            if(etype == "real"){return control_real_type;}
            if(etype == "list_real"){return control_list_real_type;}
            if(etype == "string"){return control_string_type;}
            if(etype == "list_string"){return control_list_string_type;}
            if(etype == "conditional"){return control_conditional_type;}
            if(etype == "compound"){return control_compound_type;}
            return control_unknown_type;
        }

    private:

        std::string fFormatDirectory;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;

};






}

#endif /* end of include guard: MHO_ControlElements */
