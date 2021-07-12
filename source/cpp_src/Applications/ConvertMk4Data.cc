#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>


//needed for listing/navigating files/directories on *nix
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4StationInterface.hh"

#include "MHO_Reducer.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

using namespace hops;


std::size_t count_number_of_matches(const std::string& aString, char elem)
{
    std::size_t n_elem = 0;
    for(std::size_t i=0; i<aString.size(); i++)
    {
        if(aString[i] == elem){n_elem++;}
    }
    return n_elem;
}


void get_root_file(const std::vector<std::string>& files, std::string& root_file)
{
    //sift through the list of files to find the one which matches the
    //root (ovex) file characteristics (this is relatively primative)
    root_file = "";
    for(auto it = files.begin(); it != files.end(); it++)
    {
        std::string base_filename = it->substr(it->find_last_of("/\\") + 1);
        if(count_number_of_matches(base_filename, '.') == 1) //check that there is one dot in the filename base
        {
            //check to make sure we have 6 character 'root code' extension
            std::string dot(".");
            std::size_t dotpos = base_filename.find(dot);
            if(dotpos != std::string::npos)
            {
                std::string root_code = base_filename.substr(dotpos+1);
                if( root_code.size() == 6 )
                {
                    //open file and check that "VEX" is present in the first few bytes of the file
                    std::fstream test_file(it->c_str(), std::ios::in | std::ios::binary);
                    std::size_t num_bytes = 16;
                    char data[16];
                    for(std::size_t i=0; i<num_bytes; i++)
                    {
                        test_file.read(&(data[i]),1);
                    }
                    data[15] = '\0';
                    std::string test(data);
                    std::string vex("VEX");
                    std::size_t index = test.find(vex);
                    std::cout<<"test = "<<test<<std::endl;
                    if( index != std::string::npos)
                    {
                        std::cout<<"found: "<<*it<<std::endl;
                        //found an ovex file
                        //TODO FIXME....what happens if there is more then one ovex file?!
                        root_file = *it;
                        return;
                    }
                }
            }


        }
    }
}

std::string get_basename(const std::string& filename)
{
    std::string base_filename = filename.substr(filename.find_last_of("/\\") + 1);
    return base_filename;
}

void split_corel_file_basename(const std::string& corel_basename, std::string& st_pair, std::string& root_code)
{
    st_pair = "";
    root_code = "";
    //check that the two dots are 'concurrent'
    std::string dots("..");
    std::size_t index = corel_basename.find(dots);
    if(index != std::string::npos)
    {
        //split the string at the dots into 'station pair' and 'root code'
        st_pair = corel_basename.substr(0,index);
        root_code = corel_basename.substr(index+dots.size());
    }
}

void get_corel_files(const std::vector<std::string>& files, std::vector<std::string>& corel_files)
{
    corel_files.clear();
    //sift through the list of files to find which ones matche the
    //corel file characteristics and put them in the corel_files list
    for(auto it = files.begin(); it != files.end(); it++)
    {
        std::string base_filename = get_basename(*it); // it->substr(it->find_last_of("/\\") + 1);
        if(count_number_of_matches(base_filename, '.') == 2) //check that there is two dots in the filename base
        {
            std::string st_pair, root_code;
            split_corel_file_basename(base_filename, st_pair, root_code);

            //check that the two dots are 'concurrent'
            std::string dots("..");
            std::size_t index = base_filename.find(dots);
            if(st_pair.size() == 2 && root_code.size() == 6)
            {
                corel_files.push_back(*it);
            }
        }
    }
}

void split_station_file_basename(const std::string& station_basename, std::string& st, std::string& root_code)
{
    st = "";
    root_code = "";
    std::string dots("..");
    std::size_t index = station_basename.find(dots);
    if(index != std::string::npos)
    {
        //split the string at the dots into 'station pair' and 'root code'
        st = station_basename.substr(0,index);
        root_code = station_basename.substr(index+dots.size());
    }
}

void get_station_files(const std::vector<std::string>& files, std::vector<std::string>& station_files)
{
    //sift through the list of files to find the ones which match the
    //station file characteristics
    station_files.clear();
    for(auto it = files.begin(); it != files.end(); it++)
    {
        std::string base_filename = it->substr(it->find_last_of("/\\") + 1);
        if(count_number_of_matches(base_filename, '.') == 2) //check that there is two dots in the filename base
        {
            //check that the two dots are 'concurrent'
            std::string dots("..");
            std::size_t index = base_filename.find(dots);
            if( index != std::string::npos)
            {
                //split the string at the dots into 'station' and 'root code'
                std::string st = base_filename.substr(0,index);
                std::string root_code = base_filename.substr(index+dots.size());
                if(st.size() == 1 && root_code.size() == 6)
                {
                    station_files.push_back(*it);
                }
            }
        }
    }
}


