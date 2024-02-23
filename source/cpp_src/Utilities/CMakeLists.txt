hops_require_cpp11()

include_directories(${EXTERN_HLIBS}/date/include)

if(HOPS_USE_PYBIND11)
include_directories(${EXTERN_HLIBS}/)
endif(HOPS_USE_PYBIND11)

#list header files #############################################################
set (UTILITIES_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/picohash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/legacy_hops_date.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Types.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TemplateTypenameDeduction.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Meta.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Constants.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Serializable.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FileStreamer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_UUID.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FileKey.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Units.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Tokenizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TimeStampConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultiTypeMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Interval.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BinaryFileStreamer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_UUIDGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MD5HashGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BinaryFileInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DirectoryInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ExtensibleElement.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JSONHeaderWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IndexLabelInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IntervalLabelInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ParameterStore.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_LegacyRootCodeGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_StationCodeMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Clock.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MPIInterfaceWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_EncodeDecodeValue.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_LockFileHandler.hh
)

#set_source_files_properties(picohash.h PROPERTIES COMPILE_FLAGS -fpermissive)

#list source files #############################################################

set( UTILITIES_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_TimeStampConverter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Tokenizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BinaryFileStreamer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_UUIDGenerator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DirectoryInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_LegacyRootCodeGenerator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_StationCodeMap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_LockFileHandler.cc
)

if (HOPS_USE_MPI)
    list (APPEND UTILITIES_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MPIInterface.hh
    )
    list (APPEND UTILITIES_SOURCEFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MPIInterface.cc
    )
endif(HOPS_USE_MPI)


#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
hops_internal_include_directories(${EXTERN_HLIBS}/json/include)

#compile and install library ###################################################

set(UTILITIES_LIBS MHO_Message date-tz)

add_library (MHO_Utilities SHARED ${UTILITIES_SOURCEFILES})


if(HOPS_USE_MPI)
    message(INFO "building with MPI enabled.")
    target_link_libraries (MHO_Utilities PUBLIC ${UTILITIES_LIBS} MPI::MPI_CXX)
    target_compile_definitions(MHO_Utilities PUBLIC HOPS_USE_MPI)
else(HOPS_USE_MPI)
    target_link_libraries (MHO_Utilities ${UTILITIES_LIBS})
endif(HOPS_USE_MPI)

hops_install_headers (${UTILITIES_HEADERFILES})
hops_install_libraries (MHO_Utilities)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
