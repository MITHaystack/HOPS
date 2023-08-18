#ifndef LEGACY_HOPS_DATA_HH__
#define LEGACY_HOPS_DATA_HH__

namespace hops
{

//struct compatible with the legacy hops date format
struct legacy_hops_date
{
    short   year;
    short   day;
    short   hour;
    short   minute;
    float   second;
};

}

#endif /* end of include guard: LEGACY_HOPS_DATA_HH__ */


