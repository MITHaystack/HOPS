/*
 * difxinput2json - convert a DiFX .input file into a JSON description
 *
 * Copyright (C) 2026 Massachusetts Institute of Technology, Haystack Observatory
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * NOTE: this program links against the (GPL) difxio library and is therefore
 * the *only* HOPS executable distributed under the GPL.  It is deliberately
 * isolated here so the remainder of HOPS can stay MIT-licensed: the rest of the
 * toolchain (e.g. difx2hops) consumes the JSON this program emits, rather than
 * linking difxio directly.
 */

#include <fstream>
#include <iostream>
#include <string>

#include "MHO_DiFXInputProcessor.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"

#include "CLI11.hpp"

//NOTE: difx_options.h only showed up in DiFX v2.8.1; for older versions the
//'--localdir' option is unavailable (HAVE_DIFX_OPTS is set by CMake when the
//header is present).
#ifdef HAVE_DIFX_OPTS
    #include "difxio/difx_options.h"
#endif

//difxio version this binary was compiled against
#ifndef HOPS_DIFXIO_VERSION
    #define HOPS_DIFXIO_VERSION "unknown"
#endif

using namespace hops;

int main(int argc, char** argv)
{
    std::string input_file;
    std::string output_file = "";
    bool localdir = false;
    bool pretty = false;
    int message_level = 5; //default is silent...we only want the JSON emitted

    CLI::App app{"difxinput2json: convert a DiFX .input file to a JSON description"};

    app.set_version_flag("--difxio-version", std::string("difxio ") + HOPS_DIFXIO_VERSION,
                         "print the difxio library version this program was compiled against and exit");

    app.add_option("input_file", input_file, "the DiFX .input file to convert")->required();
    app.add_option("-o,--output", output_file, "output JSON file (default: write to stdout)");
    app.add_flag("-l,--localdir", localdir,
                 "enable the difxio --localdir option (search for referenced files in the local directory)");
    app.add_flag("-p,--pretty", pretty, "pretty-print (indent) the JSON output");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");

    CLI11_PARSE(app, argc, argv);

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(localdir)
    {
#ifdef HAVE_DIFX_OPTS
        int true_val = 1;
        difxioSetOption(DIFXIO_OPT_LOCALDIR, &true_val);
#endif
    }

    MHO_DiFXInputProcessor proc;
    proc.LoadDiFXInputFile(input_file);

    mho_json input;
    proc.ConvertToJSON(input);

    std::string output;
    if(pretty)
    {
        output = input.dump(2);
    }
    else
    {
        output = input.dump();
    }

    if(output_file.empty())
    {
        std::cout << output << std::endl;
    }
    else
    {
        std::ofstream ofs(output_file.c_str());
        if(!ofs.is_open())
        {
            msg_fatal("difx_interface", "could not open output file: " << output_file << " for writing." << eom);
            return 1;
        }
        ofs << output << std::endl;
        ofs.close();
    }

    return 0;
}
