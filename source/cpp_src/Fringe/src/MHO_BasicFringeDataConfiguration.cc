#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_DirectoryInterface.hh"

//needed in case the user wants to run fourfit4 on mark4 data directly
#include "MHO_MK4ScanConverter.hh"

//POSIX includes for mkdtemp and nftw (used by convert_mk4_input)
//if we were using C++17, we could alternatively use std::filesystem
#include <ftw.h>
#include <unistd.h>

//snapshot utility lib
#include "MHO_Snapshot.hh"

//configure_data_library
#include "MHO_ElementTypeCaster.hh"

#include "MHO_VexInfoExtractor.hh"

//parse_command_line
#include "MHO_Tokenizer.hh"

#include "MHO_Message.hh"

//option parsing and help text library
#include "CLI11.hpp"

namespace hops
{

std::string MHO_BasicFringeDataConfiguration::find_associated_root_file(std::string dir)
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
        if(flist.size() > 1)
        {
            msg_warn("fringe", "multiple root files found in: " << dir << " using the first one found: " << flist[0] << eom);
        }
    }

    root_file = flist[0];
    root_file = MHO_DirectoryInterface::GetDirectoryFullPath(root_file);
    return root_file;
}

// File-scope helper for nftw recursive delete (cannot be a lambda due to C linkage requirement)
static int remove_entry(const char* path, const struct stat* /*sb*/, int /*typeflag*/, struct FTW* /*ftwbuf*/)
{
    return remove(path);
}

std::string MHO_BasicFringeDataConfiguration::convert_mk4_input(MHO_ParameterStore* paramStore)
{
    //the input directory
    std::string input_dir = paramStore->GetAs< std::string >("/cmdline/input_directory");

    int dir_type = MHO_MK4ScanConverter::DetermineDirectoryType(input_dir);

    // Read the baseline filter; "??" means all baselines
    std::string baseline = paramStore->GetAs< std::string >("/cmdline/baseline");

    // Create a unique temp directory,
    // we prefer RAM-backed /dev/shm (tmpfs) to avoid disk I/O, but only for single-scans!
    // falling back to /tmp if /dev/shm is not available, or if we were passed a full experiment
    std::string temp_root;
    {
        char shm_buf[] = "/dev/shm/hops_mk4_XXXXXX";
        if(mkdtemp(shm_buf) != nullptr && dir_type == MK4_SCANDIR)
        {
            temp_root = shm_buf;
        }
        else
        {
            char tmp_buf[] = "/tmp/hops_mk4_XXXXXX";
            if(mkdtemp(tmp_buf) == nullptr)
            {
                msg_fatal("fringe", "convert_mk4_input: could not create a temporary directory" << eom);
                return "";
            }
            temp_root = tmp_buf;
        }
    }

    if(dir_type == MK4_SCANDIR)
    {
        std::string scan_name = MHO_DirectoryInterface::GetTrailingDirectory(input_dir);
        std::string temp_scan = temp_root + "/" + scan_name;
        msg_status("fringe", "converting mark4 scan directory: " << input_dir << " -> " << temp_scan
                                                                 << " (baseline filter: " << baseline << ")" << eom);
        MHO_MK4ScanConverter::ProcessScan(input_dir, temp_scan, baseline);
        msg_info("fringe", "redirecting input directory to : " << temp_scan + "/" << eom);
        paramStore->Set("/cmdline/input_directory", temp_scan + "/");
    }
    else if(dir_type == MK4_EXPDIR)
    {
        msg_warn("fringe",
                 "on-the-fly mark4-to-hops conversion of mark4 experiment directories is not recommended, use mark42hops first"
                     << eom);
        msg_status("fringe",
                   "converting mark4 experiment directory: " << input_dir << " (baseline filter: " << baseline << ")" << eom);
        MHO_DirectoryInterface dirInterface;
        dirInterface.SetCurrentDirectory(input_dir);
        dirInterface.ReadCurrentDirectory();
        std::vector< std::string > subDirs;
        dirInterface.GetSubDirectoryList(subDirs);

        for(std::size_t i = 0; i < subDirs.size(); i++)
        {
            //strip out any sub-directories that are not mark4 scan directories
            dirInterface.SetCurrentDirectory(subDirs[i]);
            std::vector< std::string > subDirFiles;
            dirInterface.ReadCurrentDirectory();
            dirInterface.GetFileList(subDirFiles);
            std::string root_file_name = "";
            dirInterface.GetRootFile(subDirFiles, root_file_name);
            if(root_file_name != "")
            {
                std::string scan_name = MHO_DirectoryInterface::GetBasename(subDirs[i]);
                // Strip trailing '/' if present
                if(!scan_name.empty() && scan_name.back() == '/')
                {
                    scan_name.erase(scan_name.size() - 1);
                }
                std::string temp_scan = temp_root + "/" + scan_name;
                msg_status("fringe", "converting mark4 scan: " << subDirs[i] << " -> " << temp_scan << eom);
                MHO_MK4ScanConverter::ProcessScan(subDirs[i], temp_scan, baseline);
            }
        }
        paramStore->Set("/cmdline/input_directory", temp_root + "/");
        msg_info("fringe", "redirecting input directory to : " << temp_root + "/" << eom);
    }
    else
    {
        msg_fatal("fringe", "convert_mk4_input: input directory is not a recognized mark4 scan or experiment directory: "
                                << input_dir << eom);
        // Clean up the empty temp dir we just created
        nftw(temp_root.c_str(), remove_entry, 64, FTW_DEPTH | FTW_PHYS);
        return "";
    }

    return temp_root;
}

