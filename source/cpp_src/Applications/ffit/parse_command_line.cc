#include "ffit.hh"

#include <getopt.h>
#include <iomanip>

int parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
{
    //TODO make this conform/support most of the command line options of fourfit
    std::string usage = "ffit -d <directory> -c <control file> -b <baseline> -P <pol. product>";

    std::string directory = "";
    std::string control_file = "";
    std::string baseline = "";
    std::string polprod = "";
    std::string output_file = "fdump.json"; //for testing
    int message_level = -1;
    int ap_per_seg = 0;
    bool ok;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"control", required_argument, 0, 'c'},
                                          {"baseline", required_argument, 0, 'b'},
                                          {"polarization-product", required_argument, 0, 'P'},
                                          {"message-level", required_argument, 0, 'm'},
                                          {"ap-per-seg", required_argument, 0, 's'},
                                          {"output", required_argument, 0, 'o'}};

    static const char* optString = "hd:c:b:P:o:m:s:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                std::exit(0);
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('c'):
                control_file = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            case ('P'):
                polprod = std::string(optarg);
                break;
            case ('o'):
                output_file = std::string(optarg);
                break;
            case ('m'):
                message_level = std::atoi(optarg);
                break;
            case ('s'):
                ap_per_seg = std::atoi(optarg);
                if(ap_per_seg < 0){ap_per_seg = 0; msg_warn("main", "invalid ap_per_seg, ignoring." << eom);}
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( directory == "" || baseline == "" || polprod == "" || control_file == "")
    {
        msg_fatal("main", "usage: "<< usage << eom);
        return 1;
    }

    //set the message level according to the fourfit style
    //where 3 is least verbose, and '-1' is most verbose
    switch (message_level)
    {
        case -2:
            //NOTE: debug messages must be compiled-in
            #ifndef HOPS_ENABLE_DEBUG_MSG
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
            msg_warn("main", "debug messages are toggled via compiler flag, re-compile with ENABLE_DEBUG_MSG=ON to enable." << eom);
            #else
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
            #endif
        break;
        case -1:
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
        break;
        case 0:
            MHO_Message::GetInstance().SetMessageLevel(eStatus);
        break;
        case 1:
            MHO_Message::GetInstance().SetMessageLevel(eWarning);
        break;
        case 2:
            MHO_Message::GetInstance().SetMessageLevel(eError);
        break;
        case 3:
            MHO_Message::GetInstance().SetMessageLevel(eFatal);
        break;
        case 4:
            MHO_Message::GetInstance().SetMessageLevel(eSilent);
        break;
        default:
            //for now default is most verbose, eventually will change this to silent
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
    }

    if(baseline.size() != 2)
    {
        msg_fatal("main", "baseline must be passed as 2-char code."<< eom);
        return 1;
    }

    //store the raw arguments in the parameter store
    std::vector<std::string> arglist;
    for(int i=0; i<argc; i++)
    {
        arglist.push_back( std::string(argv[i]) );
    }
    paramStore->Set("/cmdline/args", arglist);

    //pass the extracted info back in the parameter store
    paramStore->Set("/cmdline/directory", directory);
    paramStore->Set("/cmdline/baseline", baseline);
    paramStore->Set("/cmdline/polprod", polprod);
    paramStore->Set("/cmdline/control_file",control_file);
    paramStore->Set("/cmdline/ap_per_seg",ap_per_seg);
    paramStore->Set("/cmdline/output_file",output_file);

    return 0;

}
