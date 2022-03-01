#include "MHO_DiFXInputProcessor.hh"

namespace hops 
{


MHO_DiFXInputProcessor::MHO_DiFXInputProcessor():
    fD(nullptr)
{};

MHO_DiFXInputProcessor::~MHO_DiFXInputProcessor()
{
    if(fD){deleteDifxInput(fD);}
};

void 
MHO_DiFXInputProcessor::LoadDiFXInputFile(std::string filename)
{
    //TODO FIXME - Why does this sometimes fail/segfault for DiFX versions <2.6 
    //when the .threads file is missing??
    fD = loadDifxInput(filename.c_str());
}

void 
MHO_DiFXInputProcessor::ConvertToJSON(json& input)
{

    // DifxJob		*job;
    // DifxConfig	*config;
    // DifxRule        *rule;
    // DifxFreq	*freq;
    // DifxFreqSet	*freqSet;
    // DifxAntenna	*antenna;
    // DifxScan	*scan;		/* assumed in time order */
    // DifxSource	*source;
    // DifxEOP		*eop;		/* assumed one per day, optional */
    // DifxDatastream	*datastream;
    // DifxBaseline    *baseline;
    
    /****** SKIP THESE OPTIONAL TABLES FOR NOW ********************************/
    // DifxSpacecraft	*spacecraft;	/* optional table */
    // DifxPulsar	*pulsar;	/* optional table */
    // DifxPhasedArray	*phasedarray;	/* optional table */
    /**************************************************************************/

    //extract the quantities at the top level of the difx input struct 
    ExtractBaseStructQuantities(input);

    //loop over jobs? -- ignore for now
    // DifxJob		*job;

    //loop over the config
    for(int i=0; i<fD->nConfig; i++)
    {
        input["config"].push_back( ExtractConfigQuantities(i) );
    }

    //difx rules? -- ignore for now

    //loop over the freqs 
    for(int i=0; i<fD->nFreq; i++)
    {
        input["freq"].push_back( ExtractFreqQuantities(i) );
    }

    //freqSet? --seems to be DiFX2Fits specific, ignore for now 

    //loop over antennas 
    for(int i=0; i<fD->nAntenna; i++)
    {
        input["antenna"].push_back( ExtractAntennaQuantities(i) );
    }

    //loop over scans
    for(int i=0; i<fD->nScan; i++)
    {
        input["scan"].push_back( ExtractScanQuantities(i) );
    }

    //loop over sources
    for(int i=0; i<fD->nSource; i++)
    {
        input["source"].push_back( ExtractSourceQuantities(i) );
    }

    //loop over EOPs 
    for(int i=0; i<fD->nSource; i++)
    {
        input["eop"].push_back( ExtractEOPQuantities(i) );
    }

    //loop over datastreams 
    for(int i=0; i<fD->nDatastream; i++)
    {
        input["datastream"].push_back( ExtractDatastreamQuantities(i) );
    }

    //loop over baselines 
    for(int i=0; i<fD->nBaseline; i++)
    {
        input["baseline"].push_back( ExtractBaselineQuantities(i) );
    }
    
}

void 
MHO_DiFXInputProcessor::ExtractBaseStructQuantities(json& input)
{
    //just copy these struct quantities in
    input["fracSecondStartTime"] = fD->fracSecondStartTime;
    input["mjdStart"] = fD->mjdStart;
    input["mjdStop"] = fD->mjdStop;
    input["refFreq"] = fD->refFreq;
    input["nAntenna"] = fD->nAntenna;
    input["nConfig"] = fD->nConfig;
    input["nRule"] = fD->nRule;
    input["nFreq"] = fD->nFreq;
    input["nFreqSet"] = fD->nFreqSet;
    input["nScan"] = fD->nScan;
    input["nSource"] = fD->nSource;
    input["nEOP"] = fD->nEOP;
    input["nFlag"] = fD->nFlag;
    input["nDatastream"] = fD->nDatastream;
    input["nBaseline"] = fD->nBaseline;
    input["nSpacecraft"] = fD->nSpacecraft;
    input["nPulsar"] = fD->nPulsar;
    input["nPhasedArray"] = fD->nPhasedArray;
    input["nJob"] = fD->nJob;
    input["quantBits"] = fD->quantBits;
    //there are more items in fD, but most are difx specific 
    //and we probably don't need them for now
}

json 
MHO_DiFXInputProcessor::ExtractConfigQuantities(int n)
{
    DifxConfig* c = &(fD->config[n]);
    json config;

    if(c != nullptr)
    {
        config["name"] = std::string(c->name, DIFXIO_NAME_LENGTH).c_str();
        config["tInt"] = c->tInt;
        config["subintNS"] = c->subintNS;
        config["fringeRotOrder"] = c->fringeRotOrder;
        config["xmacLength"] = c->xmacLength;
        config["quantBits"] = c->quantBits;
        config["nAntenna"] = c->nAntenna;
        config["nDatastream"] = c->nDatastream;
        config["nBaseline"] = c->nBaseline;

        std::vector<int> datastream_ids;
        for(int i=0; i<c->nDatastream; i++)
        {
            datastream_ids.push_back(c->datastreamId[i]);
        }
        config["datastreamId"] = datastream_ids;

        std::vector<int> baseline_ids;
        for(int i=0; i<c->nBaseline; i++)
        {
            baseline_ids.push_back(c->baselineId[i]);
        }
        config["baselineId"] = baseline_ids;
    }
    return config;
}


json 
MHO_DiFXInputProcessor::ExtractFreqQuantities(int n)
{
    DifxFreq* f = &(fD->freq[n]);
    json freq;
    if(f != nullptr)
    {
        freq["freq"] = f->freq;
        freq["bw"] = f->bw;
        freq["sideband"] = std::string( &(f->sideband),1).c_str();
        freq["nChan"] = f->nChan; //num spectral points 
        freq["specAvg"] = f->specAvg; //not clear we need this 
        freq["overSamp"] = f->overSamp;
        freq["decimation"] = f->decimation;
        freq["nTone"] = f->nTone;
        std::vector<int> tone_ids;
        for(int i=0; i<f->nTone; i++)
        {
            tone_ids.push_back(f->tone[i]);
        }
        freq["tone"] = tone_ids;
        freq["rxName"] = std::string(f->rxName, DIFXIO_RX_NAME_LENGTH).c_str();
    }
    return freq;
}



json 
MHO_DiFXInputProcessor::ExtractAntennaQuantities(int n)
{
    DifxAntenna* a = &(fD->antenna[n]);
    json ant;
    if(a != nullptr)
    {
        ant["name"] = std::string(a->name, DIFXIO_NAME_LENGTH).c_str();
        ant["origId"] = a->origId; //not clear we need this 
        ant["clockrefmjd"] = a->clockrefmjd;
        ant["clockorder"] = a->clockorder;

        for(int i=0; i <= a->clockorder; i++)
        {
            ant["clockcoeff"].push_back(a->clockcoeff[i]);
        }

        //encode a string for the mount type 
        ant["mount"] = GetAntennaMountTypeString(a->mount);
        //encode a string for the site type 
        ant["siteType"] = GetAntennaSiteTypeString(a->siteType);

        std::vector<double> ax_off;
        ax_off.push_back(a->offset[0]);
        ax_off.push_back(a->offset[1]);
        ax_off.push_back(a->offset[2]);
        ant["offset"] = ax_off;

        std::vector<double> pos;
        pos.push_back(a->X);
        pos.push_back(a->Y);
        pos.push_back(a->Z);
        ant["position"] = pos;

        std::vector<double> vel;
        vel.push_back(a->dX);
        vel.push_back(a->dY);
        vel.push_back(a->dZ);
        ant["velocity"] = vel;

        ant["spacecraftId"] = a->spacecraftId;
        //lets leave the 'shelf' parameter out
    }
    return ant;
}


std::string 
MHO_DiFXInputProcessor::GetAntennaMountTypeString(AntennaMountType type)
{
    // extern const char antennaMountTypeNames[][MAX_ANTENNA_MOUNT_NAME_LENGTH];
    std::string mount_type = std::string( antennaMountTypeNames[type], MAX_ANTENNA_MOUNT_NAME_LENGTH).c_str();
    return mount_type;
}

std::string 
MHO_DiFXInputProcessor::GetAntennaSiteTypeString(AntennaSiteType type)
{
    // extern const char antennaSiteTypeNames[][MAX_ANTENNA_SITE_NAME_LENGTH];
    std::string site_type = std::string(antennaSiteTypeNames[type], MAX_ANTENNA_SITE_NAME_LENGTH).c_str();
    return site_type;
}

json 
MHO_DiFXInputProcessor::ExtractScanQuantities(int n)
{
    DifxScan* s = &(fD->scan[n]);
    json scan;
    if(s != nullptr)
    {
        scan["mjdStart"] = s->mjdStart;
        scan["mjdEnd"] = s->mjdEnd;
        scan["startSeconds"] = s->startSeconds;
        scan["durSeconds"] = s->durSeconds;
        scan["identifier"] = std::string(s->identifier, DIFXIO_NAME_LENGTH).c_str();
        scan["obsModeName"] = std::string(s->obsModeName, DIFXIO_NAME_LENGTH).c_str();
        scan["maxNSBetweenUVShifts"] = s->maxNSBetweenUVShifts;
        scan["maxNSBetweenACAvg"] = s->maxNSBetweenACAvg;
        scan["pointingCentreSrc"] = s->pointingCentreSrc;
        scan["nPhaseCentres"] = s->nPhaseCentres;
        for(int i=0; i<s->nPhaseCentres;i++)
        {
            scan["phsCentreSrcs"].push_back(s->phsCentreSrcs[i]);
        }
        //do we need origjobPhsCenterSrcs??...probably not, skip for now

        scan["nAntenna"] = s->nAntenna;
        scan["nPoly"] = s->nPoly;

        //DifxPolyModel ***im;	/* indexed by [ant][src][poly] */
        /* ant is index of antenna in .input file */
        /*   src ranges over [0...nPhaseCentres] */
        /*   poly ranges over [0 .. nPoly-1] */
        /* NOTE : im[ant] can be zero -> no data */

        //TODO FIXME: WE NEED TO RELATE THE UNITS OF THE POLYMODEL SOMEHOW
        std::vector< std::vector< std::vector< json > > > polys;
        for(int i=0; i < s->nAntenna; i++)
        {
            std::vector< std::vector< json > > pj;
            for(int j=0; j <= s->nPhaseCentres; j++)
            {
                std::vector< json > pk;
                for(int k=0; k < s->nPoly; k++)
                {
                    pk.push_back( ExtractDifxPolyModel( &(s->im[i][j][k]) ) );
                }
                pj.push_back(pk);
            }
            polys.push_back(pj);
        }
        scan["DifxPolyModel"] = polys;

        //skip these parameters
        //DifxPolyModelLMExtension ***imLM;	/* Experimental feature; not usually used */
        //DifxPolyModelXYZExtension ***imXYZ;	/* Experimental feature; not usually used */

    }
    return scan;

}


json 
MHO_DiFXInputProcessor::ExtractSourceQuantities(int n)
{
    DifxSource* s = &(fD->source[n]);
    json source;
    if(s != nullptr)
    {
        //how should we store the units of some of these quantities 
        source["ra"] = s->ra; //radians
        source["dec"] = s->dec; //radians
        source["name"] = std::string(s->name, DIFXIO_NAME_LENGTH).c_str();
        source["calCode"] = std::string(s->calCode, DIFXIO_CALCODE_LENGTH).c_str();
        source["qual"] = s->qual;
        source["spacecraftId"] = s->spacecraftId;
        
        //do we need numFitsSourceIds and fitsSourceIds??
        
        source["pmRA"] = s->pmRA; /* arcsec/year */
        source["pmDec"] = s->pmDec; /* arcsec/year */
        source["parallax"] = s->parallax; /* arcsec/year */
        source["pmEpoch"] = s->pmEpoch; //MJD
    }
    return source;
}


json 
MHO_DiFXInputProcessor::ExtractEOPQuantities(int n)
{
    DifxEOP* e = &(fD->eop[n]);
    json eop;
    if(e != nullptr)
    {
        eop["mjd"] = e->mjd;
        eop["tai_utc"] = e->tai_utc;
        eop["ut1_utc"] = e->ut1_utc;
        eop["xPole"] = e->xPole;
        eop["yPole"] = e->yPole;
    }
    return eop;
}



json 
MHO_DiFXInputProcessor::ExtractDatastreamQuantities(int n)
{

    // typedef struct
    // {
    // 	int antennaId;		/* index to D->antenna */
    // 	float tSys;		/* 0.0 for VLBA DiFX */
    // 	char dataFormat[DIFXIO_FORMAT_LENGTH];   /* e.g., VLBA, MKIV, ... */
    // 
    // 	enum SamplingType dataSampling; /* REAL or COMPLEX */
    // 	int nFile;              /* number of files */
    //         char **file;            /* list of files to correlate (if not VSN) */
    // 	char networkPort[DIFXIO_ETH_DEV_SIZE]; /* eVLBI port for this datastream */
    // 	int windowSize;         /* eVLBI TCP window size */
    // 	int quantBits;		/* quantization bits */
    // 	int dataFrameSize;	/* (bytes) size of formatted data frame */
    // 	enum DataSource dataSource;	/* MODULE, FILE, NET, other? */
    // 
    // 	float phaseCalIntervalMHz;/* 0 if no phase cal extraction, otherwise extract every tone and retain tones selected elsewhere */
    // 	float phaseCalBaseMHz;	/* propagated from VEX1.5 but unused for now */
    // 	int tcalFrequency;	/* 0 if no switched power extraction to be done.  =80 for VLBA */
    // 	int nRecTone;     /* number of pcal tones in the *recorded* baseband*/
    // 	int *recToneFreq; /* Frequency of each pcal tone in the *recorded* baseband in MHz */
    // 	int *recToneOut;  /* bool Recorded pcal written out?*/
    // 
    // 	double *clockOffset;	/* (us) [freq] */
    // 	double *clockOffsetDelta; /* (us) [freq] */
    // 	double *phaseOffset;	/* (degrees) [freq] */
    // 	double *freqOffset;	/* Freq offsets for each frequency in Hz */
    // 
    // 	int nRecFreq;		/* number of freqs recorded in this datastream */
    // 	int nRecBand;		/* number of base band channels recorded */
    // 	int *nRecPol;		/* [recfreq] */
    // 	int *recFreqId;		/* [recfreq] index to DifxFreq table */
    // 	int *recBandFreqId;	/* [recband] index to recFreqId[] */
    // 	char *recBandPolName;	/* [recband] Polarization name (R, L, X or Y) */
    // 
    //         int nZoomFreq;		/* number of "zoom" freqs (within recorded freqs) for this datastream */
    // 	int nZoomBand;		/* number of zoom subbands */
    // 	int *nZoomPol;		/* [zoomfreq] */
    // 	int *zoomFreqId;	/* [zoomfreq] index to DifxFreq table */
    // 	int *zoomBandFreqId;	/* [zoomband] index to zoomfreqId[] */
    // 	char *zoomBandPolName;	/* [zoomband] Polarization name (R, L, X or Y) */
    // } DifxDatastream;



    DifxDatastream* d = &(fD->datastream[n]);
    json ds;
    if(d != nullptr)
    {

    }
    return ds;
}


json 
MHO_DiFXInputProcessor::ExtractBaselineQuantities(int n)
{
    // typedef struct
    // {
    // 	int dsA, dsB;		/* indices to datastream table */
    // 	int nFreq;
    // 	int *nPolProd;		/* [freq] */
    // 
    // 	/* note: band in excess of nRecBand are assumed to be zoom bands */
    // 	int **bandA;		/* [freq][productIndex] */
    // 	int **bandB;		/* [freq][productIndex] */
    // } DifxBaseline;

    DifxBaseline* b = &(fD->baseline[n]);
    json base;
    if(b != nullptr)
    {
        base["dsA"] = b->dsA;
        base["dsB"] = b->dsB;
        base["nFreq"] = b->nFreq;
        for(int i=0; i < b->nFreq; i++)
        {
            base["nPolProd"].push_back(b->nPolProd[n]);
        }


        //TODO FIXME: WE NEED TO RELATE THE UNITS SOMEHOW
        std::vector< std::vector< int > > banda;
        std::vector< std::vector< int > > bandb;
        for(int i=0; i < b->nFreq; i++)
        {
            std::vector< int > idxa;
            std::vector< int > idxb;
            for(int j=0; j <= b->nPolProd[i]; j++)
            {
                idxa.push_back(b->bandA[i][j]);
                idxb.push_back(b->bandB[i][j]);
            }
            banda.push_back(idxa);
            bandb.push_back(idxb);
        }
        base["bandA"] = banda;
        base["bandB"] = bandb;
    }
    return base;
}


json 
MHO_DiFXInputProcessor::ExtractDifxPolyModel(DifxPolyModel* m)
{
    // typedef struct
    // {
    //     int mjd;		/* day of start of polynomial validity */
    //     int sec;		/* time (sec) of start of validity */
    //     int order;		/* order of polynomial -> order+1 terms! */
    //     int validDuration;	/* (seconds), from mjd, sec */
    //     double delay[MAX_MODEL_ORDER+1];	/* (us/sec^n); n=[0, order] */
    //     double dry[MAX_MODEL_ORDER+1];		/* (us/sec^n) */
    //     double wet[MAX_MODEL_ORDER+1];		/* (us/sec^n) */
    //     double az[MAX_MODEL_ORDER+1];		/* azimuth (deg) */
    //     double elcorr[MAX_MODEL_ORDER+1];	/* el (corrected for refraction; i.e., the one used for pointing) (deg) */
    //     double elgeom[MAX_MODEL_ORDER+1];	/* el (uncorrected for refraction) (deg) */
    //     double parangle[MAX_MODEL_ORDER+1];	/* parallactic angle (deg) */
    //     double u[MAX_MODEL_ORDER+1];		/* (m/sec^n) */
    //     double v[MAX_MODEL_ORDER+1];		/* (m/sec^n) */
    //     double w[MAX_MODEL_ORDER+1];		/* (m/sec^n) */
    // } DifxPolyModel;

    json poly;
    if(m != nullptr)
    {
        poly["mjd"] = m->mjd;
        poly["sec"] = m->sec;
        poly["order"] = m->order;
        poly["validDuration"] = m->validDuration;
        for(int i=0; i <= m->order; i++)
        {
            poly["delay"].push_back(m->delay[i]);
            poly["dry"].push_back(m->dry[i]);
            poly["wet"].push_back(m->wet[i]);
            poly["az"].push_back(m->az[i]);
            poly["elcorr"].push_back(m->elcorr[i]);
            poly["elgeom"].push_back(m->elgeom[i]);
            poly["parangle"].push_back(m->parangle[i]);
            poly["u"].push_back(m->u[i]);
            poly["v"].push_back(m->v[i]);
            poly["w"].push_back(m->w[i]);
        }
    }
    return poly;
}






}//end namespace