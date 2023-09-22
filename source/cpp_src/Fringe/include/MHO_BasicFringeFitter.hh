#ifndef MHO_BasicFringeFitter_HH__
#define MHO_BasicFringeFitter_HH__

#include "MHO_FringeFitter.hh"

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
        MHO_BasicFringeFitter();
        virtual ~MHO_BasicFringeFitter();

        //basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }
        virtual void Configure() override;
        virtual void Initialize() override;
        virtual void PreRun() override;
        virtual void Run() override;
        virtual void PostRun() override;
        virtual void Finalize() override;
        virtual bool IsFinished() override;

        //TODO remove this hack in favor of 'plotting'/'output' visitors
        mho_json GetPlotData(){return fPlotData;}

    protected:

        mho_json fVexInfo;
        mho_json fControlFormat;

        int count;

        //hacks
        mho_json fDataSelectFormat;
        mho_json fPlotData;
};

}//end namespace

#endif /* end of include guard: MHO_BasicFringeFitter_HH__ */
