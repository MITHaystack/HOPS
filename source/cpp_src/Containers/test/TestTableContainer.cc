#include <iostream>
#include <string>
#include <cmath>
#include <sstream>

#include "MHO_TableContainer.hh"
#include "MHO_Axis.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"

using namespace hops;

#define NDIM 3
#define XDIM 0
#define YDIM 1
#define ZDIM 2
typedef MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double>, MHO_Axis< std::string > > axis_pack_test;
typedef MHO_TableContainer<double, axis_pack_test > test_table_type;


class MHO_NameExtension
{
    public:
        MHO_NameExtension(MHO_ExtensibleElement*){};
        virtual ~MHO_NameExtension(){};
        void SetName(std::string name){fName = name;}
        std::string GetName(){return fName;}

    private:
        std::string fName;
};


class MHO_NameVisitor:
    public MHO_ExtendedElement< MHO_NameExtension >::ExtendedVisitor
{
    public:
        MHO_NameVisitor(){};
        ~MHO_NameVisitor(){};

    public:

        virtual void VisitExtendedElement(MHO_ExtendedElement<MHO_NameExtension>* anElement) override
        {
            std::cout<<"visiting an extended element with name:"<<std::endl;
            std::cout<< anElement->GetName() <<std::endl;
        }
};

int main(int argc, char** argv)
{

    size_t dim[NDIM];
    dim[0] = 256; //x
    dim[1] = 256; //y
    dim[2] = 3; // r,g,b

    test_table_type* test = new test_table_type(dim);


    test->MakeExtension< MHO_NameExtension >()->SetName( std::string("myTest") );

    MHO_NameVisitor myVisitor;
    test->Accept(&myVisitor);

    for(size_t i=0; i<NDIM; i++)
    {
        std::cout<<"dimension @ "<<i<<" ="<<test->GetDimension(i)<<std::endl;
    }

    //set up the axis labels
    auto* x_axis = &(std::get<XDIM>(*test));
    size_t x_axis_size = x_axis->GetDimension(0);
    for(size_t i=0; i<x_axis_size; i++)
    {
        x_axis->at(i) = i*(2.0*M_PI/(double)x_axis_size);
    }

    //now add some labels to the x_axis
    size_t chan_width = 32;
    // for(size_t i=0; i < x_axis_size/chan_width; i++)
    // {
    //     MHO_IntervalLabel label;
    //     label.SetBounds(i*chan_width, (i+1)*chan_width);
    //     std::stringstream ss;
    //     ss << "x-chan-" << i;
    //     label.Insert(std::string("x-channel"), ss.str() );
    //     x_axis->InsertLabel(label);
    // }

    auto* y_axis = &(std::get<YDIM>(*test));
    size_t y_axis_size = y_axis->GetDimension(0);
    for(size_t i=0; i<y_axis_size; i++)
    {
        y_axis->at(i) = i*(2.0*M_PI/(double)y_axis_size);
    }

    //now add some labels to the y_axis
    chan_width = 64;
    // for(size_t i=0; i < x_axis_size/chan_width; i++)
    // {
    //     MHO_IntervalLabel label;
    //     label.SetBounds(i*chan_width, (i+1)*chan_width);
    //     std::stringstream ss;
    //     ss << "y-chan-" << i;
    //     label.Insert(std::string("y-channel"), ss.str() );
    //     y_axis->InsertLabel(label);
    // }

    auto* z_axis = &(std::get<ZDIM>(*test));
    size_t z_axis_size = z_axis->GetDimension(0);
    z_axis->at(0) = "r";
    z_axis->at(1) = "g";
    z_axis->at(2) = "b";

    for(size_t i=0; i<x_axis_size; i++)
    {
        for(size_t j=0; j<y_axis_size; j++)
        {
            for(size_t k=0; k<z_axis_size; k++)
            {
                double value = std::cos( 2*(k+1)*x_axis->at(i) )*std::sin( 2*(k+1)*y_axis->at(j) );
                (*test)(i,j,k) = value;
            }
        }
    }

    //lets find interval associated with some channel names
    // auto labels = x_axis->GetIntervalsWithKeyValue(std::string("x-channel"), std::string("x-chan-5"));
    // size_t xlow;
    // size_t xup;
    // for( auto iter = labels.begin(); iter != labels.end(); iter++)
    // {
    //     std::cout<<"bounds for x-chan-5 are: ["<<iter->GetLowerBound()<<", "<<iter->GetUpperBound()<<") "<<std::endl;
    //     xlow = iter->GetLowerBound();
    //     xup = iter->GetUpperBound();
    // }

    // auto label2 = y_axis->GetFirstIntervalWithKeyValue(std::string("y-channel"), std::string("y-chan-1"));
    // size_t ylow;
    // size_t yup;
    // if( label2.IsValid() )
    // {
    //     ylow = label2.GetLowerBound();
    //     yup = label2.GetUpperBound();
    //     std::cout<<"bounds for y-chan-1 are: ["<<ylow<<", "<<yup<<") "<<std::endl;
    // }


    //zero out values which happen to lie inside x-chan-5 and y-chan-3
    // for(size_t i=xlow; i<xup; i++)
    // {
    //     for(size_t j=ylow; j<yup; j++)
    //     {
    //         for(size_t k=0; k<z_axis_size; k++)
    //         {
    //             (*test)(i,j,k) = 0.0;
    //         }
    //     }
    // }


    std::cout<<"Total serializable size of test data = "<<test->GetSerializedSize()<<std::endl;

    std::string filename = "./test-table.bin";
    std::string index_filename = "./test-table.index";

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename, index_filename);

    if(status)
    {
        std::string shortname = "junk";
        inter.Write(*test, shortname);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    test_table_type* test2 = new test_table_type(dim);

    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*test2, key);
        //std::cout<<"B object label = "<<blabel<<std::endl;
        std::cout<<"Total serializable size of (read-back) test data = "<<test2->GetSerializedSize()<<std::endl;
    }
    else
    {
        std::cout<<" error opening file to read"<<std::endl;
    }

    delete test;
    delete test2;

    return 0;
}
