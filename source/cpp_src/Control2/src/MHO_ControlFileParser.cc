#include "MHO_ControlFileParser.hh"

#include <fstream>


namespace hops 
{

MHO_ControlFileParser::MHO_ControlFileParser()
{

}


MHO_ControlFileParser::~MHO_ControlFileParser(){};

void 
MHO_ControlFileParser::SetControlFile(std::string filename)
{
    fControlFileName = filename;
}

//mho_json
void
MHO_ControlFileParser::ParseControl()
{
    ReadFile(); //read file into memory
    RemoveComments(); //excise all comments
    
    
    for(auto it = fLines.begin(); it != fLines.end(); it++)
    {
        std::cout<<it->fContents<<std::endl;
    }
    
    // SplitStatements(); //split multiple ";" on one line into as many statements as needed
    // JoinLines(); //join incomplete segments split across multiple lines into a single statement
    // IndexStatements();
    // MarkBlocks(); //mark the major parsable sections

    // mho_json root;
    // root[fControlDef.ControlRevisionFlag()] = fControlVersion;
    // ProcessBlocks(root);

    // return root;
}

void 
MHO_ControlFileParser::ReadFile()
{
    //nothing special, just read in the entire file line by line and stash in memory
    if(fControlFileName != "")
    {
        //open input/output files
        std::ifstream vfile(fControlFileName.c_str(), std::ifstream::in);
        if(vfile.is_open() )
        {
            std::size_t line_count = 1;
            std::string contents;
            while( getline(vfile, contents) )
            {
                MHO_ControlLine current_line;
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
            msg_error("vex", "could not open file: "<<fControlFileName<<eom);
        }
    }
}

void 
MHO_ControlFileParser::RemoveComments()
{
    std::string flag = "*"; //fControlDef.CommentFlag();
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

/*


void
MHO_ControlFileParser::SplitStatements()
{
    
    std::string whitespace_delims(" \t\r\n");
    fTokenizer.SetDelimiter(whitespace_delims);
    //fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetIncludeEmptyTokensFalse();
    
    
    //primitive search for start/end literal statements
    auto it = fLines.begin();
    while(it != fLines.end())
    {
        
        fTokenizer.
        it->fContents;
        
        
        
        bool must_split = false;
        std::size_t n_stmt = std::count( it->fContents.begin(), it->fContents.end(), statement_end[0]);
        if(n_stmt > 1){must_split = true;}
    
        //another condition for splitting the line would be if we have something 
        //messy like the following (still legal according to the standard):
        //    def K2; VSN = 1 : HOB+0093 : 
        //    2019y133d00h00m : 2019y134d23h59m 
        //    ; enddef;
        //to detect this we need to check if there are any non white space characters 
        //after the presence of a ';' (comments should be stripped at this point)
    
        std::string whitespace_chars = MHO_ControlDefinitions::WhitespaceDelim();
        if(n_stmt >= 1)
        {
            std::size_t last_flag_pos = it->fContents.find_last_of(statement_end);
            for(std::size_t ch = last_flag_pos; ch < it->fContents.size(); ch++)
            {
                if( whitespace_chars.find( it->fContents[ch] ) == std::string::npos)
                {
                    //there is a non-whitespace character here, so we must split this line at the ';'
                    must_split = true;
                    break;
                }
            }
        }
    
    
        if(must_split)
        {
            //split this statement into multiple 'lines'
            std::vector< std::size_t > positions;
            std::vector< MHO_ControlLine > split_lines;
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
MHO_ControlFileParser::IndexStatements()
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
MHO_ControlFileParser::JoinLines()
{
    //every line/statement should be terminated with a ";"
    //so we concatenate lines which are missing a ";"
    std::string statement_end = MHO_ControlDefinitions::StatementEndFlag();
    std::vector< MHO_ControlLine > prepend_statements;
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
MHO_ControlFileParser::MarkBlocks()
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
                            << fControlFileName << " found on line #" << 
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


void 
MHO_ControlFileParser::ProcessBlocks(mho_json& root)
{
    for(auto blk_it = fFoundBlocks.begin(); blk_it != fFoundBlocks.end(); blk_it++)
    {
        std::string block_name = *blk_it;
        msg_debug("vex", "processing block: "<<block_name<<eom);
        std::vector< MHO_ControlLine > block_data = CollectBlockLines(block_name);
        mho_json block = fBlockParser.ParseBlockLines(block_name, &block_data);
        root[block_name] = block;
    }
}


std::vector< MHO_ControlLine >
MHO_ControlFileParser::CollectBlockLines(std::string block_name)
{
    std::vector< MHO_ControlLine > lines;
    auto start_it = fBlockStartLines[block_name];
    auto stop_it = fBlockStopLines[block_name];
    for(auto it = start_it; it != stop_it; it++)
    {
        lines.push_back(*it);
    }
    return lines;
}

bool 
MHO_ControlFileParser::IsPotentialBlockStart(std::string line)
{
    //first determine if a "$" is on this line
    std::string block_start_flag = fControlDef.BlockStartFlag();
    std::string ref_flag = fControlDef.RefTag();

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
MHO_ControlFileParser::IsBlockStart(std::string line, std::string blk_name)
{
    auto loc = line.find(blk_name);
    if(loc != std::string::npos)
    {
        //blk_name exists on this line...but is it an exact match?
        //this check is mainly needed to resolve $SCHED and $SCHEDULING_PARAMS
        auto start = line.find(fControlDef.BlockStartFlag());
        auto stop = line.find(fControlDef.StatementEndFlag());
        std::string sub = line.substr(start, stop);
        if(sub == blk_name){return true;}
        return false;
    }
    return false;
}

*/

}//end namespace
