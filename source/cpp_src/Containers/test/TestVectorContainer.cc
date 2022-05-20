#include <iostream>
#include <string>

#include "MHO_VectorContainer.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"


using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    size_t dim = 100;

    MHO_VectorContainer<double>* test = new MHO_VectorContainer<double>(dim);

    std::cout<<"dimension @ 0 ="<<test->GetDimension(0)<<std::endl;
    std::cout<<"total array size = "<<test->GetSize()<<std::endl;

    std::cout<<" class name: "<< MHO_ClassIdentity::ClassName(*test) <<std::endl;


    double* data = test->GetData();

    for(unsigned int i=0; i<dim; i++)
    {
        data[i] = i;
    }

    std::cout<<"data @ 3 = "<<(*test)(3)<<std::endl;

    std::string filename = "./test.bin";

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);

    if(status)
    {
        uint32_t label = 0xFF00FF00;
        std::string shortname = "junk";
        std::cout<<"A label = "<<label<<std::endl;
        std::cout<<"expected object size = "<<test->GetSerializedSize()<<std::endl;
        inter.Write(*test, shortname, label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    delete test;

    MHO_VectorContainer<double>* test2 = new MHO_VectorContainer<double>();

    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*test2, key);
        std::cout<<"data @ 3 = "<<(*test2)(3)<<std::endl;
    }
    else
    {
        std::cout<<" error opening file to read"<<std::endl;
    }

    inter.Close();

    delete test2;

    return 0;
}
