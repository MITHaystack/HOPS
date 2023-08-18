#ifndef MHO_DelayModel_HH__
#define MHO_DelayModel_HH__

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{

class MHO_DelayModel
{
    public:
        MHO_DelayModel();
        virtual ~MHO_DelayModel();
        
        void SetFourfitReferenceTimeVexString(std::string fourfit_reftime_string){fRefTimeString = fourfit_reftime_string;};
        void SetReferenceStationData(const station_coord_type* ref_data){fRefData = ref_data;};
        void SetRemoteStationData(const station_coord_type* rem_data){fRemData = rem_data;};
    
        void SetClockModel(const mho_json& clock_model){fClockModel = clock_model;};


        void compute_model();
    
    private:

        std::string fRefTimeString;
        const station_coord_type* fRefData;
        const station_coord_type* fRemData;

        mho_json fClockModel;

};

}//end of namespace

#endif /* end of include guard: MHO_DelayModel_HH__ */
