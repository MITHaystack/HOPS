hops_require_cpp11()

#list header files #############################################################
set (UTILITIES_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/picohash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TemplateTypenameDeduction.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Meta.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Serializable.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FileStreamer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_UUID.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FileKey.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Units.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Timer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Tokenizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TimeStampConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultiTypeMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Interval.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IntervalLabel.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IntervalLabelTree.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BinaryFileStreamer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_UUIDGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MD5HashGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BinaryFileInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ClassIdentityMap.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ObjectTags.hh
)

#set_source_files_properties(picohash.h PROPERTIES COMPILE_FLAGS -fpermissive)

#list source files #############################################################

set( UTILITIES_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Timer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_TimeStampConverter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IntervalLabel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IntervalLabelTree.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BinaryFileStreamer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_UUIDGenerator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ObjectTags.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(UTILITIES_LIBS MHO_Message)

add_library (MHO_Utilities SHARED ${UTILITIES_SOURCEFILES})
target_link_libraries (MHO_Utilities ${UTILITIES_LIBS})

hops_install_headers (${UTILITIES_HEADERFILES})
hops_install_libraries (MHO_Utilities)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
