#include <iostream>
#include <string>

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"


using namespace hops;


using visibility_type = std::complex<double>;

using polprod_axis_type = MHO_Axis<std::string>;
using frequency_axis_type = MHO_Axis<double>;
using time_axis_type = MHO_Axis<double>;

#define VIS_NDIM 3
#define POLPROD_AXIS 0
#define TIME_AXIS 1
#define FREQ_AXIS 2

using baseline_axis_pack = MHO_AxisPack< polprod_axis_type, time_axis_type, frequency_axis_type >;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    size_t dims[VIS_NDIM] = {4,64,64};

    baseline_axis_pack* test = new baseline_axis_pack(dims);

    std::get<POLPROD_AXIS>(*test).at(0) = std::string("XX");
    std::get<POLPROD_AXIS>(*test).at(1) = std::string("XY");
    std::get<POLPROD_AXIS>(*test).at(2) = std::string("YY");
    std::get<POLPROD_AXIS>(*test).at(3) = std::string("YX");

    for(unsigned int i=0; i<dims[FREQ_AXIS]; i++)
    {
        std::get<FREQ_AXIS>(*test).at(i) = (double)i + 0.1;
    }

    std::cout<<"data @ 3 = "<<std::get<FREQ_AXIS>(*test).at(3) <<std::endl;

    MHO_IntervalLabel label1(0,4);
    MHO_IntervalLabel label2(4,6);
    MHO_IntervalLabel label3(6,9);
    MHO_IntervalLabel label4(9,12);
    MHO_IntervalLabel label5(0,6);
    MHO_IntervalLabel label6(6,12);

    label1.Insert(std::string("channel"), 'a');
    label1.Insert(std::string("bandwidth"), 32.0e6);
    label2.Insert(std::string("channel"), 'b');
    label2.Insert(std::string("bandwidth"), 32.0e6);
    label3.Insert(std::string("channel"), 'c');
    label3.Insert(std::string("bandwidth"), 32.0e6);
    label4.Insert(std::string("channel"), 'd');
    label4.Insert(std::string("bandwidth"), 32.0e6);
    label5.Insert(std::string("sampler"), std::string("r2dbe-1"));
    label6.Insert(std::string("sampler"), std::string("r2dbe-2"));

    std::get<FREQ_AXIS>(*test).InsertLabel(label1);
    std::get<FREQ_AXIS>(*test).InsertLabel(label2);
    std::get<FREQ_AXIS>(*test).InsertLabel(label3);
    std::get<FREQ_AXIS>(*test).InsertLabel(label4);
    std::get<FREQ_AXIS>(*test).InsertLabel(label5);
    std::get<FREQ_AXIS>(*test).InsertLabel(label6);

    std::string filename = "./test-axis-pack.bin";

    std::cout<<" class name: "<< MHO_ClassIdentity::ClassName(*test) <<std::endl;

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);


    std::cout<<"size in bytes of the object: "<<test->GetSerializedSize()<<std::endl;

    if(status)
    {
        uint32_t label = 0xFF00FF00;
        std::cout<<"A label = "<<label<<std::endl;
        inter.Write(*test, "axis-pack", label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    delete test;

    baseline_axis_pack* test2 = new baseline_axis_pack(dims);

    std::cout<<"-------- Now testing read back of object --------- "<<std::endl;

    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*test2, key);

        std::cout<<"pol product axis labels = "<<std::endl;
        std::cout<<std::get<POLPROD_AXIS>(*test2).at(0)<<std::endl;
        std::cout<<std::get<POLPROD_AXIS>(*test2).at(1)<<std::endl;
        std::cout<<std::get<POLPROD_AXIS>(*test2).at(2)<<std::endl;
        std::cout<<std::get<POLPROD_AXIS>(*test2).at(3)<<std::endl;

        std::cout<<"data @ 23 = "<<std::get<FREQ_AXIS>(*test2).at(23)<<std::endl;

        auto label_vec1 = std::get<FREQ_AXIS>(*test2).GetIntervalsWhichIntersect(5);
        for(auto iter = label_vec1.begin(); iter != label_vec1.end(); iter++)
        {
            std::cout<<"label found for interval = ["<<(*iter)->GetLowerBound()<<", "<<(*iter)->GetUpperBound()<<") with key:val pairs = "<<std::endl;
            (*iter)->DumpMap<char>();
            (*iter)->DumpMap<std::string>();
            (*iter)->DumpMap<int>();
        }

    }
    else
    {
        std::cout<<" error opening file to read"<<std::endl;
    }

    inter.Close();

    delete test2;

    return 0;
}
