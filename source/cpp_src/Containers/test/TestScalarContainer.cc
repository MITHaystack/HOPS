#include <iostream>
#include <string>

#include "MHO_ScalarContainer.hh"


using namespace hops;



class Test: public MHO_ScalarContainer< double >
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
