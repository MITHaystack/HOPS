#ifndef MHO_Operator_HH__
#define MHO_Operator_HH__

#include <string>



namespace hops
{

/*!
*@file  MHO_Operator.hh
*@class  MHO_Operator
*@author  J. Barrett - barrettj@mit.edu
*@date Thu Sep 23 16:03:48 2021 -0400
*@brief
*/

class MHO_Operator
{
    public:
        MHO_Operator();
        virtual ~MHO_Operator();

        virtual bool Initialize() = 0;
        virtual bool Execute() = 0;

        void SetName(std::string name){fName = name;}
        std::string GetName() const {return fName;}

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

#endif /*! end of include guard: MHO_Operator */
