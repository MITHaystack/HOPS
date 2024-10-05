#ifndef MHO_BasicFringeFitter_HH__
#define MHO_BasicFringeFitter_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeFitter.hh"

#include "MHO_SingleSidebandNormFX.hh"
#include "MHO_NormFX.hh"

#include "MHO_InterpolateFringePeak.hh"
#include "MHO_MBDelaySearch.hh"

namespace hops
{

#ifdef HOPS_USE_CUDA
    #include "MHO_MBDelaySearchCUDA.hh"
    #define MBD_SEARCH_TYPE MHO_MBDelaySearchCUDA
#else
    #define MBD_SEARCH_TYPE MHO_MBDelaySearch
#endif

// #ifdef NORMFX_USE_EXTRA_PADDING
// using normfx_type = MHO_NormFXExtraPadding; //8x padding like legacy implementation
// #else
// using normfx_type = MHO_NormFX; //this is the default
// #endif




/*!
 *@file MHO_BasicFringeFitter.hh
 *@class MHO_BasicFringeFitter
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 15:37:46 2023 -0400
 *@brief basic single-baseline fringe fitter, no bells or whistles
 */

class MHO_BasicFringeFitter: public MHO_FringeFitter
{

    public:
        MHO_BasicFringeFitter(MHO_FringeData* data);
        virtual ~MHO_BasicFringeFitter();

        //basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }
        virtual void Configure() override;
        virtual void Initialize() override;
        virtual void PreRun() override;
        virtual void Run() override;
        virtual void PostRun() override;
        virtual void Finalize() override;
        virtual bool IsFinished() override;

    protected:

        //main work functions, operators and works space for basic fringe search function
        void coarse_fringe_search(bool set_windows = true);
        void interpolate_peak();

        bool ContainsMixedSideband(visibility_type* vis);

        //operator to transform from frequency to single-band delay space
        MHO_UnaryOperator<visibility_type>* fNormFXOp;
        MHO_NormFX fMSBNormFXOp; //used when there is mixed LSB or USB or double-sideband data
        MHO_SingleSidebandNormFX fSSBNormFXOp; //used when there is only LSB or USB data


        MBD_SEARCH_TYPE fMBDSearch;
        MHO_InterpolateFringePeak fPeakInterpolator;
        visibility_type* vis_data;
        weight_type* wt_data;
        visibility_type* sbd_data;

        //ovex info
        mho_json fVexInfo;
};

} // namespace hops

#endif /*! end of include guard: MHO_BasicFringeFitter_HH__ */
