#ifndef MHO_VexDefinitions_HH__
#define MHO_VexDefinitions_HH__

/*
*@file: MHO_VexDefinitions.hh
*@class: MHO_VexDefinitions
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: Basic definitions of various tokens and utilities
*/

#include <string>
#include <vector>
#include "MHO_Message.hh"

namespace hops 
{

enum vex_element_type
{
    vex_int_type,
    vex_list_int_type,
    vex_real_type,
    vex_string_type,
    vex_list_string_type,
    vex_epoch_type,
    vex_radec_type,
    vex_list_real_type,
    vex_compound_type,
    vex_list_compound_type,
    vex_link_type,
    vex_keyword_type,
    vex_reference_type,
    vex_unknown_type
};


class MHO_VexDefinitions
{
    public:
        MHO_VexDefinitions();
        virtual ~MHO_VexDefinitions();

        void SetVexVersion(std::string version);
        void SetVexVersion(const char* version);

        std::string GetFormatDirectory() const;
        std::vector< std::string > GetBlockNames() const {return fBlockNames;}

        static std::string DetermineFileVersion(std::string filename);

        static std::string BlockStartFlag() {return std::string("$");};
        static std::string RefTag() {return std::string("ref");}
        static std::string WhitespaceDelim() {return std::string(" \t\r\n");};
        static std::string AssignmentOp() {return std::string("=");};
        static std::string AssignmentDelim() {return std::string("=;");};
        static std::string StartTagDelim() {return std::string(" \t\r\n;");};
        static std::string ElementDelim() {return ":";};

        static std::string VexRevisionFlag() {return std::string("VEX_rev");};
        static std::string OVexRevisionFlag() {return std::string("$OVEX_REV");};
        static std::string StartLiteralFlag() {return std::string("start_literal");};
        static std::string EndLiteralFlag() {return std::string("end_literal");};
        static std::string CommentFlag() {return std::string("*");};
        static std::string StatementEndFlag() {return std::string(";");};
        static std::string StatementLineEnd() {return std::string(";\n");};

        static std::string OptionalFlag() {return std::string("#");};

        static vex_element_type DetermineType(std::string etype);

        static bool IsOptionalField(std::string& field_name);

    private:

        std::string fFormatDirectory;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;

};

}

#endif /* end of include guard: MHO_VexDefinitions */