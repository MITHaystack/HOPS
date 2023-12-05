#ifndef MHO_MultitonePhaseCalibrationExtractor_HH__
#define MHO_MultitonePhaseCalibrationExtractor_HH__

/*
*File: MHO_MultitonePhaseCalibrationExtractor.hh
*Class: MHO_MultitonePhaseCalibrationExtractor
*Author:
*Email:
*Date:
*Description: Extracts per-channel, per-ap, phase/delay correction from multitone pcal data
*/

#include <cmath>
#include <complex>
#include <vector>
#include <map>
#include <cctype>

#include "MHO_Message.hh"
#include "MHO_Constants.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_TransformingOperator.hh"



namespace hops
{


class MHO_MultitonePhaseCalibrationExtractor: public MHO_TransformingOperator< multitone_pcal_type, pcal_type>
{
    public:

        MHO_MultitonePhaseCalibrationExtractor();
        virtual ~MHO_MultitonePhaseCalibrationExtractor();

        void SetStation(std::string station){fStationCode = station;}; //2-char station code
        void SetStationMk4ID(std::string station_id){fMk4ID = station_id;} //1-char mk4id
        void SetPolarization(const std::string& pol){fPol = pol; make_upper(fPol);};

         //channel label -> pc_phases
        void SetChannelToPCPhaseMap(const std::map< std::string, double >& map){fPCMap = map;};

    protected:

        virtual bool InitializeImpl(const uch_visibility_store_type* in, visibility_store_type* out);
        virtual bool ExecuteImpl(const uch_visibility_store_type* in, visibility_store_type* out);


        // virtual bool InitializeInPlace(visibility_type* in) override;
        // virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;
        //
        // virtual bool ExecuteInPlace(visibility_type* in) override;
        // virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        std::size_t DetermineStationIndex(const visibility_type* in);
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex<double> fImagUnit;
        double fDegToRad;

        //selection
        std::string fStationCode;
        std::string fMk4ID;
        std::string fPol;

        //channel label -> pcal phases
        std::map< std::string, double > fPCMap;

        //keys for tag retrieval and matching
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;
        std::string fChannelLabelKey;
        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;


        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s){ for(char& c : s){c = toupper(c); };
    }


};


}


#endif /* end of include guard: MHO_MultitonePhaseCalibrationExtractor */
