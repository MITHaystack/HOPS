#include <iostream>
#include <string>

#include "HScalarContainer.hh"


using namespace hops;



class Test: public HScalarContainer< double >
{

    public:
        Test(){}
        virtual ~Test (){}

};

int main(int /*argc*/, char** /*argv*/)
{
    Test* A = new Test();
    double x = 3.14;

    A->SetName("a-test");
    A->SetValue(x);

    std::cout<<A->GetName()<<", "<<A->GetValue()<<std::endl;

    delete A;

    return 0;
}
