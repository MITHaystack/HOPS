#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "CompareVisibilityObjects -a <fileA> -b <fileB>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string file1 = "";
    std::string file2 = "";
    std::string type_uuid = "";

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"fileA", required_argument, 0, 'a'},
                                          {"fileB", required_argument, 0, 'b'}
    };

    static const char* optString = "ha:b:";

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
            case ('a'):
                file1 = std::string(optarg);
                break;
            case ('b'):
                file2 = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //reads in all the objects in a file, this may not be super desireable for large files
    std::cout<<"file1 = "<<file1<<std::endl;
    MHO_ContainerStore conStore1;
    MHO_ContainerFileInterface conInter1;
    conInter1.SetFilename(file1);
    conInter1.PopulateStoreFromFile(conStore1);

    std::cout<<"file2 = "<<file2<<std::endl;
    MHO_ContainerStore conStore2;
    MHO_ContainerFileInterface conInter2;
    conInter2.SetFilename(file2);
    conInter2.PopulateStoreFromFile(conStore2);


    //retrieve the first two visibility objects
    visibility_store_type* vis1 = conStore1.RetrieveObject<visibility_store_type>();
    visibility_store_type* vis2 = conStore2.RetrieveObject<visibility_store_type>();

    if(vis1 == nullptr || vis2 == nullptr)
    {
        msg_fatal("main", "missing visibility object."<<eom);
        std::exit(1);
    }

    std::size_t size1 = vis1->GetSize();
    std::size_t size2 = vis2->GetSize();

    if(size1 == size2)
    {
        //now do the comparison
        double L2_norm = 0;
        double L1_norm = 0;
        
        for(std::size_t i=0; i<size1; i++)
        {
            std::complex<double> delta = (*vis1)[i] - (*vis2)[i];
            L2_norm += std::real( delta*std::conj(delta) );
            double abs_del = std::abs(delta);
            if(L1_norm < abs_del){L1_norm = abs_del;}
        }
    
        L2_norm = std::sqrt(L2_norm);
        
        std::cout<<"L1_norm difference = "<<L1_norm<<std::endl;
        std::cout<<"L2 norm difference = "<<L2_norm<<std::endl;
        std::cout<<"Average L2 norm difference = "<<L2_norm/(double)size1<<std::endl;
    
    }
    else 
    {
        msg_fatal("main", "visibility objects have different sizes, cannot compare." <<eom);
        std::exit(1);
    }

    return 0;
}
