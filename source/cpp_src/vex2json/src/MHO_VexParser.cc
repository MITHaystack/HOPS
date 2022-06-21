#include "MHO_VexParser.hh"

#include <fstream>


namespace hops 
{

MHO_VexParser::MHO_VexParser()
{
    fVexRevisionFlag = "VEX_rev";
    fVexDelim = " :;\t\r\n";
    fWhitespace = " \t\r\n";
    fCommentFlag = "*";
    fBlockStartFlag = "$";
    fRefFlag = "ref";
    fVexVersion = "1.5";
    fStatementEndFlag = ";";
    fFormatDirectory = VEX_FORMAT_DIR;
    fStartLiteralFlag = "start_literal";
    fEndLiteralFlag = "end_literal";
    SetVexVersion(fVexVersion);
}


MHO_VexParser::~MHO_VexParser(){};

void 
MHO_VexParser::SetVexFile(std::string filename)
{
    fVexFileName = filename;
    DetermineFileVersion();
}

void 
MHO_VexParser::DetermineFileVersion()
{
    //read the first line to determine the vex revision
    //vex standard states the revision statement must start at 1st char of 1st line
    bool determined_rev = false;
    if(fVexFileName != "")
    {
        //open input/output files
        std::ifstream vfile(fVexFileName.c_str(), std::ifstream::in);
        if(vfile.is_open() )
        {
            std::size_t line_count = 1;
            std::string contents;
            getline(vfile, contents);
            std::size_t rev_pos = contents.find(fVexRevisionFlag);
            if(rev_pos != std::string::npos)
            {
                std::size_t start_pos = contents.find_first_of("=");
                std::size_t end_pos = contents.find_first_of(";");
                if(start_pos != std::string::npos && end_pos != std::string::npos)
                {
                    std::string rev = contents.substr(start_pos, end_pos-start_pos);
                    std::string revision = MHO_Tokenizer::TrimLeadingAndTrailingWhitespace(rev);
                    SetVexVersion(revision);
                    determined_rev = true;
                }
            }
            vfile.close();
        }
        else 
        {
            msg_error("vex", "could not open file: "<<fVexFileName<<eom);
        }
    }

    if(!determined_rev)
    {
        msg_error("vex", "could not determine vex revision, defaulting to version: "<< fVexVersion << eom);
    }
}

void 
MHO_VexParser::ReadFile()
{
    //nothing special, just read in the entire file line by line and stash in memory
    if(fVexFileName != "")
    {
        //open input/output files
        std::ifstream vfile(fVexFileName.c_str(), std::ifstream::in);
        if(vfile.is_open() )
        {
            std::size_t line_count = 1;
            std::string contents;
            while( getline(vfile, contents) )
            {
                MHO_VexLine current_line;
                current_line.fLineNumber = line_count;
                current_line.fContents = contents;
                current_line.fIsLiteral = false;
                fLines.push_back(current_line);
                line_count++;
            }
            vfile.close();
        }
        else 
        {
            msg_error("vex", "could not open file: "<<fVexFileName<<eom);
        }
    }
}

void 
MHO_VexParser::RemoveComments()
{
    for(auto it = fLines.begin(); it != fLines.end();)
    {
        std::size_t com_pos = it->fContents.find_first_of(fCommentFlag);
        if(com_pos != std::string::npos)
        {
            if(com_pos == 0){it = fLines.erase(it);} //this entire line is a comment
            else
            {
                std::string trimmed = it->fContents.substr(0,com_pos);
                if(trimmed.size() != 0)
                {
                    it->fContents = trimmed;
                    it++;
                }
                else{it = fLines.erase(it);}
            }
        }
        else{it++;}
    }
}

void 
MHO_VexParser::MarkLiterals()
{
    //primitive search for start/end literal statements
    auto it = fLines.begin();
    while(it != fLines.end())
    {
        std::size_t start_lit_pos = it->fContents.find(fStartLiteralFlag);
        if(start_lit_pos != std::string::npos)
        {
            bool found_end = false;
            while( !found_end && it != fLines.end() )
            {
                std::size_t end_lit_pos = it->fContents.find(fEndLiteralFlag);
                if(end_lit_pos != std::string::npos){found_end = true;}
                it->fIsLiteral = true;
                ++it;
            }
        }
        else{ ++it; }
    }
}

void
MHO_VexParser::JoinLines()
{
    //TODO FIXME - implement this 
    msg_warn("vex", "multi-line vex statements currently not supported." << eom);
    //The vex standard explicitly allows for a single vex-statement to be split across multiple lines,
    //so long as it is terminated by a ';'. However, due to the way in which the file was read in (getline)
    //and the fact that trailing comments need to be removed on a per-line basis, we need to implement 
    //the ability to re-join a vex statment spread over multiple-lines into a single ';' terminated string,
    //so that it can be tokenized and parsed more easily.
}

void 
MHO_VexParser::MarkBlocks()
{
    //brute force search for block start tags
    fBlockStartLines.clear();
    fBlockStopLines.clear();
    fFoundBlocks.clear();
    for(auto it = fLines.begin(); it != fLines.end(); it++)
    {
        if(IsPotentialBlockStart(it->fContents))
        {
            for(auto blk_it = fBlockNames.begin(); blk_it != fBlockNames.end(); blk_it++)
            {
                if(IsBlockStart(it->fContents, *blk_it))
                {
                    if(fFoundBlocks.count(*blk_it) == 0)
                    {
                        fBlockStartLines[*blk_it] = it;
                        fFoundBlocks.insert(*blk_it);
                        msg_debug("vex", "found block: "<<*blk_it<<" on line: "<< fBlockStartLines[*blk_it]->fLineNumber << eom);
                        break;
                    }
                    else 
                    {
                        //error/warning! multiple blocks of the same type in the vex file 
                        msg_error
                        (
                            "vex", "duplicate "<< *blk_it <<" block within vex file: "
                            << fVexFileName << " found on line #" << 
                            it->fLineNumber << "." << eom 
                        );
                    }
                }
            }
        }
    }

    //now figure out where each block ends (at the start of the following block)
    for(auto blk_it = fFoundBlocks.begin(); blk_it != fFoundBlocks.end(); blk_it++)
    {
        auto line_it = fBlockStartLines[*blk_it];
        std::size_t line_no = line_it->fLineNumber;
        std::string next_blk = "";
        std::size_t min_diff = fLines.size();

        for(auto blk_it2 = fFoundBlocks.begin(); blk_it2 != fFoundBlocks.end(); blk_it2++)
        {
            auto line_it2 = fBlockStartLines[*blk_it2];
            std::size_t line_no2 = line_it2->fLineNumber;
            if(blk_it != blk_it2 && line_no < line_no2)
            {
                std::size_t diff = line_no2 - line_no;
                if(diff < min_diff)
                {
                    min_diff = diff;
                    next_blk = *blk_it2;
                }
            }
        }

        if(next_blk != "")
        {
            fBlockStopLines[*blk_it] = fBlockStartLines[next_blk];
            //fBlockStopLines[*blk_it]--; //decrement iterator (--), to point to line just before the next block
        }
        else 
        {
            fBlockStopLines[*blk_it] = fLines.end();
            //fBlockStopLines[*blk_it]--; //decrement iterator (--), to point to line just before the next block
        }
    }

    // for(auto blk_it = fBlockNames.begin(); blk_it != fBlockNames.end(); blk_it++)
    // {
    //     std::cout<<"block: "<<*blk_it<<" starts on line: "<<
    //     fBlockStartLines[*blk_it]->fLineNumber << " and ends on line: "<<
    //     fBlockStopLines[*blk_it]->fLineNumber << std::endl;
    // }

}

mho_json
MHO_VexParser::ParseVex()
{
    ReadFile();
    RemoveComments();
    MarkLiterals();
    JoinLines();
    MarkBlocks();

    mho_json root;
    root[fVexRevisionFlag] = fVexVersion;
    ProcessBlocks(root);

    return root;
}

void 
MHO_VexParser::ProcessBlocks(mho_json& root)
{
    for(auto blk_it = fFoundBlocks.begin(); blk_it != fFoundBlocks.end(); blk_it++)
    {
        std::string block_name = *blk_it;
        std::vector< MHO_VexLine > block_data = CollectBlockLines(block_name);
        mho_json block = fBlockParser.ParseBlockLines(block_name, &block_data);
        root[block_name] = block;
    }
}


std::vector< MHO_VexLine >
MHO_VexParser::CollectBlockLines(std::string block_name)
{
    std::vector< MHO_VexLine > lines;
    auto start_it = fBlockStartLines[block_name];
    auto stop_it = fBlockStopLines[block_name];
    for(auto it = start_it; it != stop_it; it++)
    {
        lines.push_back(*it);
    }
    return lines;
}

bool 
MHO_VexParser::IsPotentialBlockStart(std::string line)
{
    //first determine if a "$" is on this line
    auto loc = line.find(fBlockStartFlag); 
    if(loc != std::string::npos)
    {
        //make sure "ref" is not on this line 
        auto ref_loc = line.find(fRefFlag);
        if(ref_loc == std::string::npos)
        {
            return true;
        }
    }
    return false;
}

bool 
MHO_VexParser::IsBlockStart(std::string line, std::string blk_name)
{
    auto loc = line.find(blk_name);
    if(loc != std::string::npos)
    {
        //blk_name exists on this line...but is it an exact match?
        //this check is mainly needed to resolve $SCHED and $SCHEDULING_PARAMS
        auto start = line.find(fBlockStartFlag);
        auto stop = line.find(fStatementEndFlag);
        std::string sub = line.substr(start, stop);
        if(sub == blk_name){return true;}
        return false;
    }
    return false;
}

void 
MHO_VexParser::SetVexVersion(const char* version)
{
    std::string vers(version);
    SetVexVersion(vers);
}

void 
MHO_VexParser::SetVexVersion(std::string version)
{
    fVexVersion = "1.5";
    if(version.find("1.5") != std::string::npos ){fVexVersion = "1.5";}
    else if(version.find("2.0") != std::string::npos ){fVexVersion = "2.0";}
    else 
    {
        msg_error("vex", "version string: "<< version << "not understood, defaulting to vex version 1.5." << eom );
    }

    fFormatDirectory = GetFormatDirectory();
    fBlockParser.SetFormatDirectory(fFormatDirectory);
    std::string bnames_file = fFormatDirectory + "block-names.json";
    msg_debug("vex", "block name file is: "<< bnames_file << eom);

    std::ifstream bn_ifs;
    bn_ifs.open( bnames_file.c_str(), std::ifstream::in );

    json bnames;
    if(bn_ifs.is_open())
    {
        bnames = mho_ordered_json::parse(bn_ifs);
    }
    bn_ifs.close();

    fBlockNames.clear();
    for(auto it = bnames["block_names"].begin(); it != bnames["block_names"].end(); it++)
    {
        fBlockNames.push_back(*it);
    }
}

std::string 
MHO_VexParser::GetFormatDirectory() const
{
    std::string format_dir = VEX_FORMAT_DIR;
    format_dir += "/vex-" + fVexVersion + "/";
    return format_dir;
}

}