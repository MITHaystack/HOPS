#ifndef MHO_MixedPolYShift_HH__
#define MHO_MixedPolYShift_HH__

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
 *@file MHO_MixedPolYShift.hh
 *@class MHO_MixedPolYShift
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 11 01:53:13 PM EDT 2024
 *@brief
 */

/**
 * @brief Class MHO_MixedPolYShift
 */
class MHO_MixedPolYShift: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_MixedPolYShift();
        virtual ~MHO_MixedPolYShift();

        //not called by the builder (yet)...
        //this particular operator feature always uses 90 degrees
        //but perhaps we could allow that to change via this setter
        /**
         * @brief Setter for phase offset
         * 
         * @param offset New phase offset value
         */
        void SetPhaseOffset(const double& offset) { fYPolPhaseOffset = offset; }

    protected:
        /**
         * @brief Initializes in-place for mixed polarization shift.
         * 
         * @param in Input visibility_type pointer.
         * @return True if initialization is successful.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(visibility_type* in) override;
        /**
         * @brief Initializes out-of-place operation for mixed polarization data.
         * 
         * @param in Input visibility_type data pointer
         * @param out Output visibility_type data pointer
         * @return True if initialization is successful
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        /**
         * @brief Applies a phase offset for mixed polarization data in-place.
         * 
         * @param in Input visibility_type object to apply phase offset.
         * @return True if successful, false if input is nullptr.
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(visibility_type* in) override;
        /**
         * @brief Copies input visibility data and executes in-place operation.
         * 
         * @param in Const pointer to input visibility_type data.
         * @param out (visibility_type*)
         * @return Boolean result of ExecuteInPlace operation on copied output data.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        /**
         * @brief Checks if a mixed linear/circular polarization product is applicable to a station index.
         * 
         * @param st_idx Index of the station (0 for reference, 1 for remote)
         * @param polprod Polarization product string
         * @return True if applicable, false otherwise
         */
        bool IsApplicable(std::size_t st_idx, std::string polprod);
        /**
         * @brief Function PolMatch
         * 
         * @param station_idx (std::size_t)
         * @param polprod (std::string&)
         * @return Return value (bool)
         */
        bool PolMatch(std::size_t station_idx, std::string& polprod);

        //constants
        std::complex< double > fImagUnit;
        double fDegToRad;

        //selection
        std::string fStationIdentity;

        //phase rotation (90 degrees)
        double fYPolPhaseOffset;

        //keys for tag retrieval
        std::string fStationKey;
        std::string fRemStationKey;
        std::string fRefStationKey;
        std::string fRemStationMk4IDKey;
        std::string fRefStationMk4IDKey;
        std::string fChannelLabelKey;

        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;

        //determines if this is a pol-product of the form (YR, XR, XL, YL, LX, etc.)
        bool IsMixedLinCirc(std::string polprod) const;

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

#endif /*! end of include guard: MHO_MixedPolYShift */
