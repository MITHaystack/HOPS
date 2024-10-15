#ifndef MHO_BasicFringeFitter_HH__
#define MHO_BasicFringeFitter_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeFitter.hh"

#include "MHO_MixedSidebandNormFX.hh"
#include "MHO_NormFX.hh"
#include "MHO_SingleSidebandNormFX.hh"

#include "MHO_InterpolateFringePeak.hh"
#include "MHO_MBDelaySearch.hh"

namespace hops
{

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

        //accept a visitor
        virtual void Accept(MHO_FringeFitterVisitor* visitor) override 
        {
            visitor->Visit(this);
        };

    protected:
        //main work functions, operators and works space for basic fringe search function
        void coarse_fringe_search(bool set_windows = true);
        void interpolate_peak();

        bool ContainsMixedSideband(visibility_type* vis);

        //ptr to the operator to transform vis from frequency to single-band delay space
        //(we switch depending on the type of freq setup)
        MHO_NormFX* fNormFXOp;

        //actually working operators
        MHO_MixedSidebandNormFX fMSBNormFXOp;  //used when there is mixed LSB or USB or double-sideband data
        MHO_SingleSidebandNormFX fSSBNormFXOp; //used when there is only LSB or USB data

        MHO_MBDelaySearch* fMBDSearch;
        MHO_InterpolateFringePeak fPeakInterpolator;
        visibility_type* vis_data;
        weight_type* wt_data;
        visibility_type* sbd_data;

        //ovex info
        mho_json fVexInfo;
};

} // namespace hops

#endif /*! end of include guard: MHO_BasicFringeFitter_HH__ */
