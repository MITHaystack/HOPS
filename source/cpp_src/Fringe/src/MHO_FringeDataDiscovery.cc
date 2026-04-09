#include "MHO_FringeDataDiscovery.hh"

#include "MHO_FringeCommandLineParser.hh"

namespace hops
{

std::string MHO_FringeDataDiscovery::find_associated_root_file(std::string dir)
{
    std::string path = MHO_FringeCommandLineParser::sanitize_directory(dir);
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
        if(flist.size() > 1)
        {
            msg_warn("fringe", "multiple root files found in: " << dir << " using the first one found: " << flist[0] << eom);
        }
    }

    root_file = flist[0];
    root_file = MHO_DirectoryInterface::GetDirectoryFullPath(root_file);
    return root_file;
}


void MHO_FringeDataDiscovery::determine_scans(const std::string& initial_dir, std::vector< std::string >& scans,
                                                       std::vector< std::string >& roots)
{
    scans.clear();
    roots.clear();
    //if we are only doing a single directory we will have a root file right away
    std::string root = find_associated_root_file(initial_dir);
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(initial_dir);
    dirInterface.ReadCurrentDirectory();
    if(root == "")
    {
        //no root file was located, so we might be running over a whole experiment directory
        //we need to loop over all the sub-dirs and determine if they are scans directories
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

    if(roots.size() == 0)
    {
        //no root files detected, check for the possibility that the user erroneously pointed us to a mark4 scan directory.
        std::vector< std::string > all_files;
        std::string legacy_root_file = "";
        dirInterface.GetFileList(all_files);
        dirInterface.GetRootFile(all_files, legacy_root_file);
        if(legacy_root_file != "")
        {
            msg_warn("fringe", "no hops4 data (.cor) files found, but legacy mark4 root file ("
                                   << dirInterface.GetBasename(legacy_root_file)
                                   << ") detected, either pass '-K' option, or run mark42hops/difx2hops" << eom);
        }
    }
}

void MHO_FringeDataDiscovery::determine_baselines(const std::string& dir, const std::string& baseline,
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

    for(std::size_t i = 0; i < corFiles.size(); i++)
    {
        std::string bname = MHO_DirectoryInterface::GetBasename(corFiles[i]);
        tokenizer.SetString(&bname);
        std::vector< std::string > tok;
        tokenizer.GetTokens(&tok);
        bool keep = false;
        std::string bl;
        //we have a traditional mk4 style 2-character baseline code
        if(baseline.size() == 2 && tok[0].size() == 2)
        {
            bl = tok[0];
            if(baseline == "??")
            {
                keep = true;
            }
            if(baseline[0] == '?' && baseline[1] == bl[1])
            {
                keep = true;
            }
            if(baseline[1] == '?' && baseline[0] == bl[0])
            {
                keep = true;
            }
            if(baseline == bl)
            {
                keep = true;
            }
        }
        else if((baseline.size() == 5 && tok[1].size() == 5) && (baseline.find('-') != std::string::npos) &&
                (tok[1].find('-') != std::string::npos))
        {
            bl = tok[1]; //check the extended baseline-identifier (using 2 char station codes)
            if(baseline == bl)
            {
                bl = tok[0]; //set the baseline parameter to the mk4-style 2-char form
                keep = true;
            }
        }

        if(keep)
        {
            baseline_files.push_back(std::make_pair(bl, corFiles[i]));
        }
    }
}

void MHO_FringeDataDiscovery::determine_fgroups_polproducts(const std::string& filename, const std::string& cmd_fgroup,
                                                                     const std::string& cmd_pprod,
                                                                     std::vector< std::string >& fgroups,
                                                                     std::vector< std::string >& pprods)
{
    fgroups.clear();
    pprods.clear();

    //get uuid for MHO_ObjectTags object
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor< MHO_ObjectTags >();

    //pull all the keys and byte offsets for each object
    std::vector< MHO_FileKey > ikeys;
    std::vector< std::size_t > byte_offsets;
    MHO_BinaryFileInterface inter;
    inter.ExtractFileObjectKeysAndOffsets(filename, ikeys, byte_offsets);

    //loop over keys and offsets, looking for tags offset
    bool found = false;
    std::size_t offset_bytes = 0;
    for(std::size_t i = 0; i < ikeys.size(); i++)
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
            if(obj.IsTagPresent("polarization_product_set"))
            {
                obj.GetTagValue("polarization_product_set", tmp_pprods);
                for(std::size_t i = 0; i < tmp_pprods.size(); i++)
                {
                    //only if pol-product is unspecified, will we do them all
                    //otherwise only the specified pol-product will be used
                    bool keep = false;
                    if(cmd_pprod == "??")
                    {
                        keep = true;
                    }
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

            if(obj.IsTagPresent("frequency_band_set"))
            {
                obj.GetTagValue("frequency_band_set", tmp_fgroups);
                for(std::size_t i = 0; i < tmp_fgroups.size(); i++)
                {
                    //if fgroup is unspecified we do them all
                    //otherwise we only keep the one selected
                    bool keep = false;
                    if(cmd_fgroup == "?")
                    {
                        keep = true;
                    }
                    if(cmd_fgroup == tmp_fgroups[i])
                    {
                        keep = true;
                    }
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
            msg_error("fringe", "could not determine polarization products or frequency bands from: " << filename << eom);
        }
        inter.Close();
    }
    else
    {
        msg_error("fringe", "no MHO_ObjectTags object found in file: " << filename << eom);
    }

    //if a pol-product was specified (or a linear combination was specified)
    //make sure it gets through here
    if(cmd_pprod != "??")
    {
        pprods.clear();
        pprods.push_back(cmd_pprod);
    }
}

void MHO_FringeDataDiscovery::determine_passes(MHO_ParameterStore* cmdline_params, std::string& cscans,
                                                        std::string& croots, std::string& cbaselines, std::string& cfgroups,
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
    std::string initial_dir = cmdline_params->GetAs< std::string >("/cmdline/input_directory");
    std::string cmd_bl = cmdline_params->GetAs< std::string >("/cmdline/baseline");        //if not passed, will be "??"
    std::string cmd_fg = cmdline_params->GetAs< std::string >("/cmdline/frequency_group"); //if not passed, this will be "?"
    std::string cmd_pp = cmdline_params->GetAs< std::string >("/cmdline/polprod");         //if not passed, this will be "??"
    bool exclude_autos = cmdline_params->GetAs< bool >("/cmdline/exclude_autos");

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
                if(!(exclude_autos && baseline[0] == baseline[1]))
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
                            croots += root_file + concat_delim;
                            cbaselines += baseline + concat_delim;
                            cpolprods += polprod + concat_delim;
                            cfgroups += fgroup + concat_delim;

                        } //end of pol-product loop
                    }     //end of frequency group loop
                }         //autocorr if-exclude
            }             //end of baseline loop
        }                 //end only if root exists
    }                     //end of scan loop
}

