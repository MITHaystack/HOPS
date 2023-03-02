#!/bin/sh
if [ x"${HOPS_CI_DIR}" == "x" ];
then
    echo "HOPS CI directory not set."
else
    echo "Runing build in HOPS CI directory: ${HOPS_CI_DIR}"
    START_TIME=$( date )
    GIT_PULL_LOG_FILE=$HOPS_CI_DIR/git-pull-log-$CURRENT_REV.txt
    #cd to the CI directory and check out the repository (master, assume this has already been done once)
    cd $HOPS_CI_DIR
    git pull | tee $GIT_PULL_LOG_FILE
    CURRENT_REV=$( git rev-parse --short HEAD )

    #mkdir build directory and run cmake with the following options 
    if [[ -d "$HOPS_CI_DIR/build" ]];
    then
        rm -r "$HOPS_CI_DIR/build"
        mkdir -p "$HOPS_CI_DIR/build"
    else 
        mkdir -p "$HOPS_CI_DIR/build"
    fi

    #source the DiFX environment script so it is in our path
    . $DIFX_BUILD_DIR/setup.bash

    #build
    cd "$HOPS_CI_DIR/build"
    BUILD_LOG_FILE=$HOPS_CI_DIR/build-log-$CURRENT_REV.txt
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_DEBUG_MSG=ON -DHOPS3_USE_CXX=ON -DHOPS_ENABLE_REMOTE_TEST_DATA=ON -DHOPS_ENABLE_TEST=ON -DHOPS_USE_DIFXIO=ON -DHOPS_USE_FFTW3=ON -DHOPS_USE_OPENCL=ON -DHOPS_USE_PYBIND11=ON ../ | tee $BUILD_LOG_FILE
    make -j12 install | tee $BUILD_LOG_FILE
    . $HOPS_CI_DIR/x86_64-4.00/bin/hops.bash

    #test 
    TEST_RUN_FILE=$HOPS_CI_DIR/test-runner-log-$CURRENT_REV.txt
    make test | tee $TEST_RUN_FILE
    
    #copy the test log 
    TEST_LOG_FILE=$HOPS_CI_DIR/build/Testing/Temporary/LastTest.log

    END_TIME=$( date )

    #e-mail out the log
    echo "HOPS4 cmake build test start: $START_TIME, end $END_TIME " | mailx -A $GIT_PULL_LOG_FILE -A $BUILD_LOG_FILE -A $TEST_RUN_FILE -A $TEST_LOG_FILE -s "HOPS4 build test results - $CURRENT_REV" barrettj@mit.edu
    
    #clean up
    cd "$HOPS_CI_DIR"
    rm -r "$HOPS_CI_DIR/build"
    rm -r "$HOPS_CI_DIR/x86_64-4.00"

fi