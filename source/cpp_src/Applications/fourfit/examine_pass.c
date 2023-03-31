#include "hops_complex.h"
#include "mk4_data.h"
#include "pass_struct.h"



//global messaging util
#include "MHO_Message.hh"

//global messaging util
#include "MHO_Message.hh"

//handles reading directories, listing files etc.
#include "MHO_DirectoryInterface.hh"

//needed to read hops files and extract objects
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"


using namespace hops;

void examine_pass(struct type_pass* pass)
{
    printf("EXAMINING PASS_STRUCT, nfreq = %d \n", pass->nfreq);

    msg_info("nufourfit", "test message. " <<eom);
}
