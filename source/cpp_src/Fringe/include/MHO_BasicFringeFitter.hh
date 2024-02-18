#ifndef MHO_BasicFringeFitter_HH__
#define MHO_BasicFringeFitter_HH__

#include "MHO_FringeFitter.hh"

#include "MHO_ContainerDefinitions.hh"

#include "MHO_NormFX.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_InterpolateFringePeak.hh"


/*
*File: MHO_BasicFringeFitter.hh
*Class: MHO_BasicFringeFitter
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: basic single-baseline fringe fitter, no bells or whistles
*/

namespace hops
{

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

        //void AddPolProductSummationOperator(std::string& polprod, std::vector< std::string >& pp_vec, mho_json& statements);
        // void AddDefaultOperatorFormatDef(mho_json& format);
        // void AddDefaultOperators(mho_json& statements);

        //main work functions, operators and works space for basic fringe search function
        void coarse_fringe_search();
        void interpolate_peak();

        MHO_NormFX fNormFXOp;
        MHO_MBDelaySearch fMBDSearch;
        MHO_InterpolateFringePeak fPeakInterpolator;
        visibility_type* vis_data;
        weight_type* wt_data;
        visibility_type* sbd_data;

        //ovex info
        mho_json fVexInfo;





};

}//end namespace

#endif /* end of include guard: MHO_BasicFringeFitter_HH__ */