void MHO_BasicFringeDataConfiguration::cleanup_mk4_temp_dir(const std::string& temp_dir)
{
    if(temp_dir.empty())
    {
        return;
    }
    msg_status("fringe", "removing mark4 input temp directory: " << temp_dir << eom);
    nftw(temp_dir.c_str(), remove_entry, 64, FTW_DEPTH | FTW_PHYS);
}

void MHO_BasicFringeDataConfiguration::determine_scans(const std::string& initial_dir, std::vector< std::string >& scans,
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

void MHO_BasicFringeDataConfiguration::determine_baselines(const std::string& dir, const std::string& baseline,
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

void MHO_BasicFringeDataConfiguration::determine_fgroups_polproducts(const std::string& filename, const std::string& cmd_fgroup,
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

void MHO_BasicFringeDataConfiguration::determine_passes(MHO_ParameterStore* cmdline_params, std::string& cscans,
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

void MHO_BasicFringeDataConfiguration::split_passes(std::vector< mho_json >& pass_vector, const std::string& cscans,
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

bool MHO_BasicFringeDataConfiguration::initialize_scan_data(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    //this should all be present and ok at this point
    std::string input_directory = paramStore->GetAs< std::string >("/pass/input_directory");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(input_directory);
    scanStore->Initialize();
    if(!scanStore->IsValid())
    {
        msg_error("fringe", "cannot initialize a valid scan store from this directory: " << input_directory << eom);
        return false;
    }

    std::string bl = paramStore->GetAs< std::string >("/pass/baseline");
    std::string pp = paramStore->GetAs< std::string >("/pass/polprod");
    std::string fg = paramStore->GetAs< std::string >("/pass/frequency_group");
    msg_info("fringe", "fringing data for baseline: " << bl << ", pol-product: " << pp << ", freq-group: " << fg
                                                      << ", in directory: " << input_directory << eom);

    return true;
}

void MHO_BasicFringeDataConfiguration::populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    //initialize by setting "is_finished" to false, and 'skipped' to false
    //these parameters must always be present
    paramStore->Set("/status/is_finished", false);
    paramStore->Set("/status/skipped", false);

    //these should all be present and ok at this point
    std::string input_directory = paramStore->GetAs< std::string >("/pass/input_directory");
    std::string control_file = paramStore->GetAs< std::string >("/cmdline/control_file");
    std::string baseline = paramStore->GetAs< std::string >("/pass/baseline");
    std::string polprod = paramStore->GetAs< std::string >("/pass/polprod");
    std::string fgroup = paramStore->GetAs< std::string >("/pass/frequency_group");

    //we will need the scan name to construct the output_directory,
    //if it is different from the input directory
    std::string scan = paramStore->GetAs< std::string >("/pass/scan");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE PARAMETERS
    ////////////////////////////////////////////////////////////////////////////

    //set up the file section of the parameter store to record the directory, root file, and control file
    paramStore->Set("/files/control_file", control_file);
    paramStore->Set("/files/input_directory", input_directory);
    std::string output_directory = paramStore->GetAs< std::string >("/cmdline/output_directory");
    paramStore->Set("/files/output_directory", output_directory);

    if(output_directory != input_directory)
    {
        //input and output directory are different
        //so we need to construct the top-level output directory if it doesn't exist
        if(!MHO_DirectoryInterface::DoesDirectoryExist(output_directory))
        {
            MHO_DirectoryInterface::CreateDirectory(output_directory);
        }

        //now check if the output directory incorporates the scan name,
        //if not, (it is being treated as an experiment directory)
        //we will need to construct a more specific output directory
        std::string trailing_directory = MHO_DirectoryInterface::GetTrailingDirectory(output_directory);
        if(trailing_directory != scan)
        {
            //now we have to form the scan-specific output_directory with the scan name prefix
            std::string pass_output_directory = output_directory + "/" + scan + "/";
            paramStore->Set("/files/output_directory", pass_output_directory);
        }
    }

    //set the software version info
    paramStore->Set("/config/software_version", std::string(HOPS_VERSION) + "-" + std::string(HOPS_GIT_REV));

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
    scanStore->SetDirectory(input_directory);
    scanStore->Initialize();
    if(!scanStore->IsValid())
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << input_directory << eom);
        paramStore->Set("/status/skipped", true);
    }

    if(!scanStore->IsBaselinePresent(baseline))
    {
        msg_fatal("fringe", "cannot find the specified baseline: " << baseline << " in " << input_directory << eom);
        paramStore->Set("/status/skipped", true);
    }

    //set the root file name
    paramStore->Set("/files/root_file", scanStore->GetRootFileBasename());

    msg_debug("fringe", "loading root file: " << scanStore->GetRootFileBasename() << eom);

    //load root file and extract useful vex info into parameter store
    auto vexInfo = scanStore->GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(vexInfo, paramStore);

    //make sure we construct our global map of station identities (mk4id <-> 2 char code <-> station name)
    MHO_VexInfoExtractor::extract_station_identities(vexInfo);
}

//more helper functions
void MHO_BasicFringeDataConfiguration::configure_visibility_data(MHO_ContainerStore* store)
{
    //first check if there are visibility_type and weight_type with double precision present
    std::size_t n_vis = store->GetNObjects< visibility_type >();
    std::size_t n_wt = store->GetNObjects< weight_type >();

    if(n_vis == 1 && n_wt == 1)
    {
        msg_debug(
            "initialization",
            "double precision visibility and weight types found, these will be preferred and used over single precision types."
                << eom);
        return;
    }

    //evidently there are no double precision objects, so we look for the single-precision 'storage types'
    n_vis = store->GetNObjects< visibility_store_type >();
    n_wt = store->GetNObjects< weight_store_type >();

    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject< visibility_store_type >(0);
    wt_store_data = store->GetObject< weight_store_type >(0);

    if(vis_store_data == nullptr)
    {
        msg_error("initialization", "failed to read visibility data from the .cor file." << eom);
        return;
    }

    if(wt_store_data == nullptr)
    {
        msg_error("initialization", "failed to read weight data from the .cor file." << eom);
        return;
    }

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("initialization",
                 "multiple visibility and/or weight types per-baseline not yet supported, will use first one located." << eom);
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

    MHO_ElementTypeCaster< visibility_store_type, visibility_type > up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type > wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects to save space
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    //warn on non-standard shortnames
    if(vis_shortname != "vis")
    {
        msg_warn("initialization",
                 "visibilities do not use canonical short name 'vis', but are called: " << vis_shortname << eom);
    }
    if(wt_shortname != "weight")
    {
        msg_warn("initialization", "weights do not use canonical short name 'weight', but are called: " << wt_shortname << eom);
    }

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}

void MHO_BasicFringeDataConfiguration::configure_station_data(MHO_ScanDataStore* scanStore, MHO_ContainerStore* containerStore,
                                                              std::string ref_station_mk4id, std::string rem_station_mk4id)
{
    //load station data and assign them the names 'ref_sta' or 'rem_sta'
    scanStore->LoadStation(ref_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "ref_sta");
    MHO_UUID pcal_uuid;
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if(!(pcal_uuid.is_empty()))
    {
        containerStore->RenameObject("pcal", "ref_pcal");
    }

    scanStore->LoadStation(rem_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "rem_sta");
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if(!(pcal_uuid.is_empty()))
    {
        containerStore->RenameObject("pcal", "rem_pcal");
    }
}

void MHO_BasicFringeDataConfiguration::init_and_exec_operators(MHO_OperatorBuilderManager* build_manager,
                                                               MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    if(build_manager == nullptr || opToolbox == nullptr)
    {
        msg_error("fringe", "cannot initialize or execute operators if builder or toolbox is missing" << eom);
        return;
    }

    msg_debug("fringe", "initializing and executing operators in " << cat << " category." << eom);

    //build_manager->BuildOperatorCategory(cat);
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt = ops.begin(); opIt != ops.end(); opIt++)
    {
        msg_debug("fringe", "initializing and executing operator: " << (*opIt)->GetName() << ", with priority: "
                                                                    << (*opIt)->Priority() << "." << eom);
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }
}

std::vector< std::string > MHO_BasicFringeDataConfiguration::determine_required_pol_products(std::string polprod)
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

mho_json MHO_BasicFringeDataConfiguration::ConvertProfileEvents(std::vector< MHO_ProfileEvent >& events)
{
    mho_json event_list;
    for(std::size_t i = 0; i < events.size(); i++)
    {
        mho_json obj;
        obj["event_id"] = i;
        obj["flag"] = events[i].fFlag;
        obj["line"] = events[i].fLineNumber;
        obj["thread_id"] = events[i].fThreadID;
        obj["filename"] = std::string(events[i].fFilename);
        obj["funcname"] = std::string(events[i].fFuncname);
        obj["time"] = events[i].fTime;
        event_list.push_back(obj);
    }
    return event_list;
}

} // namespace hops
