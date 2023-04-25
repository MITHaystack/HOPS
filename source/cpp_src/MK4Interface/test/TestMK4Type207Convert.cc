#include "MHO_MK4Type207Converter.hh"
#include <iostream>
#include <stdlib.h>
#include <string>

const int REFANDREMSIZE = 64;

using namespace hops;

int main(int argc, char **argv) {
  struct type_207 my207;

  // create and fill in a type_207 struct with some dummy data
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

  mho_json obj = convertToJSON(my207);
  std::cout << obj.dump(2) << std::endl;

  return 0;
}
