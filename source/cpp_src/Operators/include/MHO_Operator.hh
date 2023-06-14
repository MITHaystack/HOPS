#ifndef MHO_Operator_HH__
#define MHO_Operator_HH__

#include <string>

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

        virtual void SetName(std::string name){fName = name;}
        virtual std::string GetName() const {return fName;}

        //allow priority to vary 
        virtual void SetPriority(const double& priority){fPriority = priority;}
        //a higher value for the fPriority field implies a lower priority 
        //for this operator in the order of execution
        virtual double Priority() const {return fPriority;} 

    private:

        std::string fName;
        double fPriority;
};

}

#endif /* end of include guard: MHO_Operator */
