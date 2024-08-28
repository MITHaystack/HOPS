set( HOPS_TEST_LIBS )

set(SOURCE_BASENAMES
    TestCUDA_vector_sum
    TestCUDA_vector_sbp_reduce
    TestCUDA_vector_mbp_reduce
    TestCUDA_noise_stat
    TestCUFFT
    TestCUFFTBatch1D
)

hops_external_include_directories(${CUDA_INCLUDE_DIRS})
hops_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../CUDAModules/include)
list(APPEND HOPS_TEST_LIBS MHO_CUDAModules ${CUDA_LIBRARIES} ${CUDA_CUFFT_LIBRARIES})

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
