#include "MHO_DiFXScanProcessor.hh"

namespace hops 
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor()
{};

MHO_DiFXScanProcessor::~MHO_DiFXScanProcessor()
{};


void 
MHO_DiFXScanProcessor::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
    fFileSet = &fileSet;
    LoadInputFile(); //read .input file and build freq table
    ConvertRootFileObject(); //create the equivalent to the Mk4 'ovex' root file
    ConvertVisibilityFileObjects(); //convert visibilities and data weights 
    ConvertStationFileObjects(); //convert the station splines, and pcal data 
    CleanUp(); //delete workspace and prep for next scan
}


void 
MHO_DiFXScanProcessor::ConvertRootFileObject()
{
    //TODO FILL ME IN ...need to populate the 'ovex' structure that we typically use 
    //then convert that to the json representation (as we do in the Mk4Inteface)
    //Is this strictly necessary? We've already converted the DiFX input information into json 
    //so we could probably skip the ovex step...but we do need to make sure we cover the 
    //same set of information, so filling the ovex structures may be the simplest thing to do for now
}

void 
MHO_DiFXScanProcessor::ConvertVisibilityFileObjects()
{
    //load the visibilities
    if(fFileSet->fVisibilityFileList.size() > 1)
    {
        msg_warn("difx_interface", "more than one Swinburne file present, will only process the first one: " <<fFileSet->fVisibilityFileList[0]<< "." << eom);
    }

    //expectation here is that there is only a single file containing visibility
    //records from every baseline in the scan 
    MHO_DiFXVisibilityProcessor visProcessor;
    visProcessor.SetFilename(fFileSet->fVisibilityFileList[0]);
    visProcessor.ReadDIFXFile(fAllBaselineVisibilities);

    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        it->second.SetDiFXInputData(&fInput);
        it->second.ConstructVisibilityFileObjects();
        it->second.WriteVisibilityObjects(fFileSet->fOutputBaseDirectory);
    }

}

void 
MHO_DiFXScanProcessor::ConvertStationFileObjects()
{
    //first process pcal files (if they exist)
    for(auto it = fFileSet->fPCALFileList.begin(); it != fFileSet->fPCALFileList.end(); it++)
    {
        fPCalProcessor.SetFilename(*it);
        double ap_length = fInput["config"][0]["tInt"]; //config is a list element, grab the first
        fPCalProcessor.SetAccumulationPeriod(ap_length);
        fPCalProcessor.ReadPCalFile();
        fPCalProcessor.Organize();

        std::string station_code = fPCalProcessor.GetStationCode();
        multitone_pcal_type* pcal = fPCalProcessor.GetPCalData()->Clone();
        fStationCode2PCal[station_code] = pcal;
    }


    //DEBUG, lets write out the PCAL stuff 
    for(auto it = fStationCode2PCal.begin(); it != fStationCode2PCal.end(); it++)
    {
        std::string station_code = it->first;
        //construct output file name (eventually figure out how to construct the baseline name)
        std::string root_code = "dummy"; //TODO replace with actual 'root' code
        std::string output_file = fFileSet->fOutputBaseDirectory + "/" + station_code + "." + root_code + ".pcal";

        MHO_BinaryFileInterface inter;
        bool status = inter.OpenToWrite(output_file);
        MHO_ObjectTags tags;

        if(status)
        {
            uint32_t label = 0xFFFFFFFF; //someday make this mean something
            tags.AddObjectUUID(it->second->GetObjectUUID());
            inter.Write(tags, "tags", label);
            inter.Write( *(it->second), "pcal", label);
            inter.Close();
        }
        else
        {
            msg_error("file", "error opening pcal output file: " << output_file << eom);
        }
        inter.Close();
    }



}


void 
MHO_DiFXScanProcessor::CleanUp()
{
    //clear up and reset for next scan
    //now iterate through the pcal map and delete the objects we cloned 
    for(auto it = fStationCode2PCal.begin(); it != fStationCode2PCal.end(); it++)
    {
        multitone_pcal_type* ptr = it->second;
        delete ptr;
    }
    fStationCode2PCal.clear();

    //clear out station coords
    for(auto it = fStationCode2Coords.begin(); it != fStationCode2Coords.end(); it++)
    {
        station_coord_data_type* ptr = it->second;
        delete ptr;
    }
    fStationCode2Coords.clear();

}


