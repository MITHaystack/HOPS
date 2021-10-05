#ifndef MHO_Operator_HH__
#define MHO_Operator_HH__

/*
*@file: MHO_Operator.hh
*@class: MHO_Operator
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

namespace hops
{

class MHO_Operator
{
    public:
        MHO_Operator();
        virtual ~MHO_Operator();

        virtual bool Initialize() = 0;
        virtual bool Execute() = 0;

    private:
};

}

#endif /* end of include guard: MHO_Operator */
