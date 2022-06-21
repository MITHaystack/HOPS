#ifndef MHO_VexDefinitions_HH__
#define MHO_VexDefinitions_HH__

/*
*@file: MHO_VexDefinitions.hh
*@class: MHO_VexDefinitions
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
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

        vex_element_type DetermineType(std::string etype) const;

        std::vector< std::string > GetBlockNames() const {return fBlockNames;}

        std::string StartTag() const {return fStartTag;};
        std::string StopTag() const {return fStopTag;};
        std::string BlockStartFlag() const {return fBlockStartFlag;};
        std::string ChanDefTag() const {return fChanDefTag;};
        std::string IFDefTag() const {return fIFDefTag;};
        std::string RefTag() const {return fRefTag;}
        std::string VexDelim() const {return fVexDelim;};
        std::string StartTagDelim() const {return fStartTagDelim;};
        std::string AssignmentDelim() const {return fAssignmentDelim;};
        std::string WhitespaceDelim() const {return fWhitespaceDelim;};
        std::string VexRevisionFlag() const {return fVexRevisionFlag;};
        std::string StartLiteralFlag() const {return fStartLiteralFlag;};
        std::string EndLiteralFlag() const {return fEndLiteralFlag;};
        std::string CommentFlag() const {return fCommentFlag;};
        std::string StatementEndFlag() const {return fStatementEndFlag;};

    private:

        std::string fFormatDirectory;
        std::string fVexVersion;
        std::vector< std::string > fBlockNames;

        std::string fStartTag;
        std::string fStopTag;
        std::string fBlockStartFlag;
        std::string fChanDefTag;
        std::string fIFDefTag;
        std::string fRefTag;
        std::string fVexDelim;
        std::string fStartTagDelim;
        std::string fStatementEndFlag;
        std::string fAssignmentDelim;
        std::string fWhitespaceDelim;
        std::string fVexRevisionFlag;
        std::string fStartLiteralFlag;
        std::string fEndLiteralFlag;

        std::string fWhitespace;
        std::string fCommentFlag;



};

}

#endif /* end of include guard: MHO_VexDefinitions */