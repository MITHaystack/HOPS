#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_Clock.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

using namespace hops;

#define MAX_NUM_FREQS 64

bool isComment(const std::string& str)
{
    std::string whitespace_chars = " \t";
    // Find the first non-whitespace character
    size_t start = str.find_first_not_of(whitespace_chars);
    if(start == std::string::npos)
    {
        return true;
    }
    if(str[start] == '*')
    {
        return true;
    }
    return false;
}

void decodeFlagBytes(const char* buf)
{
    char bytes[MAX_NUM_FREQS + 2] = {'\0'};
    int offset, nibble, bread, addr, usb, lastusb, lastlsb;
    int limit = (int)strlen(buf); /* advance through all chars, stop at null */

    bread = 0;
    offset = 0;
    lastusb = offset;
    lastlsb = offset + 1;
    for(addr = 0, usb = 1; addr < MAX_NUM_FREQS; usb = !usb)
    {
        if(buf[offset])
            nibble = buf[offset];
        else if(usb)
            nibble = buf[lastusb];
        else
            nibble = buf[lastlsb];
        if(nibble >= '0' && nibble <= '9')
            nibble -= '0';
        else if(nibble >= 'A' && nibble <= 'F')
            nibble -= ('A' - 10);
        else if(nibble >= 'a' && nibble <= 'f')
            nibble -= ('a' - 10);
        else
        {
            std::cout << "illegal char" << std::endl;
            // msg("Illegal %x line %d char %d of %s, disabling flag", 2,
            //     nibble, load, offset, file);
            nibble = 0xF;
        }
        if(usb)
            bytes[bread + addr] = nibble << 4;
        else
            bytes[bread + addr++] |= nibble;
        if(buf[offset])
        {
            if(usb)
                lastusb = offset;
            else
                lastlsb = offset;
        }
        if(offset < limit)
            offset++;
    }
    bread += addr;
    bytes[bread] = 0;

    for(std::size_t i = 0; i < bread; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast< int >(bytes[i]) << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    std::string usage = "TestReadAdhocFlagFile <filename>";

    int year = 2024;

    if(argc != 2)
    {
        std::cout << "Usage: " << usage << std::endl;
        return 1;
    }
    std::string filename = std::string(argv[1]);

    std::ifstream input(filename);
    if(!input)
    {
        std::cout << "could not open file" << std::endl;
        return 1;
    }
    else
    {
        //read each line of the file using getline and tokenize it
        std::string line;
        std::vector< std::string > tokens;
        MHO_Tokenizer tokenizer;

        std::string whitespace_chars = " \t";
        tokenizer.SetDelimiter(whitespace_chars);
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();

        while(std::getline(input, line))
        {
            if(!isComment(line))
            {
                tokenizer.SetString(&line);
                tokenizer.GetTokens(&tokens);
                for(auto it = tokens.begin(); it != tokens.end(); it++)
                {
                    std::cout << *it << " | ";
                }
                std::cout << std::endl;

                if(tokens.size() == 2)
                {
                    double fpday;
                    std::stringstream ss;
                    ss << tokens[0];
                    ss >> fpday;

                    auto tp = hops_clock::from_year_fpday(year, fpday);
                    std::string isodate = hops_clock::to_iso8601_format(tp);
                    std::cout << isodate << ": " << tokens[1] << std::endl;

                    decodeFlagBytes(tokens[1].c_str());
                }
            }
        }

        input.close();
        return 0;
    }
}
