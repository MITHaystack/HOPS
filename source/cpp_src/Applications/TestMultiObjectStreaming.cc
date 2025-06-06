#include "MHO_Message.hh"
#include <getopt.h>

#include "MHO_ScalarContainer.hh"
#include "MHO_TableContainer.hh"
#include "MHO_VectorContainer.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_ContainerDefinitions.hh"

using namespace hops;

#define NDIM 3
#define XDIM 0
#define YDIM 1
#define ZDIM 2
typedef MHO_AxisPack< MHO_Axis< double >, MHO_Axis< double >, MHO_Axis< std::string > > axis_pack_test;

int main(int argc, char** argv)
{
    std::string usage = "TestMultiObjectStreaming -f <filename>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string filename;

    static struct option longOptions[] = {
        {"help",     no_argument,       0, 'h'},
        {"filename", required_argument, 0, 'f'}
    };

    static const char* optString = "hf:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if(optId == -1)
            break;
        switch(optId)
        {
            case('h'): // help
                std::cout << usage << std::endl;
                return 0;
            case('f'):
                filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //construct the set of objects we want to stream in/out
    MHO_ScalarContainer< double >* cscalar = new MHO_ScalarContainer< double >();
    cscalar->SetValue(3.14159);
    std::string csname = "pi";
    cscalar->Insert(std::string("name"), csname);
    cscalar->Insert(std::string("units"), std::string("radians"));
    cscalar->Insert(std::string("test"), 1);

    size_t vdim = 100;
    MHO_VectorContainer< int >* cvector = new MHO_VectorContainer< int >(vdim);
    for(std::size_t i = 0; i < vdim; i++)
    {
        cvector->at(i) = i;
    };

    // cvector->SetName(std::string("test-vector-here"));
    // cvector->SetUnits(std::string("m/s"));

    size_t* dim = new size_t[NDIM];
    dim[0] = 256; //x
    dim[1] = 256; //y
    dim[2] = 3;   // r,g,b
    MHO_TableContainer< double, axis_pack_test >* ctable = new MHO_TableContainer< double, axis_pack_test >(dim);
    //set up the axis labels
    auto* x_axis = &(std::get< XDIM >(*ctable));
    size_t x_axis_size = x_axis->GetDimension(0);
    for(size_t i = 0; i < x_axis_size; i++)
    {
        x_axis->at(i) = i * (2.0 * M_PI / (double)x_axis_size);
    }
    // ctable->SetName(std::string("test-table"));
    // ctable->SetUnits(std::string("kg"));

    //now add some labels to the x_axis
    size_t chan_width = 32;
    for(size_t i = 0; i < x_axis_size / chan_width; i++)
    {
        std::stringstream ss;
        ss << "x-chan-" << i;
        std::string xch = "x-channel";
        x_axis->InsertIntervalLabelKeyValue(i * chan_width, (i + 1) * chan_width, xch, ss.str());
    }

    auto* y_axis = &(std::get< YDIM >(*ctable));
    size_t y_axis_size = y_axis->GetDimension(0);
    for(size_t i = 0; i < y_axis_size; i++)
    {
        y_axis->at(i) = i * (2.0 * M_PI / (double)y_axis_size);
    }

    //now add some labels to the y_axis
    chan_width = 64;
    for(size_t i = 0; i < x_axis_size / chan_width; i++)
    {
        std::stringstream ss;
        ss << "y-chan-" << i;
        std::string ych = "y-channel";
        y_axis->InsertIntervalLabelKeyValue(i * chan_width, (i + 1) * chan_width, ych, ss.str());
    }

    auto* z_axis = &(std::get< ZDIM >(*ctable));
    size_t z_axis_size = z_axis->GetDimension(0);
    z_axis->at(0) = std::string("ar");
    z_axis->at(1) = std::string("bg");
    z_axis->at(2) = std::string("cb");

    for(size_t i = 0; i < x_axis_size; i++)
    {
        for(size_t j = 0; j < y_axis_size; j++)
        {
            for(size_t k = 0; k < z_axis_size; k++)
            {
                double value = std::cos(2 * (k + 1) * x_axis->at(i)) * std::sin(2 * (k + 1) * y_axis->at(j));
                (*ctable)(i, j, k) = value;
            }
        }
    }

    visibility_type* ch_bl_data = new visibility_type();

    MHO_ClassIdentityMap cid_map;
    cid_map.AddClassType(*cscalar);
    cid_map.AddClassType(*cvector);
    cid_map.AddClassType(*ctable);
    cid_map.AddClassType(*ch_bl_data);

    if(filename == "")
    {
        filename = "./test-multi-obj.bin";
    }
    std::string index_filename = filename + ".index";

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename, index_filename);

    if(status)
    {
        inter.Write(*cscalar, "scalar1");
        inter.Write(*cvector, "vector2");
        inter.Write(*ctable, "table3");
        inter.Write(*ch_bl_data, "vis");
        inter.Close();
    }
    else
    {
        std::cout << "error opening file" << std::endl;
    }

    inter.Close();

    std::cout << "Keys from object file:" << std::endl;

    //lets extract all of the object keys in the object file just for inspection
    std::vector< MHO_FileKey > ikeys;
    bool result = inter.ExtractFileObjectKeys(filename, ikeys);

    for(auto it = ikeys.begin(); it != ikeys.end(); it++)
    {
        std::cout << "key:" << std::endl;

        std::stringstream ss1;
        ss1 << std::hex << it->fSync;
        std::cout << "sync: " << ss1.str() << std::endl;

        std::stringstream ss2;
        ss2 << std::hex << it->fLabel;
        std::cout << "label: " << ss2.str() << std::endl;

        std::cout << "object uuid: " << it->fObjectId.as_string() << std::endl;
        std::cout << "type uuid: " << it->fTypeId.as_string() << std::endl;
        std::string class_name = cid_map.GetClassNameFromUUID(it->fTypeId);
        std::cout << "class name = " << class_name << std::endl;
        std::cout << "size (bytes): " << it->fSize << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;
    }

    ikeys.clear();
    inter.Close();

    std::cout << "Keys from index file:" << std::endl;

    //lets extract all of the object keys in the index file just for inspection
    //std::vector< MHO_FileKey > ikeys;
    result = inter.ExtractIndexFileObjectKeys(index_filename, ikeys);
    for(auto it = ikeys.begin(); it != ikeys.end(); it++)
    {
        std::cout << "key:" << std::endl;

        std::stringstream ss1;
        ss1 << std::hex << it->fSync;
        std::cout << "sync: " << ss1.str() << std::endl;

        std::stringstream ss2;
        ss2 << std::hex << it->fLabel;
        std::cout << "label: " << ss2.str() << std::endl;

        std::cout << "object uuid: " << it->fObjectId.as_string() << std::endl;
        std::cout << "type uuid: " << it->fTypeId.as_string() << std::endl;
        std::string class_name = cid_map.GetClassNameFromUUID(it->fTypeId);
        std::cout << "class name = " << class_name << std::endl;
        std::cout << "size (bytes): " << it->fSize << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;
    }

    MHO_ScalarContainer< double >* cscalar2 = new MHO_ScalarContainer< double >();
    MHO_VectorContainer< int >* cvector2 = new MHO_VectorContainer< int >();
    MHO_TableContainer< double, axis_pack_test >* ctable2 = new MHO_TableContainer< double, axis_pack_test >(dim);

    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*cscalar2, key);
        inter.Read(*cvector2, key);
        inter.Read(*ctable2, key);
    }
    else
    {
        std::cout << " error opening file to read" << std::endl;
    }

    inter.Close();

    return 0;
}
