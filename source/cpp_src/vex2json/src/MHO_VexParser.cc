#include "MHO_VexParser.hh"

#include <fstream>


namespace hops 
{

MHO_VexParser::MHO_VexParser()
{
    SetVexVersion("1.5");
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
                    std::string rev = contents.substr(start_pos+1, end_pos-start_pos-1);
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
    std::string flag = fVexDef.CommentFlag();
    for(auto it = fLines.begin(); it != fLines.end();)
    {
        std::size_t com_pos = it->fContents.find_first_of(flag);
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
    std::string start_flag = fVexDef.StartLiteralFlag();
    std::string end_flag = fVexDef.EndLiteralFlag();
    auto it = fLines.begin();
    while(it != fLines.end())
    {
        std::size_t start_lit_pos = it->fContents.find(start_flag);
        if(start_lit_pos != std::string::npos)
        {
            bool found_end = false;
            while( !found_end && it != fLines.end() )
            {
                std::size_t end_lit_pos = it->fContents.find(end_flag);
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
    //and the fact that trailing comments need to be removed on a per-line basis (as per standard), we need to implement 
    //the ability to re-join a vex statment that is spread over multiple-lines into a single ';' terminated string,
    //so that it can be tokenized and parsed.
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
        }
        else 
        {
            fBlockStopLines[*blk_it] = fLines.end();
        }
    }
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
    root[fVexDef.VexRevisionFlag()] = fVexVersion;
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
    std::string block_start_flag = fVexDef.BlockStartFlag();
    std::string ref_flag = fVexDef.RefTag();

    auto loc = line.find(block_start_flag); 
    if(loc != std::string::npos)
    {
        //make sure "ref" is not on this line 
        auto ref_loc = line.find(ref_flag);
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
        auto start = line.find(fVexDef.BlockStartFlag());
        auto stop = line.find(fVexDef.StatementEndFlag());
        std::string sub = line.substr(start, stop);
        if(sub == blk_name){return true;}
        return false;
    }
    return false;
}


void 
MHO_VexParser::SetVexVersion(std::string version)
{
    fVexVersion = version;
    fVexDef.SetVexVersion(fVexVersion);
    fBlockNames = fVexDef.GetBlockNames();
    fBlockParser.SetFormatDirectory(fVexDef.GetFormatDirectory());
}

}