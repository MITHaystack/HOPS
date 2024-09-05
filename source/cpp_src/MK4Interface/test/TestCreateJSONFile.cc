#include <cstdlib>
#include <iostream>
#include <string>
#include "MHO_MK4FringeInterface.hh"

#include "MHO_MK4Type200Converter.hh"
#include "MHO_MK4Type201Converter.hh"
#include "MHO_MK4Type202Converter.hh"
#include "MHO_MK4Type203Converter.hh"
#include "MHO_MK4Type204Converter.hh"
#include "MHO_MK4Type205Converter.hh"
#include "MHO_MK4Type206Converter.hh"
#include "MHO_MK4Type207Converter.hh"
#include "MHO_MK4Type208Converter.hh"
#include "MHO_MK4Type210Converter.hh"
#include "MHO_MK4Type212Converter.hh"

const int NUMBEROFFFITCHAN = 16;
const int NUMBEROFSIDEBANDSANDSBWEIGHTS = 64;
const int REFANDREMSIZE = 64;
const int AMPPHASE = 64;
const int DATASIZE = 1;

using namespace hops;

int main(int argc, char **argv) {

  // NOTE: npm and jsonlint are dependencies for this test to work.
  // Ubuntu: >sudo apt install npm
  // >sudo npm install jsonlint -g

  MHO_MK4FringeInterface mk4FringeInterface;

  std::cout << "Creating and filling type 200 structs..." << std::endl;
  // Create type 200 and fill with data.
  struct type_200 my200;
  my200.record_id[0] = '2';
  my200.record_id[1] = '0';
  my200.record_id[2] = '0';
  my200.version_no[0] = '0';
  my200.version_no[1] = '0';
  strncpy(my200.unused1, "fooo", sizeof(my200.unused1));
  for (int i = 0; i < 10; i++) {
    my200.software_rev[i] =
        1; // I have no idea what this field is actually used for.
  }
  my200.expt_no = 1234;
  strncpy(my200.exper_name, "WYQXVxN4Ci6FAU7by7wQLPOnhGFlPGSx", sizeof(my200.exper_name));
  strncpy(my200.scan_name, "NQxu5N642mOyC49l", sizeof(my200.scan_name));
  strncpy(my200.correlator, "baab", sizeof(my200.correlator));
  my200.scantime.year = 2022;
  my200.scantime.day = 22;
  my200.scantime.hour = 22;
  my200.scantime.minute = 22;
  my200.scantime.second = 22;
  my200.start_offset = 0;
  my200.stop_offset = 1;
  my200.corr_date.year = 2022;
  my200.corr_date.day = 22;
  my200.corr_date.hour = 22;
  my200.corr_date.minute = 22;
  my200.corr_date.second = 22;
  my200.fourfit_date.year = 2022;
  my200.fourfit_date.day = 22;
  my200.fourfit_date.hour = 22;
  my200.fourfit_date.minute = 22;
  my200.fourfit_date.second = 22;
  my200.frt.year = 2022;
  my200.frt.day = 22;
  my200.frt.hour = 22;
  my200.frt.minute = 22;
  my200.frt.second = 22;

  // Create and fill the type 201 data.
  struct type_201 my201;
  my201.record_id[0] = '2';
  my201.record_id[1] = '0';
  my201.record_id[2] = '1';
  my201.version_no[0] = '0';
  my201.version_no[1] = '0';
  strncpy(my201.unused1, "foo", sizeof(my201.unused1));
  strncpy(my201.source, "WYQXVxN4Ci6FAU7by7wQLPOnhGFlPGSx", sizeof(my201.source));
  my201.coord.ra_hrs = 1;
  my201.coord.ra_mins = 1;
  my201.coord.ra_secs = 1;
  my201.coord.dec_degs = 1;
  my201.coord.dec_mins = 1;
  my201.coord.dec_secs = 1;
  my201.epoch = 2;
  for (int i = 0; i < 4; i++) {
    my201.pulsar_phase[i] = 1;
  }
  strncpy(my201.unused2, "bar", sizeof(my201.unused2));
  my201.ra_rate = 2;
  my201.dec_rate = 2;
  my201.pulsar_epoch = 2;
  my201.dispersion = 2;

  // Create and fill the type 202 data.
  struct type_202 my202;
  strncpy(my202.record_id, "202", sizeof(my202.record_id));
  strncpy(my202.version_no, "000", sizeof(my202.version_no));
  strncpy(my202.unused1, "foo", sizeof(my202.unused1));
  strncpy(my202.baseline, "aa", sizeof(my202.baseline));
  strncpy(my202.ref_intl_id, "bb", sizeof(my202.ref_intl_id));
  strncpy(my202.rem_intl_id, "cc", sizeof(my202.rem_intl_id));
  strncpy(my202.ref_name, "abcdefgh", sizeof(my202.ref_name));
  strncpy(my202.rem_name, "jklmnopq", sizeof(my202.rem_name));
  strncpy(my202.ref_tape, "jklmnopq", sizeof(my202.ref_tape));
  strncpy(my202.rem_tape, "jklmnopq", sizeof(my202.rem_tape));
  my202.nlags = 2;
  my202.ref_ypos = 2;
  my202.rem_ypos = 2;
  my202.ref_xpos = 1;
  my202.rem_xpos = 2;
  my202.ref_zpos = 2;
  my202.rem_zpos = 2;
  my202.u = 2;
  my202.v = 2;
  my202.uf = 2;
  my202.vf = 2;
  my202.ref_clock = 2;
  my202.rem_clock = 2;
  my202.ref_clockrate = 2;
  my202.rem_clockrate = 2;
  my202.rem_idelay = 2;
  my202.ref_idelay = 2;
  my202.ref_zdelay = 2;
  my202.rem_zdelay = 2;
  my202.ref_az = 2;
  my202.rem_az = 2;
  my202.ref_elev = 2;
  my202.rem_elev = 2;

  // Create and fill the type 203 data.
  struct type_203 my203;
  strncpy(my203.record_id, "202000000", sizeof(my203.record_id));
  strncpy(my203.version_no, "000000", sizeof(my203.version_no));
  for (int i = 0; i < 512; i++) {
    // initialize all channels except the last one
    if (i < 511) {
      my203.channels[i].index = i;
      my203.channels[i].sample_rate = rand() % 20 + 1;
      my203.channels[i].refsb = 'a' + rand() % 26;
      my203.channels[i].remsb = 'a' + rand() % 26;
      my203.channels[i].rempol = 'a' + rand() % 26;
      my203.channels[i].ref_freq = rand() % 20 + 1 + .5;
      my203.channels[i].rem_freq = rand() % 20 + 1 + .5;
      strncpy(my203.channels[i].ref_chan_id, "foobarfoo", sizeof(my203.channels[i].ref_chan_id));
      strncpy(my203.channels[i].rem_chan_id, "barfoo", sizeof(my203.channels[i].rem_chan_id));
    }
  }

  // Create and fill the type 204 data.
  struct type_204 my204;
  strncpy(my204.record_id, "202", sizeof(my204.record_id));
  strncpy(my204.version_no, "000", sizeof(my204.version_no));
  my204.ff_version[0] = 2;
  my204.ff_version[1] = 2;
  strncpy(my204.platform, "Actually, it's GNU/Linux", sizeof(my204.platform));
  strncpy(my204.control_file, "foo.txt", sizeof(my204.control_file));
  my204.ffcf_date.year = 2022;
  my204.ffcf_date.day = 22;
  my204.ffcf_date.hour = 22;
  my204.ffcf_date.minute = 22;
  my204.ffcf_date.second = 22;
  strncpy(my204.override, "bar", sizeof(my204.override));

  // Create and fille the type 205 data.
  struct type_205 my205;
  strncpy(my205.record_id, "202", sizeof(my205.record_id));
  strncpy(my205.version_no, "000", sizeof(my205.version_no));
  strncpy(my205.unused1, "i'm unused1", sizeof(my205.unused1));
  my205.utc_central.year = 2022;
  my205.utc_central.day = 11;
  my205.utc_central.hour = 16;
  my205.utc_central.minute = 0;
  my205.utc_central.second = 0;
  my205.offset = 2;
  strncpy(my205.ffmode, "a mode", sizeof(my205.ffmode));
  my205.search[0] = 1;
  my205.search[1] = 1;
  my205.search[2] = 1;
  my205.search[3] = 1;
  my205.search[4] = 1;
  my205.search[5] = 1;
  my205.filter[0] = 2;
  my205.filter[1] = 2;
  my205.filter[2] = 2;
  my205.filter[3] = 2;
  my205.filter[4] = 2;
  my205.filter[5] = 2;
  my205.filter[6] = 2;
  my205.filter[7] = 2;
  my205.start.year = 2022;
  my205.start.day = 22;
  my205.start.hour = 22;
  my205.start.minute = 22;
  my205.start.second = 22;
  my205.stop.year = 3033;
  my205.stop.day = 33;
  my205.stop.hour = 33;
  my205.stop.minute = 33;
  my205.stop.second = 33;
  my205.ref_freq = 2;
  for (int i = 0; i < NUMBEROFFFITCHAN; i++) {
    my205.ffit_chan[i].ffit_chan_id = 'a';
    my205.ffit_chan[i].unused = 'u';
    my205.ffit_chan[i].channels[0] = 0;
    my205.ffit_chan[i].channels[1] = 1;
    my205.ffit_chan[i].channels[2] = 2;
    my205.ffit_chan[i].channels[3] = 3;
  }

  // Create and fill the type 206 data.
  struct type_206 my206;
  strncpy(my206.record_id, "202", sizeof(my206.record_id));
  strncpy(my206.version_no, "2", sizeof(my206.version_no));
  strncpy(my206.unused1, "unused...", sizeof(my206.unused1));
  my206.start.year = 3033;
  my206.start.day = 33;
  my206.start.hour = 33;
  my206.start.minute = 33;
  my206.start.second = 33;
  my206.first_ap = 22;
  my206.last_ap = 22;
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.accepted[i].lsb = 4;
    my206.accepted[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.weights[i].lsb = 4;
    my206.weights[i].usb = 4;
  }
  my206.intg_time = 22.22;
  my206.accept_ratio = 22.22;
  my206.discard = 22.22;
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason1[i].lsb = 4;
    my206.reason1[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason2[i].lsb = 4;
    my206.reason2[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason3[i].lsb = 4;
    my206.reason3[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason4[i].lsb = 4;
    my206.reason4[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason5[i].lsb = 4;
    my206.reason5[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason6[i].lsb = 4;
    my206.reason6[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason7[i].lsb = 4;
    my206.reason7[i].usb = 4;
  }
  for (int i = 0; i < NUMBEROFSIDEBANDSANDSBWEIGHTS; i++) {
    my206.reason8[i].lsb = 4;
    my206.reason8[i].usb = 4;
  }
  my206.ratesize = 2;
  my206.mbdsize = 2;
  my206.sbdsize = 2;
  strncpy(my206.unused2, "unuse", sizeof(my206.unused2));

  // Create and fill the type 207 data.
  struct type_207 my207;
  strncpy(my207.record_id, "202", sizeof(my207.record_id));
  strncpy(my207.version_no, "2", sizeof(my207.version_no));
  strncpy(my207.unused1, "unused...", sizeof(my207.unused1));
  my207.pcal_mode = 2;
  my207.unused2 = 2;
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.ref_pcamp[i].lsb = 4;
    my207.ref_pcamp[i].usb = 4;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.rem_pcamp[i].lsb = 4;
    my207.rem_pcamp[i].usb = 4;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.ref_pcphase[i].lsb = 4;
    my207.ref_pcphase[i].usb = 4;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.rem_pcphase[i].lsb = 4;
    my207.rem_pcphase[i].usb = 4;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.ref_pcoffset[i].lsb = 4;
    my207.ref_pcoffset[i].usb = 4;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.rem_pcoffset[i].lsb = 4;
    my207.rem_pcoffset[i].usb = 4;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.ref_pcfreq[i].lsb = 4;
    my207.ref_pcfreq[i].usb = 4;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.rem_pcfreq[i].lsb = 4;
    my207.rem_pcfreq[i].usb = 4;
  }
  my207.ref_pcrate = 2;
  my207.rem_pcrate = 2;
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.ref_errate[i] = 4.44;
  }
  for (int i = 0; i < REFANDREMSIZE; i++) {
    my207.rem_errate[i] = 4.44;
  }

  // Create and fill the type 208 data.
  struct type_208 my208;
  // create a string that is NOT null terminated.
  // use that as a test case with strcat
  // create and fill in a type_208 struct with some dummy data
  strncpy(my208.record_id, "202777777777777777777", sizeof(my208.record_id));
  strncpy(my208.version_no, "2", sizeof(my208.version_no));
  strncpy(my208.unused1, "unused...", sizeof(my208.unused1));
  my208.quality = 'q';
  my208.errcode = 'e';
  strncpy(my208.tape_qcode, "xxxxxx", sizeof(my208.tape_qcode));
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

  // Create and fill the type 210 data.
  struct type_210 my210;
  strncpy(my210.record_id, "202", sizeof(my210.record_id));
  strncpy(my210.version_no, "2", sizeof(my210.version_no));
  strncpy(my210.unused1, "unused...", sizeof(my210.unused1));
  for (int i = 0; i < AMPPHASE; i++) {
    my210.amp_phas[i].ampl = 22.22;
    my210.amp_phas[i].phase = 22.22;
  }

  // Create and fill the type 212 data.
  struct type_212 my212;
  strncpy(my212.record_id, "202", sizeof(my212.record_id));
  strncpy(my212.version_no, "2", sizeof(my212.version_no));
  my212.unused = 'u';
  my212.nap = 2;
  my212.first_ap = 2;
  my212.channel = 25;
  my212.sbd_chan = 2;
  strncpy(my212.unused2, "unused2...", sizeof(my212.unused2));
  for (int i = 0; i < DATASIZE; i++) {
    my212.data[i].amp = 22.22;
    my212.data[i].phase = 22.22;
    my212.data[i].weight = 22.22;
  }

  std::cout << "Structs created and filled!" << std::endl;
  std::cout << "Converting to JSON..." << std::endl;
  mk4FringeInterface.ExportFringeFilesToJSON(my200, my201, my202, my203, my204, my205, my206,
                                              my207, my208, my210, my212);

  std::string homeDir = getenv("HOME"); 
  std::string filePath = homeDir+"/type-200s-dump.json";
  std::string foo = "ls "+filePath;
  const char* lsCommand = foo.c_str();

  // Check if JSON file exists.
  if (system(lsCommand) == 2) {
    std::cout << "Error: type-200s-dump.json not found." << std::endl;
    exit(1);
  }

  // Check if JSON file was encoded correctly.
  std::string commandString = "jsonlint -q "+filePath+" 2> error.txt";
  const char* command = commandString.c_str();
  if (system(command) == 0){
    std::cout << "Test passed!" << std::endl;
  }
  else if (system(command) == 1) {
    std::cout << "Error: JSON error in type-200s-dump.json. See error.txt." << std::endl;
  }
  else {
    std::cout << "OS Error: Check that you have jsonlint installed via npm." << std::endl;
  }

   return 0;
}
