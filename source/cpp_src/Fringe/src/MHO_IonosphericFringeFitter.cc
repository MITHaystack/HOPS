#include "MHO_IonosphericFringeFitter.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//control
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_BasicFringeInfo.hh"
#include "MHO_InitialFringeInfo.hh"
#include "MHO_BasicFringeUtilities.hh"
#include "MHO_FringePlotInfo.hh"
#include "MHO_VexInfoExtractor.hh"

//experimental ion phase correction
#include "MHO_IonosphericPhaseCorrection.hh"
#define ION_EXP



namespace hops
{


MHO_IonosphericFringeFitter::MHO_IonosphericFringeFitter():
    MHO_BasicFringeFitter()
{
    fNdTECSteps = 20;
    fdTECLow = 0.0;
    fdTECHigh = 1.0;
    fdTECStep = (fdTECHigh - fdTECLow)/(double)fNdTECSteps;
    fStepCount = 0;
};

MHO_IonosphericFringeFitter::~MHO_IonosphericFringeFitter(){};

void MHO_IonosphericFringeFitter::PreRun()
{
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( !skipped) //execute if we are not finished and are not skipping
    {
        //apply the ionospheric dTEC to the visibilities for the current grid search point
        #ifdef ION_EXP
        visibility_type* vis_data = fContainerStore.GetObject<visibility_type>(std::string("vis"));
        MHO_IonosphericPhaseCorrection iono;
        std::cout<<"Applying dTEC of: "<<-1.0*fdTECStep*fStepCount<<std::endl;
        //iono.SetDifferentialTEC(-1.0*(fdTECLow + fdTECStep*fStepCount) );
        iono.SetDifferentialTEC(-1.0*fdTECStep);
        iono.SetArgs(vis_data);
        iono.Initialize();
        iono.Execute();
        fStepCount++;
        #endif
        //TODO FILL ME IN -- need to call specified user-scripts here
    }
}

void MHO_IonosphericFringeFitter::Run()
{
    bool status_is_finished = fParameterStore.GetAs<bool>("/status/is_finished");
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( !status_is_finished  && !skipped) //execute if we are not finished and are not skipping
    {
        //execute the basic fringe search algorithm
        MHO_BasicFringeUtilities::basic_fringe_search(&fContainerStore, &fParameterStore);

        if(fStepCount < fNdTECSteps)
        {
            fParameterStore.Set("/status/is_finished", false);
        }
        else 
        {
            //have sampled all grid points, find the solution and finalize
            //calculate the fringe properties
            MHO_BasicFringeUtilities::calculate_fringe_solution_info(&fContainerStore, &fParameterStore, fVexInfo);

            fParameterStore.Set("/status/is_finished", true);
        }
    }
}

void MHO_IonosphericFringeFitter::PostRun()
{
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( !skipped) //execute if we are not finished and are not skipping
    {
        //TODO FILL ME IN -- need to call specified user-scripts here
    }
}


void MHO_IonosphericFringeFitter::Finalize()
{
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO may want to reorg the way this is done

    bool status_is_finished = fParameterStore.GetAs<bool>("/status/is_finished");
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( status_is_finished  && !skipped ) //have to be finished and not-skipped
    {
        fPlotData = MHO_FringePlotInfo::construct_plot_data(&fContainerStore, &fParameterStore, fVexInfo);
        MHO_FringePlotInfo::fill_plot_data(&fParameterStore, fPlotData);
    }
}

}//end namespace
