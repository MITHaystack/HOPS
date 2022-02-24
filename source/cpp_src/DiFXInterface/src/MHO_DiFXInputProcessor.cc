#include "MHO_DiFXInputProcessor.hh"

namespace hops 
{


MHO_DiFXInputProcessor::MHO_DiFXInputProcessor():
    fDInput(nullptr)
{};

MHO_DiFXInputProcessor::~MHO_DiFXInputProcessor()
{
    if(fDInput){deleteDifxInput(fDInput);}
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
	// DifxSpacecraft	*spacecraft;	/* optional table */
	// DifxPulsar	*pulsar;	/* optional table */
	// DifxPhasedArray	*phasedarray;	/* optional table */

    //extract the quantities at the top level of the difx input struct 
    ExtractBaseStructQuantities(input);

    //loop over jobs -- do we need these?

    //loop over the config
    for(int i=0; i<fD.nConfig; i++)
    {
        input["config"].push_back( ExtractConfigQuantities(i) );
    }

    //difx rules? -- ignore for now

    //loop over the freqs 
    for(int i=0; i<fD.nFreq; i++)
    {
        input["freq"].push_back( ExtractFreqQuantities(i) );
    }

    //freqSet? --seems to be DiFX2Fits specific, ignore for now 

    //loop over antennas 
    for(int i=0; i<fD.nAntenna; i++)
    {
        input["antenna"].push_back( ExtractAntennaQuantities(i) );
    }


}

void 
MHO_DiFXInputProcessor::ExtractBaseStructQuantities(json& input)
{
    //just copy these struct quantities in
    input["fracSecondStartTime"] = fD.fracSecondStartTime;
    input["mjdStart"] = fD.mjdStart;
    input["mjdStop"] = fD.mjdStop;
    input["refFreq"] = fD.refFreq;
    input["nAntenna"] = fD.nAntenna;
    input["nConfig"] = fD.nConfig;
    input["nRule"] = fD.nRule;
    input["nFreq"] = fD.nFreq;
    input["nFreqSet"] = fD.nFreqSet;
    input["nScan"] = fD.nScan;
    input["nSource"] = fD.nSource;
    input["nEOP"] = fD.nEOP;
    input["nFlag"] = fD.nFlag;
    input["nDatastream"] = fD.nDatastream;
    input["nBaseline"] = fD.nBaseline;
    input["nSpacecraft"] = fD.nSpacecraft;
    input["nPulsar"] = nPulsar;
    input["nPhasedArray"] = nPhasedArray;
    input["nJob"] = nJob;
    input["quantBits"] = fD.quantBits;
    //there are more items in fD, but most are difx specific 
    //and we probably don't need them for now
}

json 
MHO_DiFXInputProcessor::ExtractConfigQuantities(int n)
{
    DifxConfig* c = &(fD.config[n]);
    json config;

    if(c != nullptr)
    {
        config["name"] = std::string(c->name, DIFXIO_NAME_LENGTH);
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
    DifxFreq* f = &(fD.freq[n]);
    json freq;
    if(f != nullptr)
    {
        freq["freq"] = f->freq;
        freq["bw"] = f->bw;
        freq["sideband"] = std::string(f->sideband,1);
        freq["nChan"] = f->nChan; //num spectral points 
        freq["specAvg"] = f->specAvg; //not clear we need this 
        freq["overSamp"] = f->overSamp;
        freq["decimation"] = f->decimation;
        freq["nTone"] = f->nTone;
        std::vector<int> tone_ids
        for(int i=0; i<f->nTone; i++)
        {
            tone_ids.push_back(f->tone[i]);
        }
        freq["tone"] = tone_ids;
        freq["rxName"] = std::string(f->rxName, DIFXIO_RX_NAME_LENGTH);
    }
    return freq;
}



json 
MHO_DiFXInputProcessor::ExtractAntennaQuantities(int n)
{

    //from difx_antenna.c
    // /* These names must match what calcserver expects */
    // const char antennaMountTypeNames[][MAX_ANTENNA_MOUNT_NAME_LENGTH] =
    // {
    // 	"AZEL",
    // 	"EQUA",
    // 	"SPACE",	/* spacecraft */
    // 	"XYEW",
    // 	"NASR",		/* note: this will correctly fall back to AZEL in calcserver */
    // 	"NASL",		/* note: this will correctly fall back to AZEL in calcserver */
    // 	"XYNS",		/* note: no FITS-IDI support */
    // 	"OTHER"		/* don't expect the right parallactic angle or delay model! */
    // };
    // 
    // /* These names must match what VEX expects */
    // const char antennaSiteTypeNames[][MAX_ANTENNA_SITE_NAME_LENGTH] =
    // {
    // 	"fixed",
    // 	"earth_orbit",
    // 	"OTHER"
    // };
    // 


    // from difx_input.c
    // /* keep this current with antennaMountTypeNames in difx_antenna.c */
    // /* Note that the numbering scheme is based on the FITS-IDI defs, but with XYNS added at end */
    // /* See AIPS memo 114 for the list of mount types */
    // enum AntennaMountType
    // {
    // 	AntennaMountAltAz = 0,
    // 	AntennaMountEquatorial = 1,
    // 	AntennaMountOrbiting = 2,	/* note: uncertain calc support */
    // 	AntennaMountXYEW = 3,		/* Hobart is the prime example */
    // 	AntennaMountNasmythR = 4,	/* note: in calcserver, falls back to azel as is appropriate */
    // 	AntennaMountNasmythL = 5,	/* note: in calcserver, falls back to azel as is appropriate */
    // 	AntennaMountXYNS = 6,		/* note: no FITS-IDI/AIPS support */
    // 	AntennaMountOther = 7,		/* set to this if different from the others */
    // 	NumAntennaMounts		/* must remain as last entry */
    // };
    // 
    // /* keep this current with antennaSiteTypeNames in difx_antenna.c */
    // enum AntennaSiteType
    // {
    // 	AntennaSiteFixed = 0,
    // 	AntennaSiteEarth_Orbiting = 1,
    // 	AntennaSiteOther = 2,
    // 	NumAntennaSiteTypes		/* must remain as last entry */
    // };
    // 
    // extern const char antennaSiteTypeNames[][MAX_ANTENNA_SITE_NAME_LENGTH];
    // 
    // enum OutputFormatType
    // {
    // 	OutputFormatDIFX = 0,
    // 	OutputFormatASCII = 1,
    // 	NumOutputFormat			/* must remain as last entry */
    // };
    // 
    // extern const char antennaMountTypeNames[][MAX_ANTENNA_MOUNT_NAME_LENGTH];



    // typedef struct
    // {
    // 	char name[DIFXIO_NAME_LENGTH];		/* null terminated */
    // 	int origId;		/* antennaId before a sort */
    // 	double clockrefmjd;	/* Reference time for clock polynomial */
    // 	int clockorder;		/* Polynomial order of the clock model */
    // 	double clockcoeff[MAX_MODEL_ORDER+1];	/* clock polynomial coefficients (us, us/s, us/s^2... */
    // 	enum AntennaMountType mount;
    // 	enum AntennaSiteType siteType;
    // 	double offset[3];	/* axis offset, (m) */
    // 	double X, Y, Z;		/* telescope position, (m) */
    // 	double dX, dY, dZ;	/* telescope position derivative, (m/s) */
    // 	int spacecraftId;	/* -1 if not a spacecraft */
    // 	char shelf[DIFXIO_SHELF_LENGTH];  /* shelf location of module; really this should not be here! */
    // } DifxAntenna;


    DifxFreq* a = &(fD.antenna[n]);
    json ant
    if(a != nullptr)
    {
        
    }
    return ant;
}

}