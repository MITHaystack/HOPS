#include <limits>
#include <vector>
#include <map>

#include "MHO_ContainerDefinitions.hh"

#ifndef MBDMXPTS
#define MBDMXPTS 8192
#endif /* MBD_GRID_MAX == MBDMXPTS */

#ifndef MBDMULT
#define MBDMULT 4
#endif /* MBDMULT */
// 2048 == 8192 / 4 == MBDMXPTS / MBDMULT
#define GRID_PTS    (MBDMXPTS / MBDMULT)

// this routine implicitly assumes all frequencies have data
// status.mb_index[MAXFREQ] indicates where in the MBD grid a channel fr sits
// values are 0..GRID_PTS-1 but there is then zero-padding to MBDMXPTS-1

#define BOGUS_MB_INDEX (GRID_PTS * MBDMULT + 1)
// this routine could be rewritten to not use pass, param or status.

namespace hops
{

void FreqSpacing(const channel_axis_type& ch_axis);

}
