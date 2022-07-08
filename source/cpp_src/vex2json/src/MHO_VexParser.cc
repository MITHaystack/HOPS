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
    std::string rev = MHO_VexDefinitions::DetermineFileVersion(fVexFileName);
    SetVexVersion(rev);
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
        if(com_pos != std::string::npos || it->fContents.size() == 0)
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
MHO_VexParser::SplitStatements()
{
    //primitive search for start/end literal statements
    std::string statement_end = fVexDef.StatementEndFlag();
    auto it = fLines.begin();
    while(it != fLines.end())
    {
        std::size_t n_stmt = std::count( it->fContents.begin(), it->fContents.end(), statement_end[0]);
        if(n_stmt != 1)
        {
            //split this statement into multiple 'lines'
            std::vector< std::size_t > positions;
            std::vector< MHO_VexLine > split_lines;
            for(std::size_t i=0; i<it->fContents.size(); i++)
            {
                if( it->fContents[i] == ';')
                {
                    positions.push_back(i);
                    split_lines.push_back(*it);
                }
            }

            std::size_t start = 0;
            std::size_t length = 0;
            for(std::size_t i=0; i<split_lines.size(); i++)
            {
                length = positions[i] + 1 - start;
                split_lines[i].fContents = it->fContents.substr(start,length);
                start = positions[i]+1;
            }
            it = fLines.erase(it);
            fLines.insert(it, split_lines.begin(), split_lines.end());
        }
        else{++it;};
    }
}

void
MHO_VexParser::IndexStatements()
{
    std::size_t statement_idx = 0;
    auto it = fLines.begin();
    while(it != fLines.end())
    {
        it->fStatementNumber = statement_idx;
        ++statement_idx;
        ++it;
    }
}

void
MHO_VexParser::JoinLines()
{
    //every line/statement should be terminated with a ";"
    //so we concatenate lines which are missing a ";"
    std::string statement_end = MHO_VexDefinitions::StatementEndFlag();
    std::vector< MHO_VexLine > prepend_statements;
    auto it = fLines.begin();
    while(it != fLines.end())
    {
        if( it->fContents.find(statement_end) == std::string::npos )
        {
            //this line is missing a statement end ";"
            prepend_statements.push_back(*it);
            it = fLines.erase(it); //remove this line as a separate entity
        }
        else
        {
            if(prepend_statements.size() != 0 )
            {
                //concatenate all of the prepending statements and insert 
                //them at the front of this line
                std::string full_statement;
                for(std::size_t i=0; i<prepend_statements.size(); i++)
                {
                    full_statement += prepend_statements[i].fContents + " ";
                }
                std::string current_contents = it->fContents;
                it->fContents = full_statement + current_contents;
                prepend_statements.clear();
            }
            ++it;
        }
    }
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
        std::size_t line_no = line_it->fStatementNumber;
        std::string next_blk = "";
        std::size_t min_diff = fLines.size();

        for(auto blk_it2 = fFoundBlocks.begin(); blk_it2 != fFoundBlocks.end(); blk_it2++)
        {
            auto line_it2 = fBlockStartLines[*blk_it2];
            std::size_t line_no2 = line_it2->fStatementNumber;
            if(blk_it != blk_it2 && line_no <= line_no2)
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
    ReadFile(); //read file into memory
    RemoveComments(); //excise all comments
    JoinLines(); //join statements split across multiple lines
    MarkLiterals(); //excise all 'literal' sections
    SplitStatements(); //split multiple ";" on one line into as many statements as needed
    IndexStatements();
    MarkBlocks(); //mark the major parsable sections

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
        msg_debug("vex", "processing block: "<<block_name<<eom);
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