//convert a corel file
void ConvertCorel(const std::string root_file, const std::string& input_file, const std::string& output_file)
{
    MHO_MK4CorelInterface mk4inter;

    std::cout<<"input_file = "<<input_file<<std::endl;
    mk4inter.SetCorelFile(input_file);
    mk4inter.SetVexFile(root_file);
    mk4inter.ExtractCorelFile();
    baseline_data_type* bl_data = mk4inter.GetExtractedVisibilities();
    baseline_weight_type* bl_wdata = mk4inter.GetExtractedWeights();

    MHO_VisibilityChannelizer channelizer;
    channelizer.SetInput(bl_data);
    ch_baseline_data_type* ch_bl_data = new ch_baseline_data_type();
    channelizer.SetOutput(ch_bl_data);
    bool init = channelizer.Initialize();
    std::cout<<"initialization done"<<std::endl;
    bool exe = channelizer.ExecuteOperation();
    if(exe){std::cout<<"vis channelizer done"<<std::endl;}

    MHO_WeightChannelizer wchannelizer;
    wchannelizer.SetInput(bl_wdata);
    ch_baseline_weight_type* ch_bl_wdata = new ch_baseline_weight_type();
    wchannelizer.SetOutput(ch_bl_wdata);
    bool winit = wchannelizer.Initialize();
    std::cout<<"initialization done"<<std::endl;
    bool wexe = wchannelizer.ExecuteOperation();
    if(wexe){std::cout<<"weight channelizer done"<<std::endl;}

    //std::string index_file = output_file + ".index";
    //bool status = inter.OpenToWrite(output_file, index_file);
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF;
        inter.Write(*ch_bl_data, "vis", label);
        inter.Write(*ch_bl_wdata, "weight", label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    delete bl_data;
    delete ch_bl_data;
}



//convert a station data  file
void ConvertStation(const std::string root_file, const std::string& input_file, const std::string& output_file)
{
    MHO_MK4StationInterface mk4inter;

    std::cout<<"input_file = "<<input_file<<std::endl;
    mk4inter.SetStationFile(input_file);
    mk4inter.SetVexFile(root_file);
    station_coord_data_type* st_data = mk4inter.ExtractStationFile();

    MHO_BinaryFileInterface inter;
    //std::string index_file = output_file + ".index";
    //bool status = inter.OpenToWrite(output_file, index_file);

    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF;
        inter.Write(*st_data, "sta", label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();
    delete st_data;

}





int main(int argc, char** argv)
{
    std::string usage = "ConvertMk4Data -i <input_directory> -o <output_directory>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string output_dir;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"output_directory", required_argument, 0, 'o'}};

    static const char* optString = "hi:o:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('i'):
                input_dir = std::string(optarg);
                break;
            case ('o'):
                output_dir = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allDirs;
    DIR *dpdf;
    struct dirent* epdf = NULL;
    dpdf = opendir(input_dir.c_str());
    if (dpdf != NULL)
    {
        do
        {
            epdf = readdir(dpdf);
            if(epdf != NULL)
            {
                std::string tmp_path = input_dir + "/" + std::string(epdf->d_name);
                char buffer[PATH_MAX];
                for(std::size_t i=0; i<PATH_MAX; i++){buffer[i] = '\0';}
                char* tmp = realpath( tmp_path.c_str(), buffer);
                std::string fullpath(buffer);
                if(fullpath.size() != 0)
                {
                    struct stat st;
                    if(stat( fullpath.c_str(), &st) == 0 )
                    {
                        if( st.st_mode & S_IFDIR ){allDirs.push_back(fullpath);}
                        if( st.st_mode & S_IFREG ){allFiles.push_back(fullpath);}
                    }
                }
            }
        }
        while(epdf != NULL);
    }
    closedir(dpdf);

    //TODO FIXME - check if the output directory exists, and if not, create it


    //debug
    for(auto it=allFiles.begin(); it != allFiles.end(); it++)
    {
        std::cout<<"file: "<<*it<<std::endl;
    }

    //sort files, locate root, corel and station files
    std::vector< std::string > corelFiles;
    std::vector< std::string > stationFiles;
    std::string root_file;
    get_root_file(allFiles, root_file);
    std::cout<<"root file = "<<root_file<<std::endl;

    MHO_MK4VexInterface vexInter;
    vexInter.OpenVexFile(root_file);
    json ovex;
    bool ovex_ok = vexInter.ExportVexFileToJSON(ovex);
    if(ovex_ok)
    {
        std::cout<<ovex.dump(2)<<std::endl;
        //write out to a json file
        std::string output_file = output_dir + "/" + get_basename(root_file) + ".json";

        //open file for binary writing
        std::fstream jfile;
        jfile.open(output_file.c_str(), std::fstream::out);
        if( !jfile.is_open() || !jfile.good() )
        {
            msg_error("file", "Failed to open for writing, file: " << output_file << eom);
        }
        else
        {
            jfile << std::setw(4) << ovex;
            jfile.close();
        }
    }

    // // write prettified JSON to another file
    // std::ofstream o("pretty.json");
    // o << std::setw(4) << j << std::endl;



    get_corel_files(allFiles, corelFiles);
    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::cout<<"corel file: "<< *it <<std::endl;
        std::string st_pair, root_code;
        std::string input_basename = get_basename(*it);
        split_corel_file_basename(input_basename, st_pair, root_code);
        std::string output_file = output_dir + "/" + st_pair + "." + root_code + ".cor";
        ConvertCorel(root_file, *it, output_file);
    }

    get_station_files(allFiles, stationFiles);
    for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    {
        std::cout<<"station file: "<< *it <<std::endl;
        std::string st, root_code;
        std::string input_basename = get_basename(*it);
        split_station_file_basename(input_basename, st, root_code);
        std::string output_file = output_dir + "/" + st + "." + root_code + ".sta";
        ConvertStation(root_file, *it, output_file);
    }











    return 0;
}
