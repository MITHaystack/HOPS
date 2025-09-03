#!/bin/bash

#needed to define the following variables in .bashrc 
# HOPS_CICD_KEY
# DIFX_BUILD_DIR
# HOPS_CI_DIR
# HOPS_CACHED_TESTDATA_DIR
# HOPS_CI_MAILER
# HOPS_CI_MAIL_ADDRESS
# HOPS_CI_LOG_DIR

source $HOME/.bashrc
if [ x"${HOPS_CI_DIR}" == "x" ];
then
    echo "HOPS CI directory not set."
else
    START_TIME=$( date )
    
    #cd to the CI working directory and make a fresh clone of the repository master
    cd $HOPS_CI_DIR/../
    git clone https://github.com/MITHaystack/HOPS.git
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
    #clean up any old test-data directory:
    rm -rf "$HOPS_CI_DIR/build/test_data"
    CONFIG_LOG=${HOPS_CI_LOG_DIR}/config-${CURRENT_REV}.log
    BUILD_LOG=${HOPS_CI_LOG_DIR}/build-${CURRENT_REV}.log
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_DEBUG_MSG=ON -DHOPS3_USE_CXX=OFF -DHOPS_ENABLE_TEST=ON -DHOPS_USE_DIFXIO=ON -DHOPS_USE_FFTW3=ON -DHOPS_USE_OPENCL=OFF -DHOPS_USE_PYBIND11=ON $HOPS_CI_DIR | tee $CONFIG_LOG

    make clean
    make -j12 install | tee $BUILD_LOG
    source $HOPS_CI_DIR/x86_64-4.0.0/bin/hops.bash

    #get test data
    if [ x"${HOPS_CACHED_TESTDATA_DIR}" == "x" ];
    then
        $HOPS_CI_DIR/x86_64-4.0.0/bin/testdata_download_all.sh
    else
        cp -r ${HOPS_CACHED_TESTDATA_DIR}/* $HOPS_CI_DIR/x86_64-4.0.0/data/test_data/
    fi

    TEST_RUN_FILE=${HOPS_CI_LOG_DIR}/../test-runner-${CURRENT_REV}.log
    make test | tee $TEST_RUN_FILE

    #copy the test log
    TEST_LOG=${HOPS_CI_LOG_DIR}/test-output-${CURRENT_REV}.log
    cp $HOPS_CI_DIR/build/Testing/Temporary/LastTest.log $TEST_LOG

    END_TIME=$( date )

    #e-mail out the test log
    echo "HOPS4 cmake build start: $START_TIME, end $END_TIME" $'\n \n' "Log files in $CONFIG_LOG, $BUILD_LOG, and $TEST_LOG" $'\n \n' "$( cat $TEST_RUN_FILE)" | ${HOPS_CI_MAILER} -s "HOPS4 build test results - $CURRENT_REV" ${HOPS_CI_MAIL_ADDRESS}


fi
