#ifndef MHO_ManualPolDelayCorrection_HH__
#define MHO_ManualPolDelayCorrection_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_ManualPolDelayCorrection.hh
 *@class MHO_ManualPolDelayCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 27 10:36:00 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_ManualPolDelayCorrection
 */
class MHO_ManualPolDelayCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_ManualPolDelayCorrection();
        virtual ~MHO_ManualPolDelayCorrection();

        /**
         * @brief Setter for reference frequency
         * 
         * @param ref_freq New reference frequency value in Hertz
         */
        void SetReferenceFrequency(double ref_freq) { fRefFreq = ref_freq; }

        //treated as follows:
        //1-char => mk4 id
        //2-char => 2char station code
        /**
         * @brief Setter for station identifier
         * 
         * @param station_id mk4 id of type std::string
         */
        void SetStationIdentifier(std::string station_id) { fStationIdentity = station_id; }

        /**
         * @brief Setter for polarization
         * 
         * @param pol Input polarization string
         */
        void SetPolarization(const std::string& pol)
        {
            fPol = pol;
            make_upper(fPol);
        };

        /**
         * @brief Setter for pcdelay offset
         * 
         * @param pc_delay_offset Time offset between two correlated signals
         */
        void SetPCDelayOffset(const double& pc_delay_offset) { fDelayOffset = pc_delay_offset; }

    protected:
        /**
         * @brief Initializes in-place visibility_type pointer.
         * 
         * @param in Pointer to visibility_type for initialization
         * @return True if successful
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place visibility data from input pointer.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean indicating successful initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies manual delay offset and calculates phase correction factors for each channel frequency.
         * 
         * @param in Input visibility data of type visibility_type.
         * @return True if successful, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and executes in-place processing.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean result of ExecuteInPlace operation on copied output.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        /**
         * @brief Checks if manual polarimetric delay correction is applicable for a given station index and visibility type.
         * 
         * @param st_idx Station index of type std::size_t
         * @param in Const reference to visibility_type
         * @return Boolean indicating whether the correction should be applied
         */
        bool IsApplicable(std::size_t st_idx, const visibility_type* in);
        /**
         * @brief Checks if polarization product matches first element in fPol at given station index and converts polprod to uppercase.
         * 
         * @param station_idx Index of the station for which to check the polarization product.
         * @param polprod (std::string&)
         * @return True if polarization product matches first element in fPol at given station index, false otherwise.
         */
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex< double > fImagUnit;
        double fNanoSecToSecond;
        double fMHzToHz;
        double fPi;

        //selection
        std::string fStationIdentity;
        std::string fPol;

        //ref freq and pc delay
        double fRefFreq;
        double fDelayOffset;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;

        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        //minor helper function to make sure all strings are compared as upper-case only
        void make_upper(std::string& s)
        {
            for(char& c : s)
            {
                c = toupper(c);
            };
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_ManualPolDelayCorrection */
