#include "MHO_MK4JSONDateConverter.hh"
#include <iostream>
#include "mk4_typedefs.h"

namespace hops {

   mho_json convertDateToJSON(const date &t){
      return {
          {"year", t.year}, 
          {"day", t.day},
          {"hour", t.hour},
          {"minute", t.minute},
          {"second", t.second}
      };
   }
}
