#!/bin/bash
verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh

export DATADIR2=`cd $srcdir/vt9105; pwd`
RET_VAL=0
EXP_DIR=$DATADIR2
SCAN_DIR=105-1800
CM42H_DIR=105-1800b
H2M4_DIR=105-1800c
cd $EXP_DIR
mkdir -p ./1111
echo "Running: mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}"
rm -r ./1111/${CM42H_DIR}
mkdir -p ./1111/${CM42H_DIR}
mark42hops -i ./1234/${SCAN_DIR} -o ./1111/${CM42H_DIR}

#test run hops2mark4
hops2mark4 -i ./1111/${CM42H_DIR} -o ./1111/${H2M4_DIR}

#run fourfit3 on the original mark4 data
echo "Running: fourfit3 -m 1 -t -c ./cf_test5 -b GE -P I ./1234/${SCAN_DIR} "
time fourfit3 -m 4 -t -c ./cf_test5 -b GY -P I ./1234/${SCAN_DIR}  set plot_data_dir ./chk89 2>&1 | tee ./ff.out

#run fourfit3 on the re-converted mark4 data
echo "Running: fourfit3 -m 1 -t -c ./cf_test5 -b GE -P I ./1111/${H2M4_DIR} "
time fourfit3 -m 4 -t -c ./cf_test5 -b GY -P I ./1111/${H2M4_DIR}  set plot_data_dir ./chk88 2>&1 | tee ./ff.out

pdd2json.py ./chk89/105-1800-GY-X-Ixy*
#pdd2json.py ./chk88/105-1800c-GE-X-Ixy*
compjsonpdd.py -r 0.0105 ./chk89/105-1800-GY-X-Ixy*.json ./chk88/105-1800c-GY-X-Ixy*

RET_VAL1=$?

export DATADIR3=`cd $srcdir/3741; pwd`
EXP_DIR=$DATADIR3
SCAN_DIR=190-1800a
CM42H_DIR=190-1800a
cd $EXP_DIR
echo "Running: mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}"
rm ./${CM42H_DIR}/*.cor
rm ./${CM42H_DIR}/*.sta
rm ./${CM42H_DIR}/*.frng
rm ./${CM42H_DIR}/*.json
mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}

if [ ! -d "$srcdir/3764" ]; then
  echo "Directory $srcdir/3764 does not exist, will not apply mark42hops in this directory"
  exit $RET_VAL
fi

export DATADIR=`cd $srcdir/3764; pwd`
EXP_DIR=$DATADIR
SCAN_DIR=104-1228
CM42H_DIR=104-1228a
H2M4_DIR=104-1228c
cd $EXP_DIR
echo "Running: mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}"
rm -r ./${CM42H_DIR}
mkdir -p ./${CM42H_DIR}

#test-run mark42hops
mark42hops -i ./${SCAN_DIR} -o ./${CM42H_DIR}

#test run hops2mark4
hops2mark4 -i ./${CM42H_DIR} -o ./${H2M4_DIR}

#run fourfit3 on the original mark4 data
echo "Running: fourfit3 -m 4 -c ./test0.cf -b AS -P RR ./${SCAN_DIR} set plot_data_dir ./chk98 "
time fourfit3 -m 4 -c ./test0.cf -b AS -P RR ./${SCAN_DIR} set plot_data_dir ./chk98 2>&1  | tee ./ff2.out

#run fourfit3 on the re-converted mark4 data
echo "Running: fourfit3 -m 4 -c ./test0.cf -b AS -P RR ./${H2M4_DIR} set plot_data_dir ./chk99 "
time fourfit3 -m 4 -c ./test0.cf -b AS -P RR ./${H2M4_DIR} set plot_data_dir ./chk99 2>&1  | tee ./ff1.out

#pdd2json.py ./chk98/104-1228-AS-B-RR.*
pdd2json.py ./chk99/104-1228c-AS-B-RR.*
compjsonpdd.py -r 0.001 ./chk99/104-1228c-AS-B-RR.*.json ./chk98/104-1228-AS-B-RR.*

RET_VAL2=$?

if [[ $RET_VAL1 -eq 0 && $RET_VAL2 -eq 0 ]]; then
    exit 0
else
    exit 1
fi
