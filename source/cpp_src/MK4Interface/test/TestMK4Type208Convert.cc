#include "MHO_MK4Type208Converter.hh"
#include <iostream>
#include <stdlib.h>
#include <string>

using namespace hops;

int main(int argc, char **argv) {
  struct type_208 my208;

  // create and fill in a type_208 struct with some dummy data
  strcpy(my208.record_id, "202");
  strcpy(my208.version_no, "2");
  strcpy(my208.unused1, "unused...");
  my208.quality = 'q';
  my208.errcode = 'e';
  strcpy(my208.tape_qcode, "xxxxxx");
  my208.adelay = 2;
  my208.arate = 2;
  my208.aaccel = 2;
  my208.tot_mbd = 2;
  my208.tot_sbd = 2;
  my208.tot_rate = 2;
  my208.tot_mbd_ref = 2;
  my208.tot_sbd_ref = 2;
  my208.tot_rate_ref = 2;
  my208.resid_mbd = 2;
  my208.resid_rate = 2.22;
  my208.mbd_error = 2.22;
  my208.sbd_error = 2.22;
  my208.rate_error = 2.22;
  my208.ambiguity = 2.22;
  my208.amplitude = 2.22;
  my208.inc_seg_ampl = 2.22;
  my208.inc_chan_ampl = 2.22;
  my208.snr = 2.22;
  my208.prob_false = 2.22;
  my208.totphase = 2.22;
  my208.totphase_ref = 2.22;
  my208.tec_error = 2.22;

  json obj = convertToJSON(my208);
  std::cout << obj.dump(2) << std::endl;

  return 0;
}
