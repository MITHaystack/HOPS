#include "MHO_MK4ScanConverter.hh"

namespace hops
{

MHO_MK4ScanConverter::MHO_MK4ScanConverter(){};

MHO_MK4ScanConverter::~MHO_MK4ScanConverter(){};


//convert a corel file
void 
MHO_MK4ScanConverter::ConvertCorel(const std::string& root_file, 
                                const std::string& input_file,
                                const std::string& output_file)
{
    MHO_MK4CorelInterface mk4inter;

    msg_info("mk4interface", "converting corel input file: " << input_file << eom);
    mk4inter.SetCorelFile(input_file);
    mk4inter.SetVexFile(root_file);
    mk4inter.ExtractCorelFile();
    uch_visibility_store_type* bl_data = mk4inter.GetExtractedVisibilities();
    uch_weight_store_type* bl_wdata = mk4inter.GetExtractedWeights();

    MHO_VisibilityChannelizer channelizer;
    visibility_store_type* ch_bl_data = new visibility_store_type();
    channelizer.SetArgs(bl_data, ch_bl_data);
    bool init = channelizer.Initialize();
    if(init)
    {
        bool exe = channelizer.Execute();
        if(!exe){msg_error("mk4interface", "failed to channelize visibility data." << eom);}
    }
    ch_bl_data->CopyTags(*bl_data);

    MHO_WeightChannelizer wchannelizer;
    weight_store_type* ch_bl_wdata = new weight_store_type();
    wchannelizer.SetArgs(bl_wdata, ch_bl_wdata);
    bool winit = wchannelizer.Initialize();
    if(winit)
    {
        bool wexe = wchannelizer.Execute();
        if(!wexe){msg_error("mk4interface", "failed to channelize weight data." << eom);}
    }
    ch_bl_wdata->CopyTags(*bl_wdata);

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //someday make this mean something
        inter.Write(*ch_bl_data, "vis", label);
        inter.Write(*ch_bl_wdata, "weight", label);
    }
    else
    {
        msg_error("mk4interface", "Error opening corel output file: " << output_file << eom);
    }

    #pragma message("TODO FIXME -- add an tags object for extraneous meta data");

    inter.Close();

    delete bl_data;
    delete ch_bl_data;
}



//convert a station data  file
void 
MHO_MK4ScanConverter::ConvertStation(const std::string& root_file,
                                  const std::string& input_file,
                                  const std::string& output_file)
{
    MHO_MK4StationInterface mk4inter;

    msg_info("mk4interface", "Converting station input file: " << input_file << eom);
    mk4inter.SetStationFile(input_file);
    // mk4inter.SetVexFile(root_file);
    station_coord_type* st_data = mk4inter.ExtractStationFile();

    std::size_t n_pcal_obj = mk4inter.GetNPCalObjects();

    MHO_BinaryFileInterface inter;
    //std::string index_file = output_file + ".index";
    //bool status = inter.OpenToWrite(output_file, index_file);

    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF;
        inter.Write(*st_data, "sta", label); //write out station data
        //write out pcal objects
        for(std::size_t i=0; i<n_pcal_obj; i++)
        {
            inter.Write( *(mk4inter.GetPCalObject(i)), "pcal", label);
        }
        inter.Close();
    }
    else
    {
        msg_error("mk4interface", "error opening station output file: " << output_file << eom);
    }

    inter.Close();
    delete st_data;
}

void 
MHO_MK4ScanConverter::ProcessScan(const std::string& in_dir, const std::string& out_dir)
{
    //directory interface
    MHO_DirectoryInterface dirInterface;
    std::string output_dir = dirInterface.GetDirectoryFullPath(out_dir);
    std::string input_dir = dirInterface.GetDirectoryFullPath(in_dir);

    msg_info("mk4interface", "processing scan from input directory: " << input_dir <<
        " to output directory: "<< output_dir << eom );

    if( !dirInterface.DoesDirectoryExist(output_dir) )
    {
        dirInterface.CreateDirectory(output_dir);
    }

    //get list of all the files (and directories) in the input directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allDirs;

    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFileList(allFiles);
    dirInterface.GetSubDirectoryList(allDirs);

    //sort files, locate root, corel and station files
    std::vector< std::string > corelFiles;
    std::vector< std::string > stationFiles;
    std::string root_file;
    dirInterface.GetRootFile(allFiles, root_file);

    MHO_MK4VexInterface vexInter;
    vexInter.OpenVexFile(root_file);
    mho_json ovex;
    bool ovex_ok = vexInter.ExportVexFileToJSON(ovex);
    if(ovex_ok)
    {
        //write out to a json file
        std::string output_file = output_dir + "/" + dirInterface.GetBasename(root_file) + ".root.json";

        //open file for binary writing
        std::fstream jfile;
        jfile.open(output_file.c_str(), std::fstream::out);
        if( !jfile.is_open() || !jfile.good() )
        {
            msg_error("mk4interface", "failed to open for writing, file: " << output_file << eom);
        }
        else
        {
            jfile << std::setw(4) << ovex;
            jfile.close();
        }
    }
    else 
    {
        msg_error("mk4interface", "could not convert root file: "<<root_file<< eom);
    }

    dirInterface.GetCorelFiles(allFiles, corelFiles);
    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::string st_pair, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitCorelFileBasename(input_basename, st_pair, root_code);
        std::string output_file = output_dir + "/" + st_pair + "." + root_code + ".cor";
        ConvertCorel(root_file, *it, output_file);
    }

    dirInterface.GetStationFiles(allFiles, stationFiles);
    for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    {
        std::string st, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitStationFileBasename(input_basename, st, root_code);
        std::string output_file = output_dir + "/" + st + "." + root_code + ".sta";
        ConvertStation(root_file, *it, output_file);
    }
}




}//end of namespace
