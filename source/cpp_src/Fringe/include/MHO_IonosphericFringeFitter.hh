#ifndef MHO_IonosphericFringeFitter_HH__
#define MHO_IonosphericFringeFitter_HH__

#include "MHO_FringeFitter.hh"
#include "MHO_Tokenizer.hh"

/*
*File: MHO_IonosphericFringeFitter.hh
*Class: MHO_IonosphericFringeFitter
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: basic single-baseline fringe fitter, no bells or whistles
*/

namespace hops
{

class MHO_IonosphericFringeFitter: public MHO_BasicFringeFitter
{

    public:
        MHO_IonosphericFringeFitter();
        virtual ~MHO_IonosphericFringeFitter();

        //basic run scheme: configure, init, then while(!IsFinished() ){ pre-run, run, post-run }
        // virtual void Configure() override;
        // virtual void Initialize() override;
        virtual void PreRun() override;
        virtual void Run() override;
        virtual void PostRun() override;
        virtual void Finalize() override;
        // virtual bool IsFinished() override;

        // //TODO remove this hack in favor of 'plotting'/'output' visitors
        // mho_json GetPlotData(){return fPlotData;}

    protected:

        // std::vector< std::string > DetermineRequiredPolProducts(std::string polprod);
        // void AddPolProductSummationOperator(std::string& polprod, std::vector< std::string >& pp_vec, mho_json& statements);
        // 
        // void AddDefaultOperatorFormatDef(mho_json& format);
        // void AddDefaultOperators(mho_json& statements);
        // 
        // mho_json fVexInfo;
        // mho_json fControlFormat;
        // mho_json fControlStatements;
        // 
        // //hacks
        // mho_json fDataSelectFormat;
        // mho_json fPlotData;
        // 
        // //utility 
        // MHO_Tokenizer fTokenizer;
};

}//end namespace

#endif /* end of include guard: MHO_IonosphericFringeFitter_HH__ */
