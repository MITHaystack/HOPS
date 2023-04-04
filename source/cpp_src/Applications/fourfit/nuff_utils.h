#ifndef NUFF_UTILS_H__
#define NUFF_UTILS_H__

#include "hops_complex.h"
#include "mk4_data.h"

#include "vex.h"
#include "mk4_util.h"
#include "refringe.h"
#include "pass_struct.h"
#include "param_struct.h"

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"

using namespace hops;

visibility_type* extract_visibilities(struct type_pass*);
weight_type* extract_weights(struct type_pass*);
sbd_type* extract_sbd(struct type_pass*);
sbd_type* extract_sbd_v2(struct type_pass* pass, struct type_param* param);

#endif
