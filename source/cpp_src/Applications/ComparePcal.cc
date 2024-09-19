#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;

#define SINGLE_PRECISION_MACHINE_EPS 1.19e-07


int main(int argc, char** argv)
{
    std::string usage = "ComparePcal -a <fileA> -b <fileB>";

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

    //retrieve the first two pcal objects
    multitone_pcal_type* pcal1 = conStore1.GetObject<multitone_pcal_type>(0);
    multitone_pcal_type* pcal2 = conStore2.GetObject<multitone_pcal_type>(0);

    if(pcal1 == nullptr || pcal2 == nullptr)
    {
        msg_fatal("main", "missing pcal object."<<eom);
        std::exit(1);
    }

    std::size_t size1 = pcal1->GetSize();
    std::size_t size2 = pcal2->GetSize();

    //now do the comparison
    double L2_norm = 0;
    double L1_norm = 0;
    double rel_L1_norm = 0;


    msg_debug("main", "pcal object 'A' has size: ("
    << pcal1->GetDimension(0)<<", "
    << pcal1->GetDimension(1)<<", "
    << pcal1->GetDimension(2)<<") " << eom );

    msg_debug("main", "pcal object 'B' has size: ("
    << pcal2->GetDimension(0)<<", "
    << pcal2->GetDimension(1)<<", "
    << pcal2->GetDimension(2)<<") " << eom );

    std::size_t npol1 = pcal1->GetDimension(0);
    std::size_t nap1 =  pcal1->GetDimension(1);
    std::size_t nfreq1 = pcal1->GetDimension(2);

    std::size_t npol2 = pcal2->GetDimension(0);
    std::size_t nap2 =  pcal2->GetDimension(1);
    std::size_t nfreq2 = pcal2->GetDimension(2);

    if(size1 == size2)
    {
        for(std::size_t i=0; i<size1; i++)
        {
            std::complex<double> delta = (*pcal1)[i] - (*pcal2)[i];
            double geom_mean_mag = std::sqrt( std::abs( (*pcal1)[i] ) * std::abs( (*pcal2)[i] ) );

            L2_norm += std::real( delta*std::conj(delta) );
            double abs_del = std::abs(delta);
            double rel_del = abs_del/geom_mean_mag;
            if(L1_norm < abs_del){L1_norm = abs_del;}
            if(rel_L1_norm < rel_del){rel_L1_norm = rel_del;};
        }

        L2_norm = std::sqrt(L2_norm);

        std::cout<<"pcal absolute L1_norm difference = "<<L1_norm<<std::endl;
        std::cout<<"pcal relative L1 norm difference = "<<rel_L1_norm<<std::endl;
        std::cout<<"pcal absolute L2 norm difference = "<<L2_norm<<std::endl;
        std::cout<<"pcal average absolute L2 norm difference = "<<L2_norm/(double)size1<<std::endl;
    }
    else if( npol1 == npol2 && nap1 == nap1)
    {
        //try to do a comparison across just the tones which match in frequency 
        double freq_tol = 0.001;
    
        auto tone_ax1 = &(std::get<MTPCAL_FREQ_AXIS>(*pcal1));
        auto tone_ax2 = &(std::get<MTPCAL_FREQ_AXIS>(*pcal2));

        for(std::size_t p=0;p<npol1;p++)
        {
            for(std::size_t ap=0;ap<nap1;ap++)
            {
                for(std::size_t tf1=0; tf1<nfreq1; tf1++)
                {
                    double f1 = tone_ax1->at(tf1);
                    for(std::size_t tf2=0; tf2<nfreq2; tf2++)
                    {
                        double f2 = tone_ax2->at(tf2);
                        if( std::fabs(f2-f1) < freq_tol)
                        {

                            
                            std::complex<double> a = (*pcal1)(p,ap,tf1);
                            std::complex<double> b = (*pcal2)(p,ap,tf2);
                            std::complex<double> delta = a-b;
                            double geom_mean_mag = std::sqrt( std::abs(a) * std::abs(b) );


                            if(ap == 0)
                            {
                                std::cout<<f1<<", "<<f2<<", "<<a<<", "<<b<<std::endl;
                            }

                            L2_norm += std::real( delta*std::conj(delta) );
                            double abs_del = std::abs(delta);
                            double rel_del = abs_del/geom_mean_mag;
                            if(L1_norm < abs_del){L1_norm = abs_del;}
                            if(rel_L1_norm < rel_del){rel_L1_norm = rel_del;};

                            break;
                        }
                    }
                }
            }
        }

        std::cout<<"pcal absolute L1_norm difference = "<<L1_norm<<std::endl;
        std::cout<<"pcal relative L1 norm difference = "<<rel_L1_norm<<std::endl;
        std::cout<<"pcal absolute L2 norm difference = "<<L2_norm<<std::endl;
        std::cout<<"pcal average absolute L2 norm difference = "<<L2_norm/(double)size1<<std::endl;

    }
    else 
    {
        msg_fatal("main", "pcal objects have different sizes, cannot compare." <<eom);
        std::exit(1);
    }

    //if the relative L1 norm is worse than 10x the machine precision, return error val rather than 0
    if( rel_L1_norm > 10*SINGLE_PRECISION_MACHINE_EPS){return 1;}


    return 0;
}
