#!/bin/sh
if [ x"${HOPS_CI_DIR}" == "x" ];
then
    echo "HOPS CI directory not set."
else
    echo "Runing build in HOPS CI directory: ${HOPS_CI_DIR}"
    #cd to the CI directory and check out the repository (master, assume this has already been done once)
    cd $HOPS_CI_DIR
    git pull

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
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_DEBUG_MSG=ON -DHOPS3_USE_CXX=ON -DHOPS_ENABLE_REMOTE_TEST_DATA=ON -DHOPS_ENABLE_TEST=ON -DHOPS_USE_DIFXIO=ON -DHOPS_USE_FFTW3=ON -DHOPS_USE_OPENCL=ON -DHOPS_USE_PYBIND11=ON ../
    make -j6 
    make install
    . $HOPS_CI_DIR/x86_64-4.00/bin/hops.bash

    #test 
    make test
    
    #copy the test log 
    cp $HOPS_CI_DIR/build/Testing/Temporary/LastTest.log $HOPS_CI_DIR
    
    #clean up
    cd "$HOPS_CI_DIR"
    rm -r "$HOPS_CI_DIR/build"
    rm -r "$HOPS_CI_DIR/x86_64-4.00"

fi