#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_DirectoryInterface.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//configure_data_library
#include "MHO_ElementTypeCaster.hh"

#include "MHO_VexInfoExtractor.hh"

//parse_command_line
#include "MHO_Tokenizer.hh"

//option parsing and help text library
#include "CLI11.hpp"

namespace hops
{


void
MHO_BasicFringeDataConfiguration::parse_baseline_freqgrp(std::string baseline_freqgrp, std::string& baseline, std::string& freqgrp)
{
    MHO_Tokenizer tokenizer;
    if( baseline_freqgrp.find(':') == std::string::npos )
    {
        //no ':' present, so we must only have a baseline specification
        baseline = baseline_freqgrp;
        freqgrp = "?"; //not passed, set to wildcard
        if(baseline.size() != 2) //error out if something odd was passed
        {
            msg_fatal("fringe", "baseline must be passed as 2-char code."<< eom);
            std::exit(1);
        }
    }
    else
    {
        //split on ':' into baseline and frequency group
        std::vector< std::string> tokens;
        std::string delim = ":";
        tokenizer.SetDelimiter(delim);
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.SetString(&baseline_freqgrp);
        tokenizer.GetTokens(&tokens);
        if(tokens.size() != 2)
        {
            msg_fatal("fringe", "could not parse '-b' command line argument: "<<baseline_freqgrp<< eom);
            std::exit(1);
        }
        baseline = tokens[0];
        freqgrp = tokens[1];
    }
}

std::string
MHO_BasicFringeDataConfiguration::parse_set_string(const std::vector< std::string >& arglist, int& set_arg_index)
{
    set_arg_index = -1;
    int nargs = arglist.size();
    for(int i=0; i<nargs; i++)
    {
        if(arglist[i] == "set")
        {
            set_arg_index = i;
            break;
        }
    }

    //if we've found a 'set' command, assume everything after this is control
    //file syntax and concatenate everything together with spaces
    std::string set_string = "";
    int start_idx = set_arg_index+1;
    if(set_arg_index != -1 && start_idx < nargs-1 )
    {
        for(int i=start_idx; i<nargs; i++)
        {
            set_string += arglist[i];
            set_string += " ";
        }
    }

    return set_string;
}

std::string
MHO_BasicFringeDataConfiguration::sanitize_directory(std::string dir)
{
    //first lets determine if this actually a directory (or if is a file)
    bool is_dir = MHO_DirectoryInterface::IsDirectory(dir);
    std::string path = dir;
    if(is_dir)
    {
        //this is really a directory, so just make sure it ends with a '/'
        if( path.back() != '/' ){path += "/";}
    }
    else
    {
        //this is actually a file (probably the root file)
        //so strip the file and return the directory it is in
        std::size_t dir_end = dir.find_last_of("/");
        if(dir_end != std::string::npos)
        {
            path = dir.substr(0,dir_end) + "/";
        }
    }

    msg_debug("fringe", "sanitized directory path: " << path << eom );

    //check that this directory exists
    bool ok;
    std::string fullpath = MHO_DirectoryInterface::GetDirectoryFullPath(path);
    ok = MHO_DirectoryInterface::DoesDirectoryExist(fullpath);
    if(!ok)
    {
        msg_error("fringe", "could not find the directory: "<< fullpath << eom );
        std::exit(1);
    }
    return path; //we do not return the full-path in order to preserve symlinks
}

std::string
MHO_BasicFringeDataConfiguration::find_associated_root_file(std::string dir)
{
    std::string path = sanitize_directory(dir);
    MHO_DirectoryInterface dirInter;

    dirInter.SetCurrentDirectory(path);
    dirInter.ReadCurrentDirectory();
    std::vector< std::string > flist;
    std::string ext(".root.json");
    dirInter.GetFilesMatchingExtention(flist, ext);

    std::string root_file = "";
    if(flist.size() != 1)
    {
        if(flist.size() == 0)
        {
            // msg_warn("fringe", "no root file found in: "<< dir << eom );
            return root_file; //empty root file (nothing here)
        }
        if(flist.size() > 1 )
        {
            msg_warn("fringe", "multiple root files found in: "<< dir <<" using the first one found: "<< flist[0] << eom );
        }
    }

    root_file = flist[0];
    root_file = MHO_DirectoryInterface::GetDirectoryFullPath(root_file);
    return root_file;

}



//sanity check of parameters after command line parsing
int
MHO_BasicFringeDataConfiguration::sanity_check(MHO_ParameterStore* paramStore)
{
    //command line parameters
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline");
    std::string freqgrp = paramStore->GetAs<std::string>("/cmdline/frequency_group");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string directory = paramStore->GetAs<std::string>("/cmdline/directory");
    //bool estimate_time = false; //'-e' estimate run time
    int first_plot_chan = paramStore->GetAs<int>("/cmdline/first_plot_channel");
    int message_level = paramStore->GetAs<int>("/cmdline/message_level");
    int nplot_chans = paramStore->GetAs<int>("/cmdline/nplot_channels");
    bool show_plot = paramStore->GetAs<bool>("/cmdline/show_plot");
    //std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = paramStore->GetAs<int>("/cmdline/ap_per_seg");
    bool test_mode = paramStore->GetAs<bool>("/cmdline/test_mode");
    //bool update_mode = false; //'-u' not yet enabled
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    //std::string reftime = "";
    //bool xpower_output = false;
    //std::string output_file = paramStore->GetAs<std::string>("/cmdline/output_file");

    TODO_FIXME_MSG("TODO FIXME - fill out the sanity_check function for command line arguments")
    if( directory == "" || control_file == "")
    {
        return 1;
    }

    return 0;
}





int MHO_BasicFringeDataConfiguration::parse_fourfit_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
{
    //store the raw arguments in the parameter store
    std::vector<std::string> arglist;
    for(int i=0; i<argc; i++){arglist.push_back( std::string(argv[i]) );}
    paramStore->Set("/cmdline/args", arglist);

    //command line parameters
    bool accounting = false; //'-a' perform run-time accounting/profiling
    std::string baseline_opt = "??:?"; //'-b' baseline:frequency_group selection
    std::string baseline = "??"; // the baseline
    std::string freqgrp = "?"; // the frequency group
    std::string control_file = ""; //'-c' specifies the control file
    bool exclude_autos = false; //'-e' estimate run time
    int first_plot_chan = 0; //'-n' specifies the first channel displayed in the fringe plot
    int message_level = -1; //'-m' specifies the message verbosity level
    std::vector< std::string > message_categories;  // -'M' limits the allowed message categories to those the user specifies
    int nplot_chans = 0; //'-n' specifies the number of channels to display in the fringe plot
    bool show_plot = false; //'-p' generates and shows fringe plot
    std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = 0; //'-s' specify the APs to be averaged per plot-segment
    bool test_mode = false; //'-t' if true, then no output is written
    bool update_mode = false; //'-u' not yet enabled
    std::string polprod = "??"; //'-P' polarization product argument (e.g XX or I or RR+LL)
    std::string reftime = ""; //'-T' specify the fourfit reference time - not yet enabled
    //bool xwindows; //'-x' same as option '-p' we no long use pgplot/xwindows
    bool xpower_output = false; //-X export xpower spectrum
    bool use_mk4_output = false;
    std::string input;

    std::vector< std::string > msg_cats =
    {
        "main", "calibration", "containers", "control",
        "fringe", "file", "initialization", "mk4interface",
        "utilities", "vex", "python_bindings"
    };

    std::stringstream ss;
    ss << "limit the allowed message categories to only those which the user specifies, the available categories are: \n";
    for(auto it = msg_cats.begin(); it != msg_cats.end(); it++)
    {
        ss << "    "<< *it <<"\n";
    }
    ss <<"if the '-M' option is not used, the default is to allow all categories. ";
    std::string msg_cat_help = ss.str();

    CLI::App app{"fourfit"};

    // Remove help flag because it shortcuts all processing
    app.set_help_flag();

    // Add custom flag that activates help
    auto *help = app.add_flag("-h,--help", "print this help message and exit");
    app.add_flag("-a,--accounting", accounting, "perform run-time accounting/profiling");
    app.add_option("-b,--baseline", baseline_opt, "baseline or baseline:frequency_group selection (e.g GE or GE:X)");
    app.add_option("-c,--control-file", control_file, "specify the control file");
    app.add_flag("-e,--exclude-autocorrs", exclude_autos, "exclude auto-correlations from fringe-fitting");
    app.add_option("-f,--first-plot-channel", first_plot_chan, "specifies the first channel displayed in the fringe plot (ignored, not yet implemented)");
    app.add_option("-M,--message-categories", message_categories, msg_cat_help.c_str() )->delimiter(',');
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-n,--nplot-channels", nplot_chans, "specifies the number of channels to display in the fringe plot (ignored, not yet implemented)");
    app.add_flag("-p,--plot", show_plot, "generate and shows fringe plot on completion");
    app.add_option("-r,--refringe-alist", refringe_alist_file, "alist file for refringing (ignored, not yet implemented)");
    app.add_option("-s,--ap-per-segment", ap_per_seg, "specify the APs to be averaged per plot-segment");
    app.add_flag("-t,--test-mode", test_mode, "if passed, then no output is written");
    app.add_flag("-u,--update-mode", update_mode, "(ignored, not yet implemented)");
    app.add_option("-P,--polprod", polprod, "polarization product argument (e.g XX or I or RR+LL, etc.)");
    app.add_option("-T,--reftime", reftime, "specify the fourfit reference time (ignored, not yet implemented)");
    //app.add_flag("-x,--xwindows", xwindows, "display plot using xwindows (ignored, deprecated)");
    app.add_flag("-X,--xpower-output", xpower_output, "output spectral cross power data (visibilities with corrections/residual fringe solution applied)");
    app.add_option("input,-i,--input", input, "name of the input directory (scan) or root file")->required();
    app.add_flag("-k,--mark4-output", use_mk4_output, "write output files in mark4 type_2xx format");

    //add the 'set' command for control file parameter overrides
    auto *setcom = app.add_subcommand("set", "pass control file parameters and related syntax on the command line")->prefix_command();

    try
    {
        app.parse(argc, argv);
        if(*help)
        {
            throw CLI::CallForHelp();
        }
    }
    catch(const CLI::Error &e)
    {
        std::cout << app.help() << std::endl;
        std::exit(1); //just exit don't bother returning to main
    }

    //grab the set string if present
    std::string set_string = " ";
    std::vector< std::string > set_tokens;
    for(const auto &aarg : setcom->remaining())
    {
        set_tokens.push_back(aarg);
        set_string += aarg + " ";
    }

    //clamp message level
    if(message_level > 5){message_level = 5;}
    if(message_level < -2){message_level = -2;}
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //check if any message categories were passed, if so, we limit the messages
    //to only those categories
    if(message_categories.size() != 0)
    {
        for(std::size_t m=0; m<message_categories.size(); m++)
        {
            MHO_Message::GetInstance().AddKey(message_categories[m]);
        }
        MHO_Message::GetInstance().LimitToKeySet();
    }

    //enable profiling if passed '-a'
    if(accounting){ MHO_Profiler::GetInstance().Enable();}

    //catch no input case
    if(input == ""){msg_fatal("main", "input directory/root file not set" << eom); std::exit(1);}

    //catch no control file case and set to /dev/null
    //TODO detect DEF_CONTROL environmental variable if present and use that
    if(control_file == ""){control_file = "/dev/null";}

    //for now we require these options to be set (may relax this once we allow mult-pass fringe fitting)
    MHO_BasicFringeDataConfiguration::parse_baseline_freqgrp(baseline_opt, baseline, freqgrp);

    //set the message level
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //clean the directory string
    std::string directory = MHO_BasicFringeDataConfiguration::sanitize_directory(input);
    //if there is no root file (i.e. we were passed and experiment directory, we return an empty string)
    std::string root_file = MHO_BasicFringeDataConfiguration::find_associated_root_file(input);

    //pass the extracted command line info back in the parameter store
    paramStore->Set("/cmdline/accounting", accounting);
    paramStore->Set("/cmdline/baseline", baseline);
    paramStore->Set("/cmdline/frequency_group", freqgrp);

    paramStore->Set("/cmdline/control_file",control_file);
    paramStore->Set("/cmdline/directory", directory); //sanitized directory path
    paramStore->Set("/cmdline/root_file", root_file); //fully resolved (symlink free path to the root file)...or empty

    paramStore->Set("/cmdline/exclude_autos", exclude_autos);
    paramStore->Set("/cmdline/first_plot_channel", first_plot_chan); //TODO
    paramStore->Set("/cmdline/message_level", message_level);
    paramStore->Set("/cmdline/nplot_channels", nplot_chans); //TODO
    paramStore->Set("/cmdline/show_plot", show_plot); //TODO
    //refringe_alist_file = ""; //not implemented
    paramStore->Set("/cmdline/ap_per_seg",ap_per_seg);
    paramStore->Set("/cmdline/test_mode", test_mode); //TODO
    //update_mode = false; //not implemented
    paramStore->Set("/cmdline/polprod", polprod);
    //reftime = ""; //not implemented
    paramStore->Set("/cmdline/xpower_output", xpower_output);
    paramStore->Set("/cmdline/set_string", set_string); //TODO
    paramStore->Set("/cmdline/mk4format_output", use_mk4_output);

    int status = MHO_BasicFringeDataConfiguration::sanity_check(paramStore);

    return status; //0 is ok, anything else is an error
}



void
MHO_BasicFringeDataConfiguration::determine_scans(const std::string& initial_dir, std::vector< std::string >& scans, std::vector< std::string >& roots)
{
    scans.clear();
    roots.clear();
    //if we are only doing a single directory we will have a root file right away
    std::string root = find_associated_root_file(initial_dir);
    if(root == "")
    {
        //no root file was located, so we might be running over a whole experiment directory
        //we need to loop over all the sub-dirs and determine if they are scans directories
        MHO_DirectoryInterface dirInterface;
        dirInterface.SetCurrentDirectory(initial_dir);
        dirInterface.ReadCurrentDirectory();
        std::vector< std::string > subdirs;
        dirInterface.GetSubDirectoryList(subdirs);

        //loop over all these directories and check if they have a root file
        //if they do, then add them to the list, if not, skip
        for(auto it = subdirs.begin(); it != subdirs.end(); it++)
        {
            std::string path = *it;
            dirInterface.SetCurrentDirectory(path);
            dirInterface.ReadCurrentDirectory();
            std::vector< std::string > flist;
            std::string ext(".root.json");
            dirInterface.GetFilesMatchingExtention(flist, ext);
            if(flist.size() != 0)
            {
                scans.push_back(path + "/"); //NOTE: the trailing '/' is important!
                roots.push_back(flist[0]);
            }
        }
    }
    else
    {
        //just one scan passed
        scans.push_back(initial_dir);
        roots.push_back(root);
    }
}

void
MHO_BasicFringeDataConfiguration::determine_baselines(const std::string& dir,
                                                      const std::string& baseline,
                                                      std::vector< std::pair< std::string, std::string > >& baseline_files)
{
    baseline_files.clear();
    std::vector< std::string > corFiles;
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFilesMatchingExtention(corFiles, "cor");

    //loop over 'cor' files and extract the baseline information
    //we expect that baseline visibility data to be stored in files with names of the form:
    //GE.Gs-Wf.ABCDEF.cor = <2-char baseline code>.<ref_station-rem_station>.<root code>.cor
    //this is a little redundant, but preserves some backwards compatibility with
    //single-char station mk4id's while also allowing identification with 2-char codes

    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(".");
    tokenizer.SetIncludeEmptyTokensFalse();

    for(std::size_t i=0; i<corFiles.size(); i++)
    {
        std::string bname = MHO_DirectoryInterface::GetBasename(corFiles[i]);
        tokenizer.SetString( &bname );
        std::vector< std::string > tok;
        tokenizer.GetTokens(&tok);

        //we have a traditional mk4 style 2-character baseline code
        if(baseline.size() == 2 && tok[0].size() == 2)
        {

            std::string bl = tok[0];
            bool keep = false;
            if(baseline == "??"){keep = true;}
            if(baseline[0]  == '?' && baseline[1] == bl[1]){keep = true;}
            if(baseline[1] == '?' && baseline[0] == bl[0]){keep = true;}
            if(baseline == bl){keep = true;}
            if(keep)
            {
                baseline_files.push_back( std::make_pair(bl, corFiles[i]) );
            }
        }
    }
}

void
MHO_BasicFringeDataConfiguration::determine_fgroups_polproducts(const std::string& filename,
                                    const std::string& cmd_fgroup,
                                    const std::string& cmd_pprod,
                                    std::vector< std::string >& fgroups,
                                    std::vector< std::string >& pprods )
{
    fgroups.clear();
    pprods.clear();

    //get uuid for MHO_ObjectTags object
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor<MHO_ObjectTags>();

    //pull all the keys and byte offsets for each object
    std::vector< MHO_FileKey > ikeys;
    std::vector< std::size_t > byte_offsets;
    MHO_BinaryFileInterface inter;
    inter.ExtractFileObjectKeysAndOffsets(filename, ikeys, byte_offsets);

    //loop over keys and offsets, looking for tags offset
    bool found = false;
    std::size_t offset_bytes = 0;
    for(std::size_t i=0; i<ikeys.size(); i++)
    {
        if(ikeys[i].fTypeId == tag_uuid)
        {
            offset_bytes = byte_offsets[i];
            found = true;
            break; //only first tag object is used
        }
    }

    std::vector< std::string > tmp_fgroups;
    std::vector< std::string > tmp_pprods;
    if(found)
    {
        inter.OpenToReadAtOffset(filename, offset_bytes);
        MHO_ObjectTags obj;
        MHO_FileKey obj_key;
        //we read the tags object
        //now pull the pol-products and frequency groups info
        //and check them agains the command line arguments
        bool ok = inter.Read(obj, obj_key);
        if(ok)
        {
            if( obj.IsTagPresent("polarization_product_set") )
            {
                obj.GetTagValue("polarization_product_set", tmp_pprods);
                for(std::size_t i=0; i<tmp_pprods.size(); i++)
                {
                    //only if pol-product is unspecified, will we do them all
                    //otherwise only the specified pol-product will be used
                    bool keep = false;
                    if(cmd_pprod == "??"){keep = true;}
                    if(keep)
                    {
                        pprods.push_back(tmp_pprods[i]);
                    }
                }
            }
            else
            {
                msg_error("fringe", "no polarization_product_set present in MHO_ObjectTags" << eom);
            }

            if( obj.IsTagPresent("frequency_band_set") )
            {
                obj.GetTagValue("frequency_band_set", tmp_fgroups);
                for(std::size_t i=0; i<tmp_fgroups.size(); i++)
                {
                    //if fgroup is unspecified we do them all
                    //otherwise we only keep the one selected
                    bool keep = false;
                    if(cmd_fgroup == "?"){keep = true;}
                    if(cmd_fgroup == tmp_fgroups[i]){keep = true;}
                    if(keep)
                    {
                        fgroups.push_back(tmp_fgroups[i]);
                    }
                }
            }
            else
            {
                msg_error("fringe", "no frequency_band_set present in MHO_ObjectTags" << eom);
            }
        }
        else
        {
            msg_error("fringe", "could not determine polarization products or frequency bands from: "<< filename << eom);
        }
        inter.Close();
    }
    else
    {
        msg_error("fringe", "no MHO_ObjectTags object found in file: "<< filename << eom);
    }

    //if a pol-product was specified (or a linear combination was specified)
    //make sure it gets through here
    if(cmd_pprod != "??")
    {
        pprods.clear();
        pprods.push_back(cmd_pprod);
    }
}

void
MHO_BasicFringeDataConfiguration::determine_passes(MHO_ParameterStore* cmdline_params,
                                                   std::string& cscans,
                                                   std::string& croots,
                                                   std::string& cbaselines,
                                                   std::string& cfgroups,
                                                   std::string& cpolprods)
{
    //pass search order is: scans, then baselines, then fgroups, then pol-products
    std::vector< std::string > scans; //list of scan directories
    std::vector< std::string > roots; //list of associated root files
    std::vector< std::pair< std::string, std::string > > baseline_files;
    std::vector< std::string > fgroups;
    std::vector< std::string > polproducts;

    //flattened pass parameters (for return values)
    std::string concat_delim = ",";
    cscans = "";
    croots = "";
    cbaselines = "";
    cfgroups = "";
    cpolprods = "";

    //determine which directories contain scans to process
    std::string initial_dir = cmdline_params->GetAs<std::string>("/cmdline/directory");
    std::string cmd_bl = cmdline_params->GetAs<std::string>("/cmdline/baseline"); //if not passed, will be "??"
    std::string cmd_fg = cmdline_params->GetAs<std::string>("/cmdline/frequency_group"); //if not passed, this will be "?"
    std::string cmd_pp = cmdline_params->GetAs<std::string>("/cmdline/polprod"); //if not passed, this will be "??"
    bool exclude_autos = cmdline_params->GetAs<bool>("/cmdline/exclude_autos");

    determine_scans(initial_dir, scans, roots);

    //form all the data passes that must be processed
    for(std::size_t sc = 0; sc < scans.size(); sc++)
    {
        std::string scan_dir = scans[sc];
        std::string root_file = roots[sc];
        if(root_file != "")
        {
            determine_baselines(scan_dir, cmd_bl, baseline_files);
            for(auto bl = baseline_files.begin(); bl != baseline_files.end(); bl++)
            {
                std::string baseline = bl->first;
                if( !(exclude_autos && baseline[0] == baseline[1]) )
                {
                    std::string corFile = bl->second;
                    determine_fgroups_polproducts(corFile, cmd_fg, cmd_pp, fgroups, polproducts);
                    for(auto fg = fgroups.begin(); fg != fgroups.end(); fg++)
                    {
                        std::string fgroup = *fg;
                        for(auto pprod = polproducts.begin(); pprod != polproducts.end(); pprod++)
                        {
                            std::string polprod = *pprod;
                            //repeatedly append info for every work item
                            //TODO FIXME we may want to rethink this
                            cscans += scan_dir + concat_delim;
                            croots  += root_file + concat_delim;
                            cbaselines += baseline + concat_delim;
                            cpolprods += polprod + concat_delim;
                            cfgroups += fgroup + concat_delim;

                        } //end of pol-product loop
                    } //end of frequency group loop
                } //autocorr if-exclude
            }//end of baseline loop
        } //end only if root exists
    } //end of scan loop
}

void MHO_BasicFringeDataConfiguration::split_passes(std::vector<mho_json>& pass_vector,
                                                    const std::string& cscans,
                                                    const std::string& croots,
                                                    const std::string& cbaselines,
                                                    const std::string& cfgroups,
                                                    const std::string& cpolprods)
{
    std::vector< std::string > sdirs;
    std::vector< std::string > rts;
    std::vector< std::string > blines;
    std::vector< std::string > fgrps;
    std::vector< std::string > ppds;

    MHO_Tokenizer tokenizer;
    std::string concat_delim = ",";
    tokenizer.SetDelimiter(concat_delim);
    tokenizer.SetIncludeEmptyTokensFalse();
    tokenizer.SetString(&cscans);
    tokenizer.GetTokens(&sdirs);
    tokenizer.SetString(&croots);
    tokenizer.GetTokens(&rts);
    tokenizer.SetString(&cbaselines);
    tokenizer.GetTokens(&blines);
    tokenizer.SetString(&cfgroups);
    tokenizer.GetTokens(&fgrps);
    tokenizer.SetString(&cpolprods);
    tokenizer.GetTokens(&ppds);

    //TODO FIXME...all these vectors better have the same size!
    std::size_t npass = sdirs.size();
    std::size_t nroots = rts.size();
    std::size_t nblines = blines.size();
    std::size_t nfgrps = fgrps.size();
    std::size_t nppds = ppds.size();

    bool ok = true;
    if(npass != nroots){ok = false;}
    if(npass != nblines){ok = false;}
    if(npass != nfgrps){ok = false;}
    if(npass != nppds){ok = false;}

    if(!ok)
    {
        msg_fatal("fringe", "error forming data passes, " <<
                  "mismatched number of scans/roots/baselines/fgroups/polproducts - " <<
                  npass<<"/"<<nroots<<"/"<<nblines<<"/"<<nfgrps<<"/"<<nppds<<eom);
        std::exit(1);
    }

    pass_vector.clear();
    for(std::size_t i=0; i<npass; i++)
    {
        mho_json pass;
        pass["directory"] = sdirs[i];
        pass["root_file"] = rts[i];
        pass["baseline"] = blines[i];
        pass["polprod"] = ppds[i];
        pass["frequency_group"] = fgrps[i];
        pass_vector.push_back(pass);
    }
}


bool MHO_BasicFringeDataConfiguration::initialize_scan_data(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    //this should all be present and ok at this point
    std::string directory = paramStore->GetAs<std::string>("/pass/directory");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(directory);
    scanStore->Initialize();
    if( !scanStore->IsValid() )
    {
        msg_error("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        return false;
    }

    //set the root file name
    paramStore->Set("/files/root_file", scanStore->GetRootFileBasename() );

    msg_debug("fringe", "loading root file: "<< scanStore->GetRootFileBasename() << eom);

    //load root file and extract useful vex info into parameter store
    auto vexInfo = scanStore->GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(vexInfo, paramStore);

    std::string bl = paramStore->GetAs<std::string>("/pass/baseline");
    std::string pp = paramStore->GetAs<std::string>("/pass/polprod");
    std::string fg = paramStore->GetAs<std::string>("/pass/frequency_group");
    msg_info("fringe", "fringing data for baseline: "<<bl<<", pol-product: "<<pp<<", freq-group: "<<fg<<", in directory: "<<directory<< eom);

    return true;
}



void MHO_BasicFringeDataConfiguration::populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    //initialize by setting "is_finished" to false, and 'skipped' to false
    //these parameters must always be present
    paramStore->Set("/status/is_finished", false);
    paramStore->Set("/status/skipped", false);

    //these should all be present and ok at this point
    std::string directory = paramStore->GetAs<std::string>("/pass/directory");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string baseline = paramStore->GetAs<std::string>("/pass/baseline");
    std::string polprod = paramStore->GetAs<std::string>("/pass/polprod");
    std::string fgroup = paramStore->GetAs<std::string>("/pass/frequency_group");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE PARAMETERS
    ////////////////////////////////////////////////////////////////////////////


    //set up the file section of the parameter store to record the directory, root file, and control file
    paramStore->Set("/files/control_file", control_file);
    paramStore->Set("/files/directory", directory);
    //paramStore->Set("/files/output_file", paramStore->GetAs<std::string>("/cmdline/output_file"));

    
    //set the software version info
    paramStore->Set("/config/software_version", std::string(HOPS_VERSION) + " " + std::string(HOPS_GIT_REV) );

    //put the baseline and pol product selection into the parameter store
    paramStore->Set("/config/polprod", polprod);
    paramStore->Set("/config/baseline", baseline);
    paramStore->Set("/config/fgroup", fgroup);

    //parse the polprod string in order to determine which pol-products are needed (if more than one)
    std::vector< std::string > pp_vec = determine_required_pol_products(polprod);
    paramStore->Set("/config/polprod_set", pp_vec);

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(directory);
    scanStore->Initialize();
    if( !scanStore->IsValid() )
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    if( !scanStore->IsBaselinePresent(baseline) )
    {
        msg_fatal("fringe", "cannot find the specified baseline: " << baseline << " in " << directory << eom);
        std::exit(1);
    }

    //set the root file name
    paramStore->Set("/files/root_file", scanStore->GetRootFileBasename() );

    msg_debug("fringe", "loading root file: "<< scanStore->GetRootFileBasename() << eom);

    //load root file and extract useful vex info into parameter store
    auto vexInfo = scanStore->GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(vexInfo, paramStore);

}

//more helper functions
void
MHO_BasicFringeDataConfiguration::configure_visibility_data(MHO_ContainerStore* store)
{
    //first check if there are visibility_type and weight_type with double precision present
    std::size_t n_vis = store->GetNObjects<visibility_type>();
    std::size_t n_wt = store->GetNObjects<weight_type>();

    if(n_vis == 1 && n_wt == 1)
    {
        msg_debug("initialization", "double precision visibility and weight types found, these will be preferred and used over single precision types." << eom);
        return;
    }

    //evidently there are no double precision objects, so we look for the single-precision 'storage types'
    n_vis = store->GetNObjects<visibility_store_type>();
    n_wt = store->GetNObjects<weight_store_type>();

    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject<visibility_store_type>(0);
    wt_store_data = store->GetObject<weight_store_type>(0);

    if(vis_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("initialization", "multiple visibility and/or weight types per-baseline not yet supported, will use first located." << eom);
    }

    auto vis_store_uuid = vis_store_data->GetObjectUUID();
    auto wt_store_uuid = wt_store_data->GetObjectUUID();

    std::string vis_shortname = store->GetShortName(vis_store_uuid);
    std::string wt_shortname = store->GetShortName(wt_store_uuid);

    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();

    //assign the storage UUID's to their up-casted counter-parts
    //we do this so we can associate them to the file objects (w.r.t to program output, error messages, etc.)
    vis_data->SetObjectUUID(vis_store_uuid);
    wt_data->SetObjectUUID(wt_store_uuid);

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects to save space
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    //warn on non-standard shortnames
    if(vis_shortname != "vis"){msg_warn("initialization", "visibilities do not use canonical short name 'vis', but are called: "<< vis_shortname << eom);}
    if(wt_shortname != "weight"){msg_warn("initialization", "weights do not use canonical short name 'weight', but are called: "<< wt_shortname << eom);}

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}

void
MHO_BasicFringeDataConfiguration::configure_station_data(MHO_ScanDataStore* scanStore, MHO_ContainerStore* containerStore,
                                                         std::string ref_station_mk4id, std::string rem_station_mk4id)
{
    //load station data and assign them the names 'ref_sta' or 'rem_sta'
    scanStore->LoadStation(ref_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "ref_sta");
    MHO_UUID pcal_uuid;
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if( !(pcal_uuid.is_empty()) )
    {
        containerStore->RenameObject("pcal", "ref_pcal");
    }

    scanStore->LoadStation(rem_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "rem_sta");
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if( !(pcal_uuid.is_empty()) )
    {
        containerStore->RenameObject("pcal", "rem_pcal");
    }
}

void
MHO_BasicFringeDataConfiguration::init_and_exec_operators(MHO_OperatorBuilderManager* build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    if(build_manager == nullptr || opToolbox == nullptr)
    {
        msg_error("fringe", "cannot initialize or execute operators if builder or toolbox is missing" << eom );
        return;
    }

    msg_debug("fringe", "initializing and executing operators in "<<cat<<" category."<<eom);

    build_manager->BuildOperatorCategory(cat);
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
    {
        msg_debug("fringe", "initializing and executing operator: "<< (*opIt)->GetName() <<", with priority: "<< (*opIt)->Priority() << "." << eom);
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }
}

std::vector< std::string >
MHO_BasicFringeDataConfiguration::determine_required_pol_products(std::string polprod)
{
    MHO_Tokenizer tokenizer;
    std::set<std::string> pp_set;
    std::vector<std::string> pp_vec;
    //first we parse the polprod string to see what individual pol-products we need
    if( polprod.find("+") != std::string::npos)
    {
        //we have a pol-product summation like (RR+LL) or XX+YY, or RX+RY
        //so split on all '+' symbols (currently we only support '+' not '-')
        tokenizer.SetDelimiter("+");
        tokenizer.SetString(&polprod);
        //fTokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.GetTokens(&pp_vec);
    }
    else if(polprod == "I") //special pseudo-Stokes-I mode (linear pol only)
    {
        pp_vec.push_back("XX");
        pp_vec.push_back("YY");
        pp_vec.push_back("XY");
        pp_vec.push_back("YX");
    }
    else
    {
        pp_vec.push_back(polprod); //polprod is just a single value
    }

    //push the values into a set, so we don't have any duplicates
    pp_set.insert( pp_vec.begin(), pp_vec.end() );

    //push the set values into the vector for return
    pp_vec.clear();
    pp_vec.insert(pp_vec.begin(), pp_set.begin(), pp_set.end() );

    std::stringstream ss;
    for(std::size_t i=0; i<pp_vec.size(); i++)
    {
        ss << pp_vec[i];
        if(i != pp_vec.size() - 1){ss <<", "; }
    }
    msg_debug("fringe", "required pol-products are: {" << ss.str() << "}." << eom );

    return pp_vec;
}


mho_json
MHO_BasicFringeDataConfiguration::ConvertProfileEvents(std::vector< MHO_ProfileEvent >& events)
{
    mho_json event_list;
    for(std::size_t i=0; i<events.size(); i++)
    {
        mho_json obj;
        obj["event_id"] = i;
        obj["flag"] = events[i].fFlag;
        obj["line"] = events[i].fLineNumber;
        obj["thread_id"] = events[i].fThreadID;
        obj["filename"] = std::string( events[i].fFilename );
        obj["funcname"] = std::string( events[i].fFuncname );
        obj["time"] = events[i].fTime;
        event_list.push_back(obj);
    }
    return event_list;
}


}//end namespace
