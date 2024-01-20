#ifndef MHO_BasicFringeFitter_HH__
#define MHO_BasicFringeFitter_HH__

#include "MHO_FringeFitter.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_NormFX.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_ContainerDefinitions.hh"

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

        std::vector< std::string > DetermineRequiredPolProducts(std::string polprod);
        void AddPolProductSummationOperator(std::string& polprod, std::vector< std::string >& pp_vec, mho_json& statements);

        void AddDefaultOperatorFormatDef(mho_json& format);
        void AddDefaultOperators(mho_json& statements);

        //main work function and operators for basic fringe search function 
        void basic_fringe_search();
        MHO_NormFX fNormFXOp;
        MHO_MBDelaySearch fMBDSearch;
        visibility_type* vis_data;
        weight_type* wt_data;
        visibility_type* sbd_data;


        mho_json fVexInfo;
        mho_json fControlFormat;
        mho_json fControlStatements;

        //hacks
        mho_json fDataSelectFormat;
        mho_json fPlotData;

        //utility 
        MHO_Tokenizer fTokenizer;
};

}//end namespace

#endif /* end of include guard: MHO_BasicFringeFitter_HH__ */
