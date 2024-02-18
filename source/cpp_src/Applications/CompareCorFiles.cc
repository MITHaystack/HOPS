#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;

#define SINGLE_PRECISION_MACHINE_EPS 1.19e-07


int main(int argc, char** argv)
{
    std::string usage = "CompareCorFiles -a <fileA> -b <fileB>";

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
    MHO_ContainerStore conStore1;
    MHO_ContainerFileInterface conInter1;
    conInter1.SetFilename(file1);
    conInter1.PopulateStoreFromFile(conStore1);

    MHO_ContainerStore conStore2;
    MHO_ContainerFileInterface conInter2;
    conInter2.SetFilename(file2);
    conInter2.PopulateStoreFromFile(conStore2);

    //retrieve the first two visibility objects
    visibility_store_type* vis1 = conStore1.GetObject<visibility_store_type>(0);
    visibility_store_type* vis2 = conStore2.GetObject<visibility_store_type>(0);

    //retrieve the first two weight objects
    weight_store_type* w1 = conStore1.GetObject<weight_store_type>(0);
    weight_store_type* w2 = conStore2.GetObject<weight_store_type>(0);


    if(vis1 == nullptr || vis2 == nullptr)
    {
        msg_fatal("main", "missing visibility object."<<eom);
        std::exit(1);
    }

    std::size_t size1 = vis1->GetSize();
    std::size_t size2 = vis2->GetSize();

    //now do the comparison
    double L2_norm = 0;
    double L1_norm = 0;
    double rel_L1_norm = 0;

    if(size1 == size2)
    {

        for(std::size_t i=0; i<size1; i++)
        {
            std::complex<double> delta = (*vis1)[i] - (*vis2)[i];
            double geom_mean_mag = std::sqrt( std::abs( (*vis1)[i] ) * std::abs( (*vis2)[i] ) );

            L2_norm += std::real( delta*std::conj(delta) );
            double abs_del = std::abs(delta);
            double rel_del = abs_del/geom_mean_mag;
            if(L1_norm < abs_del){L1_norm = abs_del;}
            if(rel_L1_norm < rel_del){rel_L1_norm = rel_del;};
        }

        L2_norm = std::sqrt(L2_norm);

        std::cout<<"Visibility absolute L1_norm difference = "<<L1_norm<<std::endl;
        std::cout<<"Visibility relative L1 norm difference = "<<rel_L1_norm<<std::endl;
        std::cout<<"Visibility absolute L2 norm difference = "<<L2_norm<<std::endl;
        std::cout<<"Visibility average absolute L2 norm difference = "<<L2_norm/(double)size1<<std::endl;
    }
    else
    {
        msg_fatal("main", "visibility objects have different sizes, cannot compare." <<eom);
        std::exit(1);
    }



    if(w1 == nullptr || w2 == nullptr)
    {
        msg_fatal("main", "missing weight object."<<eom);
        std::exit(1);
    }

    std::size_t wsize1 = w1->GetSize();
    std::size_t wsize2 = w2->GetSize();

    //now do the comparison
    double wL2_norm = 0;
    double wL1_norm = 0;
    double wrel_L1_norm = 0;

    if(wsize1 == wsize2)
    {
        for(std::size_t i=0; i<wsize1; i++)
        {
            double delta = (*w1)[i] - (*w2)[i];
            double geom_mean_mag = std::sqrt( std::abs( (*w1)[i] ) * std::abs( (*w2)[i] ) );

            wL2_norm += delta*delta;
            double abs_del = std::abs(delta);
            double rel_del = abs_del/geom_mean_mag;
            if(wL1_norm < abs_del){wL1_norm = abs_del;}
            if(wrel_L1_norm < rel_del){wrel_L1_norm = rel_del;};
        }

        wL2_norm = std::sqrt(wL2_norm);
        std::cout<<"Weight absolute L1_norm difference = "<<wL1_norm<<std::endl;
        std::cout<<"Weight relative L1 norm difference = "<<wrel_L1_norm<<std::endl;
        std::cout<<"Weight absolute L2 norm difference = "<<wL2_norm<<std::endl;
        std::cout<<"Weight average absolute L2 norm difference = "<<wL2_norm/(double)wsize1<<std::endl;

    }
    else
    {
        msg_fatal("main", "visibility objects have different sizes, cannot compare." <<eom);
        std::exit(1);
    }


    //if the relative L1 norm is worse than 10x the machine precision, return error val rather than 0
    if( rel_L1_norm > 10*SINGLE_PRECISION_MACHINE_EPS){return 1;}
    if( wrel_L1_norm > 10*SINGLE_PRECISION_MACHINE_EPS){return 2;}


    return 0;
}
