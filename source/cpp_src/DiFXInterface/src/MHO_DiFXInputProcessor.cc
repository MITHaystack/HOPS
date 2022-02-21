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
    //extract the quantities at the top level of the difx input struct 
    ExtractBaseStructQuantities(input);

    //loop over jobs

    //loop over the config
    for(int i=0; i<fD.nConfig; i++)
    {
        json["config"].push_back( ExtractConfigQuantities(i) );
    }

    //difx rules? ignore for now
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
    //there are more items in fD, but most of difx specific 
    //and we probably don't need them for now
}

json 
MHO_DiFXInputProcessor::ExtractConfigQuantities(int n)
{
    DifxConfig* c = &(fD.config[n]);
    json config;
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
    return config;
}



}