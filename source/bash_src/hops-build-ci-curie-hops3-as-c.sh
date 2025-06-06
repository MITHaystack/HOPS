#!/bin/bash
source $HOME/.bash_profile
if [ x"${HOPS_CI_DIR}" == "x" ];
then
    echo "HOPS CI directory not set."
else
    START_TIME=$( date )
    #cd to the CI directory and make a fresh clone of the repository master
    cd $HOPS_CI_DIR/../
    rm -rf $HOPS_CI_DIR
    mkdir -p $HOPS_CI_DIR
    git clone -c "core.sshCommand=ssh -i ${HOME}/.ssh/curie_ci_id_ed25519" git@github.mit.edu:barrettj/hops-git.git
    cd $HOPS_CI_DIR
    CURRENT_REV=$( git rev-parse --short HEAD )

    #mkdir build directory and run cmake with the following options
    echo "Runing build in HOPS CI directory: ${HOPS_CI_DIR}"
    if [ -d "$HOPS_CI_DIR/build" ];
    then
        rm -r "$HOPS_CI_DIR/build"
        mkdir -p "$HOPS_CI_DIR/build"
    else
        mkdir -p "$HOPS_CI_DIR/build"
    fi

    #source the DiFX environment script so it is in our path
    source $DIFX_BUILD_DIR/setup.bash

    #build
    cd "$HOPS_CI_DIR/build"
    CONFIG_LOG=$HOPS_CI_DIR/../config-${CURRENT_REV}.log
    BUILD_LOG=$HOPS_CI_DIR/../build-${CURRENT_REV}.log
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_DEBUG_MSG=ON -DHOPS3_USE_CXX=OFF -DHOPS_ENABLE_TEST=ON -DHOPS_USE_DIFXIO=ON -DHOPS_USE_FFTW3=ON -DHOPS_USE_OPENCL=ON -DHOPS_USE_PYBIND11=ON $HOPS_CI_DIR | tee $CONFIG_LOG
    make -j12 install | tee $BUILD_LOG
    source $HOPS_CI_DIR/x86_64-4.00/bin/hops.bash

    #test
    TEST_RUN_FILE=$HOPS_CI_DIR/../test-runner-${CURRENT_REV}.log
    make test | tee $TEST_RUN_FILE

    #copy the test log
    TEST_LOG=$HOPS_CI_DIR/../test-output-${CURRENT_REV}.log
    cp $HOPS_CI_DIR/build/Testing/Temporary/LastTest.log $TEST_LOG

    END_TIME=$( date )

    #e-mail out the log
    #echo "HOPS4 cmake build test start: $START_TIME, end $END_TIME" $'\n \n' "$( cat $TEST_RUN_FILE)" | mailx  -A $CONFIG_LOG -A $BUILD_LOG -A $TEST_LOG -s "HOPS4 build test results - $CURRENT_REV" barrettj@mit.edu
    echo "HOPS4 cmake build (HOPS3 as C) test start: $START_TIME, end $END_TIME" $'\n \n' "Log files in $CONFIG_LOG, $BUILD_LOG, and $TEST_LOG" $'\n \n' "$( cat $TEST_RUN_FILE)" | mailx   -s "HOPS4 build test results - $CURRENT_REV" barrettj@mit.edu



    #clean up
    cd "$HOPS_CI_DIR"
    rm -r "$HOPS_CI_DIR/build"
    rm -r "$HOPS_CI_DIR/x86_64-4.00"

fi
