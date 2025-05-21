#include "MHO_VexGenerator.hh"
#include <fstream>

namespace hops
{

MHO_VexGenerator::MHO_VexGenerator()
{
    fSpace = " ";
    fIndentPad = "    "; //4 spaces
    fVexRevisionFlag = MHO_VexDefinitions::VexRevisionFlag();
}

MHO_VexGenerator::~MHO_VexGenerator(){};

void MHO_VexGenerator::SetFilename(std::string filename)
{
    fFilename = filename;
}

void MHO_VexGenerator::GenerateVex(mho_json& root)
{
    std::vector< std::string > all_lines;
    //first line is always version line
    std::string vers = root[fVexRevisionFlag].get< std::string >();
    SetVexVersion(vers);
    if(vers != "ovex") //only output this line for vex, not ovex
    {
        std::string version_line =
            fVexRevisionFlag + MHO_VexDefinitions::AssignmentOp() + vers + MHO_VexDefinitions::StatementLineEnd();
        all_lines.push_back(version_line);
    }
    else if(vers == "ovex")
    {
        //handle the 'special' case of ovex and insert all this completely useless info
        all_lines.push_back("$OVEX_REV" + MHO_VexDefinitions::StatementLineEnd());
        all_lines.push_back("rev = 1.5" + MHO_VexDefinitions::StatementLineEnd());
        all_lines.push_back("$EVEX_REV" + MHO_VexDefinitions::StatementLineEnd());
        all_lines.push_back("rev = 1.0" + MHO_VexDefinitions::StatementLineEnd());
        all_lines.push_back("$IVEX_REV" + MHO_VexDefinitions::StatementLineEnd());
        all_lines.push_back("rev = 1.0" + MHO_VexDefinitions::StatementLineEnd());
        all_lines.push_back("$LVEX_REV" + MHO_VexDefinitions::StatementLineEnd());
        all_lines.push_back("rev = 1.0" + MHO_VexDefinitions::StatementLineEnd());
    }

    //open block-names file for this version
    //loop over blocks, and extract the data from from the root and append them
    for(auto blk_it = fBlockNames.begin(); blk_it != fBlockNames.end(); blk_it++)
    {
        std::string block_name = *blk_it;
        if(!IsExcludedOvex(block_name))
        {
            std::vector< std::string > block_lines;
            std::string block_opening = block_name + MHO_VexDefinitions::StatementLineEnd();
            all_lines.push_back(block_opening);
            ConstructBlockLines(root, block_name, block_lines);
            all_lines.insert(all_lines.end(), block_lines.begin(), block_lines.end());
        }
    }

    //open and dump to file
    std::ofstream outFile(fFilename.c_str(), std::ofstream::out);
    if(outFile.is_open())
        for(auto lit = all_lines.begin(); lit != all_lines.end(); lit++)
        {
            outFile << *lit;
        }
    outFile.close();
}

void MHO_VexGenerator::ConstructBlockLines(mho_json& root, std::string block_name, std::vector< std::string >& lines)
{
    lines.clear();
    LoadBlockFormat(block_name);

    fPad = fIndentPad;

    if(block_name == "$GLOBAL") //global block is special
    {
        if(root.contains(block_name))
        {
            mho_json block = root[block_name];
            ConstructReferenceLines(block, lines);
        }
    }
    else if(fBlockFormat["block_type"].get< std::string >() == "primitive")
    {
        std::string start_tag = fBlockFormat["start_tag"].get< std::string >();
        std::string stop_tag = fBlockFormat["stop_tag"].get< std::string >();
        if(root.contains(block_name))
        {
            mho_json block = root[block_name];
            for(auto element : block.items())
            {
                std::string element_key = element.key();
                std::string start_line = fPad + start_tag + " " + element_key + MHO_VexDefinitions::StatementLineEnd();
                lines.push_back(start_line);
                ConstructElementLines(root[block_name][element.key()], lines);
                //ovex is also special
                if(block_name == "$EVEX" || block_name == "$CORR_INIT")
                {
                    ConstructReferenceLines(root[block_name][element.key()], lines);
                }
                std::string stop_line = fPad + stop_tag + MHO_VexDefinitions::StatementLineEnd();
                lines.push_back(stop_line);
            }
        }
    }
    else if(fBlockFormat["block_type"].get< std::string >() == "high_level")
    {
        std::string start_tag = fBlockFormat["start_tag"].get< std::string >();
        std::string stop_tag = fBlockFormat["stop_tag"].get< std::string >();
        if(root.contains(block_name))
        {
            mho_json block = root[block_name];
            for(auto element : block.items())
            {
                std::string element_key = element.key();
                std::string start_line = fPad + start_tag + " " + element_key + MHO_VexDefinitions::StatementLineEnd();
                lines.push_back(start_line);
                ConstructReferenceLines(root[block_name][element.key()], lines);
                std::string stop_line = fPad + stop_tag + MHO_VexDefinitions::StatementLineEnd();
                lines.push_back(stop_line);
            }
        }
    }
}

void MHO_VexGenerator::ConstructElementLines(mho_json& element, std::vector< std::string >& lines)
{
    std::string prevPad = fPad;
    fPad += fIndentPad;

    //loop over items in format, and extract from element
    std::string bang = MHO_VexDefinitions::OptionalFlag();
    std::string nothing = "";
    for(auto field : fBlockFormat["fields"].items())
    {
        std::string raw_field_name = field.value().get< std::string >();
        //remove # prefix indicating optional elements
        std::string field_name = raw_field_name;
        if(raw_field_name[0] == bang[0])
        {
            field_name =  string_pattern_replace(raw_field_name, bang, nothing);
            //std::regex_replace(raw_field_name, std::regex(bang), nothing);
        }

        if(element.contains(field_name))
        {
            std::string par_type = fBlockFormat["parameters"][field_name]["type"].get< std::string >();
            if(par_type.find("list_compound") != std::string::npos)
            {
                for(std::size_t j = 0; j < element[field_name].size(); j++)
                {
                    std::string line = fPad + fLineGen.ConstructElementLine(field_name, element[field_name][j],
                                                                            fBlockFormat["parameters"][field_name]);
                    if(line.size() != 0)
                    {
                        lines.push_back(line);
                    }
                }
            }
            else if(par_type.find("compound") != std::string::npos)
            {
                std::string line = fPad + fLineGen.ConstructElementLine(field_name, element[field_name],
                                                                        fBlockFormat["parameters"][field_name]);
                if(line.size() != 0)
                {
                    lines.push_back(line);
                }
            }
            else if(par_type.find("reference") != std::string::npos)
            {
                //skip...this is a special case for 'ovex' (EVEX and CORR_INIT)
            }
            else
            {
                std::string line = fPad + fLineGen.ConstructElementLine(field_name, element[field_name],
                                                                        fBlockFormat["parameters"][field_name]);
                if(line.size() != 0)
                {
                    lines.push_back(line);
                }
            }
        }
    }
    fPad = prevPad;
}

void MHO_VexGenerator::ConstructReferenceLines(mho_json& element, std::vector< std::string >& lines)
{
    std::string prevPad = fPad;
    fPad += fIndentPad;
    //loop over items in format, and extract from element
    for(auto field : fBlockFormat["fields"].items())
    {
        std::string field_name = field.value().get< std::string >();
        if(element.contains(field_name))
        {
            std::string par_type = fBlockFormat["parameters"][field_name]["type"].get< std::string >();
            if(par_type.find("reference") != std::string::npos)
            {
                for(std::size_t j = 0; j < element[field_name].size(); j++)
                {
                    std::string line = fPad + MHO_VexDefinitions::RefTag() + fSpace + field_name + fSpace +
                                       MHO_VexDefinitions::AssignmentOp() + fSpace;
                    line += element[field_name][j]["keyword"].get< std::string >();

                    if(element[field_name][j].contains("qualifiers"))
                    {
                        for(std::size_t k = 0; k < element[field_name][j]["qualifiers"].size(); k++)
                        {
                            line += MHO_VexDefinitions::ElementDelim() +
                                    element[field_name][j]["qualifiers"][k].get< std::string >();
                        }
                    }
                    line += MHO_VexDefinitions::StatementLineEnd();
                    if(line.size() != 0)
                    {
                        lines.push_back(line);
                    }
                }
            }
        }
    }
    fPad = prevPad;
}

void MHO_VexGenerator::SetVexVersion(std::string version)
{
    fVexVersion = version;
    fVexDef.SetVexVersion(fVexVersion);
    fBlockNames = fVexDef.GetBlockNames();
    fFormatDirectory = fVexDef.GetFormatDirectory();
}

void MHO_VexGenerator::LoadBlockFormat(std::string block_name)
{
    fBlockFormatLoaded = false;
    std::string block_format_file = GetBlockFormatFileName(block_name);
    std::string format_file = fFormatDirectory + block_format_file;

    std::ifstream bf_ifs;
    bf_ifs.open(format_file.c_str(), std::ifstream::in);

    mho_json bformat;
    if(bf_ifs.is_open())
    {
        bformat = mho_json::parse(bf_ifs);
        fBlockFormatLoaded = true;
    }
    bf_ifs.close();

    if(fBlockFormatLoaded)
    {
        fBlockFormat = bformat;
    }
}

std::string MHO_VexGenerator::GetBlockFormatFileName(std::string block_name)
{
    //remove '$', and convert to lower-case
    std::string file_name = block_name.substr(1);
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    file_name += ".json";
    return file_name;
}

bool MHO_VexGenerator::IsExcludedOvex(std::string block_name)
{
    if(block_name == "$OVEX_REV")
    {
        return true;
    }
    if(block_name == "$EVEX_REV")
    {
        return true;
    }
    if(block_name == "$IVEX_REV")
    {
        return true;
    }
    if(block_name == "$LVEX_REV")
    {
        return true;
    }
    return false;
}

} // namespace hops