void MHO_FringeDataDiscovery::split_passes(std::vector< mho_json >& pass_vector, const std::string& cscans,
                                                    const std::string& croots, const std::string& cbaselines,
                                                    const std::string& cfgroups, const std::string& cpolprods)
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
    if(npass != nroots)
    {
        ok = false;
    }
    if(npass != nblines)
    {
        ok = false;
    }
    if(npass != nfgrps)
    {
        ok = false;
    }
    if(npass != nppds)
    {
        ok = false;
    }

    if(!ok)
    {
        msg_fatal("fringe", "error forming data passes, "
                                << "mismatched number of scans/roots/baselines/fgroups/polproducts - " << npass << "/" << nroots
                                << "/" << nblines << "/" << nfgrps << "/" << nppds << eom);
        std::exit(1);
    }

    pass_vector.clear();
    for(std::size_t i = 0; i < npass; i++)
    {
        mho_json pass;
        std::string scan = MHO_DirectoryInterface::GetTrailingDirectory(rts[i]);
        pass["scan"] = scan; //needed if output dir != input dir
        pass["input_directory"] = sdirs[i];
        pass["root_file"] = rts[i];
        pass["baseline"] = blines[i];
        pass["polprod"] = ppds[i];
        pass["frequency_group"] = fgrps[i];
        pass_vector.push_back(pass);
    }
}

std::vector< std::string > MHO_FringeDataDiscovery::determine_required_pol_products(std::string polprod)
{
    MHO_Tokenizer tokenizer;
    std::set< std::string > pp_set;
    std::vector< std::string > pp_vec;
    //first we parse the polprod string to see what individual pol-products we need
    if(polprod.find("+") != std::string::npos)
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
    pp_set.insert(pp_vec.begin(), pp_vec.end());

    //push the set values into the vector for return
    pp_vec.clear();
    pp_vec.insert(pp_vec.begin(), pp_set.begin(), pp_set.end());

    std::stringstream ss;
    for(std::size_t i = 0; i < pp_vec.size(); i++)
    {
        ss << pp_vec[i];
        if(i != pp_vec.size() - 1)
        {
            ss << ", ";
        }
    }
    msg_debug("fringe", "required pol-products are: {" << ss.str() << "}." << eom);

    return pp_vec;
}

}//end namespace