void 
MHO_DiFXScanProcessor::LoadInputFile()
{
    //convert the input to json 
    MHO_DiFXInputProcessor input_proc;
    input_proc.LoadDiFXInputFile(fFileSet->fInputFile);
    input_proc.ConvertToJSON(fInput);

    msg_debug("difx_interface", "difx .input file: " << fFileSet->fInputFile <<" converted to json." << eom);
}


void MHO_DiFXScanProcessor::ExtractStationCoords()
{
    //populate fStationCode2Coords with each station present in fInput 
    //(e.g. the station name/codes, coordinates, and delay spline info, etc. for each station)

    std::size_t nAntenna = fInput["scan"][0]["nAntenna"];
    std::size_t phase_center = 0; //TODO FIXME, currently only one phase-center supported

    for(std::size_t n=0; n<nAntenna; n++)
    {
        station_coord_data_type* st_coord = new station_coord_data_type();
        json antenna_poly = fInput["scan"][0]["DifxPolyModel"][n][phase_center];

        std::size_t n_poly = antenna_poly.size(); //aka nsplines in d2m4
        std::size_t n_order = antenna_poly["order"];
        
        st_coord->Resize();

        for(std::size_t p=0; p<=n_order; p++)
        {

        }



    }






        //first thing we have to do is figure out the data dimensions
        //the items stored in the mk4sdata objects are mainly:
        //(1) delay spline polynomial coeff (type_301)
        //(2) phase spline polynomial coeff (type_302)
        //(3) parallatic angle spline coeff (type_303)
        //(4) uvw-coords spline coeff (type_303)
        //(5) phase-cal data (type_309)

        //We need to determine 4 things:
        //the number of channels
        //the number of intervals
        //the number of spline coefficients
    //     fNCoord = NCOORD; //delay, phase, az, el, par-angle, u, v, w
    //     fNCoeffs = NCOEFF; // hard-coded value in the mk4 type_301, 302, 303s for max spline coeff
    //     fNIntervals = 0;
    //     fNChannels = 0;
    //     DetermineDataDimensions();
    // 
    //     std::size_t st_dim[STATION_NDIM] = {fNCoord, fNChannels, fNIntervals, fNCoeffs};
    //     st_data = new station_coord_data_type(st_dim);
    // 
    //     std::get<COORD_AXIS>(*st_data)[0] = std::string("delay");
    //     std::get<COORD_AXIS>(*st_data)[1] = std::string("phase");
    //     std::get<COORD_AXIS>(*st_data)[2] = std::string("azimuth");
    //     std::get<COORD_AXIS>(*st_data)[3] = std::string("elevation");
    //     std::get<COORD_AXIS>(*st_data)[4] = std::string("parallactic_angle");
    //     std::get<COORD_AXIS>(*st_data)[5] = std::string("u");
    //     std::get<COORD_AXIS>(*st_data)[6] = std::string("v");
    //     std::get<COORD_AXIS>(*st_data)[7] = std::string("w");
    // 
    //     //now lets extract the spline data from each channel
    //     for(std::size_t ch=0; ch<fNChannels; ch++)
    //     {
    //         std::string chan_id = std::string(fStation->model[ch].chan_id);
    //         if(chan_id.size() != 0)
    //         {
    //             std::get<CHAN_AXIS>(*st_data)[ch] = chan_id;
    //             for(std::size_t sp=0; sp<fNIntervals; sp++)
    //             {
    //                 std::get<INTERVAL_AXIS>(*st_data)[sp] = sp;
    // 
    //                 type_301* t301 = fStation->model[ch].t301[sp]; //delay
    //                 type_302* t302 = fStation->model[ch].t302[sp]; //phase
    //                 type_303* t303 = fStation->model[ch].t303[sp]; //az,el,par,u,v,w
    //                 if( t301 != nullptr && t302 != nullptr && t302 != nullptr)
    //                 {
    //                     if( t301->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};
    //                     if( t302->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};
    //                     if( t303->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};
    // 
    //                     for(std::size_t cf=0; cf<fNCoeffs; cf++)
    //                     {
    //                         std::get<COEFF_AXIS>(*st_data)[cf] = cf;
    //                         st_data->at(0, ch, sp, cf) = t301->delay_spline[cf];
    //                         st_data->at(1, ch, sp, cf) = t302->phase_spline[cf];
    //                         st_data->at(2, ch, sp, cf) = t303->azimuth[cf];
    //                         st_data->at(3, ch, sp, cf) = t303->elevation[cf];
    //                         st_data->at(4, ch, sp, cf) = t303->parallactic_angle[cf];
    //                         st_data->at(5, ch, sp, cf) = t303->u[cf];
    //                         st_data->at(6, ch, sp, cf) = t303->v[cf];
    //                         st_data->at(7, ch, sp, cf) = t303->w[cf];
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }














    // 
    //     // finish forming type 300 and write it
    // t300.id = (stns+n)->mk4_id;
    // memcpy (t300.intl_id, (stns+n)->intl_name, 2);
    // memcpy (t300.name, (stns+n)->difx_name, 2);
    // t300.name[2] = 0;           // null terminate to form string
    //     // check that model was read in OK
    // if (D->scan[scanId].im == 0)
    // {
    // fprintf (stderr, "ERROR: problem accessing model array\n");
    // fclose(fout);
    // return (-1);
    // }
    // t = (**(D->scan[scanId].im+n))->mjd + (**(D->scan[scanId].im+n))->sec / 86400.0;
    // conv2date (t, &t300.model_start);
    // 
    // t300.model_interval = (float)(**(D->scan[scanId].im+n))->validDuration;
    // t300.nsplines = (short int) D->scan[scanId].nPoly;
    // write_t300 (&t300, fout);
    // 
    //     // construct type 301, 302, and 303's and write them
    //     // loop over channels
    // for (i=0; i<D->nFreq; i++)
    // {
    //     // find matching freq channel
    //     // loop through whole fblock table
    // nf = -1;
    // while (pfb[++nf].stn[0].ant >= 0) // check for end-of-table marker
    // {
    // 
    // for (k=0; k<2; k++)
    // {
    // freq_i =  D->freq[i].freq;
    // bw_i =  D->freq[i].bw;
    // sideband_i =  D->freq[i].sideband;
    //     // zoom bands are identified by their lower edge, as if USB
    // if (pfb[nf].stn[k].zoom && sideband_i == 'L')
    // {
    // sideband_i = 'U';
    // freq_i -= bw_i;
    // }
    //     // check for match to this frequency
    // if (pfb[nf].stn[k].freq     == freq_i
    // && pfb[nf].stn[k].bw       == bw_i
    // && pfb[nf].stn[k].sideband == sideband_i
    // && pfb[nf].stn[k].ant      == n)
    // {
    // strcpy (t301.chan_id, pfb[nf].stn[k].chan_id);
    // strcpy (t302.chan_id, pfb[nf].stn[k].chan_id);
    // strcpy (t303.chan_id, pfb[nf].stn[k].chan_id);
    // break;      // found freq, do double break
    // }
    // }
    // if (k < 2)
    // break;          // 2nd part of double break
    // }
    //     // this freq not in table - skip it
    // if (k == 2)
    // continue;
    //     // loop over polynomial intervals
    // for (j=0; j<D->scan[scanId].nPoly; j++)
    // {
    //     // insert polynomial indices
    // t301.interval = (short int) j;
    // t302.interval = t301.interval;
    // t303.interval = t301.interval;
    //     // units of difx are usec, ff uses sec
    //     // shift clock polynomial to start of model interval
    // deltat = 8.64e4 * ((**(D->scan[scanId].im+n)+j)->mjd - (D->antenna+n)->clockrefmjd)
    //                    + (**(D->scan[scanId].im+n)+j)->sec;
    // nclock = getDifxAntennaShiftedClock (D->antenna+n, deltat, 6, clock);
    //     // difx delay doesn't have clock added in, so
    //     // we must do it here; also apply sign reversal
    //     // for opposite delay convention
    // for (l=0; l<6; l++)
    // {
    // t301.delay_spline[l]
    // = -1.e-6 * (**(D->scan[scanId].im+n)+j)->delay[l];
    // 
    // if (l < nclock) // add in those clock coefficients that are valid
    // t301.delay_spline[l] -= 1e-6 * clock[l];
    // 
    // t302.phase_spline[l] = t301.delay_spline[l] * (D->freq+i)->freq;
    //     // fill t303 with az and el polynomials
    // t303.azimuth[l] = (**(D->scan[scanId].im+n)+j)->az[l];
    // t303.elevation[l] = (**(D->scan[scanId].im+n)+j)->elgeom[l];
    // t303.u[l] = (**(D->scan[scanId].im+n)+j)->u[l];
    // t303.v[l] = (**(D->scan[scanId].im+n)+j)->v[l];
    // t303.w[l] = (**(D->scan[scanId].im+n)+j)->w[l];
    // }
    //     // par. angle from calc program is NYI
    //     // separate loop so l=1 values defined for az & el
    // for (l=0; l<6; l++)
    // {
    // if (l == 0)     // for now, only constant term is non-zero
    // {
    //     // calculate geocentric latitude (rad)
    // geoc_lat = atan2 (D->antenna[n].Z,
    //           sqrt (D->antenna[n].X * D->antenna[n].X
    //               + D->antenna[n].Y * D->antenna[n].Y));
    //     // get declination for this source
    // sourceId = D->scan[scanId].pointingCentreSrc;
    // dec = D->source[sourceId].dec;
    //     // evaluate az & el at midpoint of spline interval
    // el = M_PI / 180.0 * (t303.elevation[0]
    // + 0.5 * t300.model_interval * t303.elevation[1]);
    // az = M_PI / 180.0 * (t303.azimuth[0]
    // + 0.5 * t300.model_interval * t303.azimuth[1]);
    //     // evaluate sin and cos of the local hour angle
    // sha = - cos(el) * sin(az) / cos(dec);
    // cha = (sin(el) - sin(geoc_lat) * sin(dec))
    // / (cos(geoc_lat) * cos(dec));
    //     // approximate (first order in f) conversion
    // geod_lat = atan(1.00674 * tan(geoc_lat));
    //     // finally ready for par. angle
    // t303.parallactic_angle[l] = 180 / M_PI *
    // atan2 (sha, (cos (dec) * tan(geod_lat) - sin(dec) * cha));
    // }
    // else
    // t303.parallactic_angle[l] = 0.0;
    // }
    // 
    // write_t301 (&t301, fout);
    // write_t302 (&t302, fout);
    // write_t303 (&t303, fout);
    // }
    // }
    // 










        // for(int i=0; i <= m->order; i++)
        // {
        //     poly["delay"].push_back(m->delay[i]);
        //     poly["dry"].push_back(m->dry[i]);
        //     poly["wet"].push_back(m->wet[i]);
        //     poly["az"].push_back(m->az[i]);
        //     poly["elcorr"].push_back(m->elcorr[i]);
        //     poly["elgeom"].push_back(m->elgeom[i]);
        //     poly["parangle"].push_back(m->parangle[i]);
        //     poly["u"].push_back(m->u[i]);
        //     poly["v"].push_back(m->v[i]);
        //     poly["w"].push_back(m->w[i]);
        // }





    // char        record_id[3];           /* Standard 3-digit id */
    // char        version_no[2];          /* Standard 2-digit version # */
    // char        unused1[2];             /* Reserved space */
    // U8          SU_number;              /* Station unit, filled by suman */
    // char        id;                     /* 1-char vex letter code */
    // char        intl_id[2];             /* 2-char international id code */
    // char        name[32];               /* Full station name, null-term. */
    // char        unused2;                /* Padding */
    // struct date model_start;            /* Start time for 1st spline */
    // float       model_interval;         /* Spline interval secs (rec time) */
    // short       nsplines;               /* Number of splines in scan */



}


}//end of namespace
