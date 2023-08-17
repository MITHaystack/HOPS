#ifndef MHO_ComputeDelayModel_HH__
#define MHO_ComputeDelayModel_HH__

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

namespace hops 
{

class MHO_ComputeDelayModel
{
    public:
        MHO_ComputeDelayModel();
        virtual ~MHO_ComputeDelayModel();
        
        void SetReferenceTime(){};
        
        void SetReferenceStationData(){};
        void SetRemoteStationData(){};
        
    private:
        
        void compute_model();
    
};

}//end of namespace

#endif /* end of include guard: MHO_ComputeDelayModel_HH__ */
