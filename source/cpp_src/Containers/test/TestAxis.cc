#include <iostream>
#include <string>

#include "MHO_Axis.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"


using namespace hops;



int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    size_t dim = 100;

    MHO_Axis<double>* test = new MHO_Axis<double>(dim);

    std::cout<<"dimension @ 0 ="<<test->GetDimension(0)<<std::endl;
    std::cout<<"total array size = "<<test->GetSize()<<std::endl;

    double* data = test->GetData();

    for(unsigned int i=0; i<dim; i++)
    {
        data[i] = (double)i  + 0.1;
    }

    std::cout<<"data @ 3 = "<<(*test)(3)<<std::endl;

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

    test->InsertLabel(label1);
    test->InsertLabel(label2);
    test->InsertLabel(label3);
    test->InsertLabel(label4);
    test->InsertLabel(label5);
    test->InsertLabel(label6);

    std::string filename = "./test-axis.bin";

    std::cout<<" class name: "<< MHO_ClassIdentity::ClassName(*test) <<std::endl;

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);

    std::cout<<"size in bytes of the object: "<<test->GetSerializedSize()<<std::endl;

    if(status)
    {
        uint32_t label = 0xFF00FF00;
        std::cout<<"A label = "<<label<<std::endl;
        inter.Write(*test, "axis", label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    delete test;

    MHO_Axis<double>* test2 = new MHO_Axis<double>();

    std::cout<<"-------- Now testing read back of object --------- "<<std::endl;

    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*test2, key);
        //std::cout<<"object label = "<<blabel<<std::endl;
        std::cout<<"data @ 3 = "<<(*test2)(3)<<std::endl;

        auto label_vec1 = test2->GetIntervalsWhichIntersect(5);
        for(auto iter = label_vec1.begin(); iter != label_vec1.end(); iter++)
        {
            std::cout<<"label found for interval = ["<<iter->GetLowerBound()<<", "<<iter->GetUpperBound()<<") with key:val pairs = "<<std::endl;
            iter->DumpMap<char>();
            iter->DumpMap<std::string>();
            iter->DumpMap<int>();
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
