#ifndef MHO_ScanStructWrapper_HH__
#define MHO_ScanStructWrapper_HH__

#include "MHO_JSONOutputObject.hh"
#include <cmath>

extern "C"
{
    #include "ovex.h"
}


namespace hops
{

class MHO_ScanStructWrapper: public MHO_JSONOutputObject
{
    public:

        MHO_ScanStructWrapper(scan_struct aScan)
        {
            fScanStruct = aScan;
            fScanStruct.st = new station_struct[aScan.nst];
            for(int i=0; i<fScanStruct.nst; i++)
            {
                fScanStruct.st[i] = aScan.st[i];
            }
            fIsOwned = true;
        };

        MHO_ScanStructWrapper():fIsOwned(false){};

        virtual ~MHO_ScanStructWrapper()
        {
            if(fIsOwned)
            {
                delete[] fScanStruct.st;
            }
        };

        MHO_ScanStructWrapper(const MHO_ScanStructWrapper& copyObject)
        {
            fScanStruct = copyObject.fScanStruct;
            fScanStruct.st = new station_struct[fScanStruct.nst];
            for(int i=0; i<fScanStruct.nst; i++)
            {
                fScanStruct.st[i] = copyObject.fScanStruct.st[i];
            }
            fIsOwned = true;
        };

        MHO_ScanStructWrapper& operator=(const MHO_ScanStructWrapper& rhs)
        {
            if(&rhs != this)
            {
                fScanStruct = rhs.fScanStruct;
                fScanStruct.st = new station_struct[fScanStruct.nst];
                for(int i=0; i<fScanStruct.nst; i++)
                {
                    fScanStruct.st[i] = rhs.fScanStruct.st[i];
                }
            }
            return *this;
            fIsOwned = true;
        };

        virtual void DumpToJSON(json& json_obj);

        virtual const char* GetName() const {return "scan_struct";};
        virtual const char* ClassName() const { return "scan_struct"; };

    public:

        bool fIsOwned;
        scan_struct fScanStruct;

};

}

#endif /* end of include guard: MHO_ScanStructWrapper_HH__ */
