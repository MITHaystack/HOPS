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

extern void examine_pass(struct type_pass*, int);
extern void examine_pass_sbd(struct type_pass*, int);
visibility_type* extract_visibilities(struct type_pass*);
weight_type* extract_weights(struct type_pass*);

#endif
