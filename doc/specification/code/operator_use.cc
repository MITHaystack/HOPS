//prexisting pointers to a particular data container
//data_type* array1;
//data_type* array2;

//storage for an ordered list of operators
std::vector< MHO_Operator* > operators;

//construct and configure two operations, and insert in list
MHO_ScalarMultiply mult;
mult.SetFactor(2);
mult.SetArgs(array1);

MHO_SubSample sub;
sub.SetDimensionStride(0, 4);
sub.SetArgs(array1, array2);

operators.push_back(&mult);
operators.push_back(&sub);

//chained initialization of the operators
for(auto it = operators.begin(); it != operators.end(); it++)
{
    it->Initialize();
}

{ //some possible outer loop here...

    //chained execution of the operators
    for(auto it = operators.begin(); it != operators.end(); it++)
    {
        it->Execute();
    }

}
//array2 now contains the contents of array1, multiplied by 2
//but sampled only every 4th element in the 0-th dimension
