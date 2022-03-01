#include "MHO_MK4Type200Converter.hh"
#define TYPE_200_FILES_PATH "../../../c_src/dfio/include"

// struct type_200
//     {
//     char                record_id[3];           /* Standard 3-digit id */
//     char                version_no[2];          /* Standard 2-digit version # */
//     char                unused1[3];             /* Reserved space */
//     short               software_rev[10];       /* Revision levels for online progs */
//     int                 expt_no;                /* Experiment number */
//     char                exper_name[32];         /* Observing program name */
//     char                scan_name[32];          /* Scan label from OVEX */
//     char                correlator[8];          /* Correlator identification */
//     struct date         scantime;               /* Scan time to 1 second */
//     int                 start_offset;           /* Nom. bline start rel. to scantime (s) */
//     int                 stop_offset;            /* Nom. bline stop rel. to scantime (s) */
//     struct date         corr_date;              /* Time of correlation */
//     struct date         fourfit_date;           /* Time of fourfit processing */
//     struct date         frt;                    /* Fourfit reference time */
//     };


namespace hops
{

//boiler-plate constructor/destructors
MHO_MKType200Converter::MHO_MKType200Converter():
    fPtr(nullptr) //initialize out type_200 pointer to null
{};

MHO_MKType200Converter::~MHO_MKType200Converter(){};

void MHO_MKType200Converter::SetType200(type_200* type200ptr) {
    fPtr = type200ptr;
};

void MHO_MKType200Converter::ConvertToJSON() {
    //first we make sure the JSON object is empty by clearing it 
    fJSON.clear();

    //better check that our type_200 pointer is not null 
    if(fPtr != nullptr)
    {
        //go ahead and do the import of the struct members

        //first one is a string (converted from a char array of length 3)
        fJSON["record_id"] = std::string(fPtr->record_id, 3); 

        //do the rest here... (skip the dates for now)
    }
}

json MHO_MKType200Converter::GetJSON() {
    return fJSON;
}



}
