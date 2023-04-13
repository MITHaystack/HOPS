#include "MHO_ControlBlockWrapper.hh"

namespace hops
{

MHO_ControlBlockWrapper::MHO_ControlBlockWrapper(struct c_block* block, mho_json vex_info, std::string baseline)
{
    fControlBlock = block;
    fVexInfo = vex_info; //full copy (TODO -- maybe should just use a reference?)
    fBaseline = baseline;
    if( !(fVexInfo["VEX_rev"] == "ovex") ){ msg_warn("control", "cannot find an ovex object in the vex info" << eom); }
    Initialize();
}

MHO_ControlBlockWrapper::~MHO_ControlBlockWrapper()
{

}



void
MHO_ControlBlockWrapper::Initialize()
{
    DetermineStationInfo();
    ConstructManualPhaseCalOffsets();
    ConstructManualPhaseCalDelayOffsets();
}


double
MHO_ControlBlockWrapper::GetReferenceFrequency()
{
    return fControlBlock->ref_freq;
}


void
MHO_ControlBlockWrapper::DetermineStationInfo()
{
    fRefMk4ID = "";
    fRefMk4ID = "";
    fRefSiteCode = "";
    fRemSiteCode = "";
    fRefSiteName = "";
    fRemSiteCode = "";

    fRefMk4ID.append( &(fBaseline[0]),1);
    fRemMk4ID.append( &(fBaseline[1]),1);

    auto sites = fVexInfo["$SITE"];
    for(auto sit = sites.begin(); sit != sites.end(); sit++)
    {
        if( (*sit)["mk4_site_ID"] == fRefMk4ID )
        {
            fRefSiteCode = (*sit)["site_ID"];
            fRefSiteName = (*sit)["site_name"];
        }

        if( (*sit)["mk4_site_ID"] == fRemMk4ID )
        {
            fRemSiteCode = (*sit)["site_ID"];
            fRemSiteName = (*sit)["site_name"];
        }
    }

    msg_debug("control", "control block associated with baseline: "<< fBaseline<<" with reference site: ("<< fRefMk4ID<<", "<<fRefSiteCode<<", "<<fRefSiteName<<") and remote site: ("<< fRemMk4ID<<", "<<fRemSiteCode<<", "<<fRemSiteName<<")" << eom );

}

void
MHO_ControlBlockWrapper::ConstructManualPhaseCalOffsets()
{
    //construct the pcal array...this is a really ugly on-off testing kludge
    fRefManPcal.Resize(2,MAXFREQ);
    fRemManPcal.Resize(2,MAXFREQ);

    //label the axes - TODO FIXME -- how to distinguish between L,X,H and R,Y,V ???
    std::string pol_arr[2];
    //from parser.c
    // #define LXH 0
    // #define RYV 1
    pol_arr[0] = "X";
    pol_arr[1] = "Y";

    for(unsigned int p=0; p<2; p++)
    {
        std::get<0>(fRefManPcal)(p) = pol_arr[p];
        std::get<0>(fRemManPcal)(p) = pol_arr[p];
    }

    for(int ch=0; ch<MAXFREQ; ch++)
    {
        std::get<1>(fRefManPcal)(ch) = ch;
        std::get<1>(fRemManPcal)(ch) = ch;
    }

    std::complex<double> imag_unit(0.0, 1.0);
    for(unsigned int p=0; p<2; p++)
    {
        for(std::size_t ch=0; ch<MAXFREQ; ch++)
        {
            double ref_ph = fControlBlock->pc_phase[ch][p].ref;
            double rem_ph = fControlBlock->pc_phase[ch][p].rem;
            fRefManPcal(p,ch) = ref_ph;
            fRemManPcal(p,ch) = rem_ph;
            //std::cout<<"chan: "<< ch <<" ref-pc: "<< fControlBlock->pc_phase[ch][p].ref << " rem-pc: " << fControlBlock->pc_phase[ch][p].rem << std::endl;
        }
    }

    fRefManPcal.Insert(std::string("station"), fRefSiteCode );
    fRemManPcal.Insert(std::string("station"), fRemSiteCode );

    fRefManPcal.Insert(std::string("station_mk4id"), fRefMk4ID );
    fRemManPcal.Insert(std::string("station_mk4id"), fRemMk4ID );

    fRefManPcal.Insert(std::string("station_name"), fRefSiteName );
    fRemManPcal.Insert(std::string("station_name"), fRemSiteName );
}


void
MHO_ControlBlockWrapper::ConstructManualPhaseCalDelayOffsets()
{
    //construct the pcal delay array...this is a really ugly on-off testing kludge
    fRefManPcalDelay.Resize(2,MAXFREQ);
    fRemManPcalDelay.Resize(2,MAXFREQ);

    //label the axes - TODO FIXME -- how to distinguish between L,X,H and R,Y,V ???
    std::string pol_arr[2];
    //from parser.c
    // #define LXH 0
    // #define RYV 1
    pol_arr[0] = "X";
    pol_arr[1] = "Y";

    for(unsigned int p=0; p<2; p++)
    {
        std::get<0>(fRefManPcalDelay)(p) = pol_arr[p];
        std::get<0>(fRemManPcalDelay)(p) = pol_arr[p];
    }

    for(int ch=0; ch<MAXFREQ; ch++)
    {
        std::get<1>(fRefManPcalDelay)(ch) = ch;
        std::get<1>(fRemManPcalDelay)(ch) = ch;
    }

    std::complex<double> imag_unit(0.0, 1.0);
    for(unsigned int p=0; p<2; p++)
    {
        for(std::size_t ch=0; ch<MAXFREQ; ch++)
        {
            double ref_ph = fControlBlock->delay_offs_pol[ch][p].ref;
            double rem_ph = fControlBlock->delay_offs_pol[ch][p].rem;
            fRefManPcalDelay(p,ch) = ref_ph;
            fRemManPcalDelay(p,ch) = rem_ph;
            //std::cout<<"chan: "<< ch <<" ref-pc-delay: "<< fControlBlock->delay_offs_pol[ch][p].ref << " rem-pc-delay: " << fControlBlock->delay_offs_pol[ch][p].rem << std::endl;
        }
    }

    fRefManPcalDelay.Insert(std::string("station"), fRefSiteCode );
    fRemManPcalDelay.Insert(std::string("station"), fRemSiteCode );

    fRefManPcalDelay.Insert(std::string("station_mk4id"), fRefMk4ID );
    fRemManPcalDelay.Insert(std::string("station_mk4id"), fRemMk4ID );

    fRefManPcalDelay.Insert(std::string("station_name"), fRefSiteName );
    fRemManPcalDelay.Insert(std::string("station_name"), fRemSiteName );

}


void
MHO_ControlBlockWrapper::DetermineStartStop()
{
    //This logic comes from make_passes.c
        //                             /* Now sort out timerange for this pass */
        //                                 /* Note that cstart/cstop are true times */
        //                                 /* that correspond to the values in the */
        //                                 /* pass structure, which in Mk4 are the */
        //                                 /* same as appear in the param structure */
        //
        // cstart = (p->control.time_span[0] < 0)?      /* kludge: negative t means */
        //     param->start_nom - (double)p->control.time_span[0]: /* relative time */
        //     86400 *(ovex->start_time.day - 1)         /* else it's absolute time */
        //    + 3600 * ovex->start_time.hour
        //    + (double)p->control.time_span[0];
        //
        // cstop  = (p->control.time_span[1] < 0)?
        //     param->stop_nom + (double)p->control.time_span[1]:      /* relative */
        //     86400 *(ovex->start_time.day - 1)
        //    + 3600 * ovex->start_time.hour
        //    + (double)p->control.time_span[1];                        /* absolute */
        //
        // if (cstart < param->start)
        //     cstart = param->start;
        // if (cstop > param->stop)
        //     cstop = param->stop;
        //                                 /* Actual start is beginning of 1st ap */
        //                                 /* .001 to avoid rejection of aps when */
        //                                 /* cstart/cstop right on ap boundary */
        // pstart = param->start;
        // msg ("pstart %10f cstart %10f cstop %10f", 0, pstart, cstart, cstop);
        // if (pstart == cstart)
        //     start_offset = 0;
        // else
        //     start_offset = (int)((cstart - pstart)
        //                         / param->acc_period + .001);
        // stop_offset = (int)((cstop - pstart) / param->acc_period + .001);
        // p->num_ap = stop_offset - start_offset;
        // p->ap_off = start_offset;
        // msg ("num_ap %d ap_off %d", 0, p->num_ap, p->ap_off);
        //                                 /* Autocorrelation? */
        // if ((param->cormode == AUTO_PER_LAG) || (param->cormode == AUTO_GLOBAL))
        //     p->autocorr = TRUE;
        //                                 /* No data, skip this pass */
        // if (p->num_ap == 0) continue;
        //                                 /* True start/stop times */
        // p->start = param->start + (start_offset * param->acc_period);
        // p->stop = param->start + (stop_offset * param->acc_period);
        // p->reftime = param->reftime;


}


//get the start/stop offsets
double
MHO_ControlBlockWrapper::GetStartOffset()
{
    return fControlBlock->time_span[0]; //length of time since start, casts int to double
}

double
MHO_ControlBlockWrapper::GetStopOffset()
{
    return fControlBlock->time_span[1]; //length of time before end, casts int to double
}



}
