#!/bin/bash

verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/vt9105; pwd`

RET_VAL=0
EXP_DIR=$DATADIR
EXP_NUM=1111
#NOTE: difx2hops must be run from within the difx directory
#This is because any relative paths specified in the difx.input files are
#relative to where the executable is run, not relative to the directory they
#are placed in. This is just a quirk of the difxio library. Normally, absolute
#paths are used, but that doesn't work outside of a correlator environment.
cd $EXP_DIR
difx2hops -e $EXP_NUM -i ./ -c ./station.codes
RET_VAL=$?

SCAN_DIR=$(ls -1 $EXP_DIR/1111/)
if [ "${SCAN_DIR}" == "105-1800" ];
then
    echo "Converted 1 scan: $SCAN_DIR"

    NUM_COREL_FILES=$(ls -1 $EXP_DIR/1111/105-1800/*.cor | wc -l)
    NUM_STATION_FILES=$(ls -1 $EXP_DIR/1111/105-1800/*.sta | wc -l)
    NUM_JSON_FILES=$(ls -1 $EXP_DIR/1111/105-1800/*.json | wc -l)

    if [ "${NUM_COREL_FILES}" == "21" ];
    then
        echo "Found $NUM_COREL_FILES corel files as expected."
    else
        RET_VAL=2
    fi
    if [ "${NUM_STATION_FILES}" == "6" ];
    then
        echo "Found $NUM_STATION_FILES station files as expected."
    else
        RET_VAL=3
    fi
    if [ "${NUM_JSON_FILES}" == "1" ];
    then
        echo "Found $NUM_JSON_FILES json files as expected."
    else
        RET_VAL=4
    fi

else
    RET_VAL=1
fi

[ "${RET_VAL}" != "0" ]


# GE_FILE=$(ls ./GE.*.cor)
# ConvertFileToJSON -f $GE_FILE -d 0 -o ./tmp.json

# grep -Po '"class_uuid":.*?[^\\]",' ./tmp.json
# "class_uuid":"f05838a616aa848562a57d5ace23e8d1",
# "class_uuid":"330c5f9889eaa350f8955c6e709a536c",
# "class_uuid":"a5c26065821b6dc92b06f780f8641d0e",
#
# grep -Po '"class_name":.*?[^\\]",' ./tmp.json
# "class_name":"MHO_TableContainer<float,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>",
# "class_name":"MHO_ObjectTags",
# "class_name":"MHO_TableContainer<std::complex<float>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>",
