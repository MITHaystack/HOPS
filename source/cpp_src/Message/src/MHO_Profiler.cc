#include "MHO_Profiler.hh"

namespace hops
{


//static initialization to nullptr
MHO_Profiler* MHO_Profiler::fInstance = nullptr;


void MHO_Profiler::AddEntry(int flag, uint64_t thread_id, std::string filename, int line_num, std::string func_name)
{
    if(fDisabled){return;}

    MHO_ProfileEvent event;
    event.fFlag = flag;
    event.fLineNumber = line_num;
    event.fThreadID = thread_id;
    strncpy(event.fFilename, filename.c_str(), PROFILE_INFO_LEN);
    event.fFilename[PROFILE_INFO_LEN-1] = '\0';
    strncpy(event.fFuncname, func_name.c_str(), PROFILE_INFO_LEN);
    event.fFilename[PROFILE_INFO_LEN-1] = '\0';
    event.fTime = fTimer.GetTimeSinceStart();
    fEvents.push_back(event);

}

void MHO_Profiler::DumpEvents()
{
    for(std::size_t i=0; i<fEvents.size(); i++)
    {
        std::cout<<"event: "<<i<<std::endl;
        std::cout<<"flag: "<< fEvents[i].fFlag<<std::endl;
        std::cout<<"line: "<< fEvents[i].fLineNumber<<std::endl;
        std::cout<<"thread id: "<< fEvents[i].fThreadID<<std::endl;
        std::cout<<"filename: "<< fEvents[i].fFilename<<std::endl;
        std::cout<<"funcname: "<< fEvents[i].fFuncname<<std::endl;
        std::cout<<"time: "<< fEvents[i].fTime<<std::endl;
        std::cout<<std::endl;
    }
}


}
