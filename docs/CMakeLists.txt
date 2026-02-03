include(FetchContent)

find_package(Doxygen)

message("Fetching doxygen-awesome-css")

FetchContent_Declare(
    doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(doxygen-awesome-css)
 
# Save the location the files were cloned into
# This allows us to get the path to doxygen-awesome.css
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)
 
# Generate the Doxyfile
set(DOXYFILE_IN ./Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(GLOB_RECURSE CRAB_DOXYGEN_ALL_SOURCES ${PROJECT_SOURCE_DIR}/include/crab/*.hpp)
set(CRAB_DOXYGEN_SOURCES "")

foreach(f ${CRAB_DOXYGEN_ALL_SOURCES})
        if(NOT f MATCHES ".*/impl/.*")
                list(APPEND CRAB_DOXYGEN_SOURCES ${f})
        endif()
endforeach()


doxygen_add_docs(
        docs
        ${CRAB_DOXYGEN_SOURCES}
        ALL
        COMMENT "Generating Doxygen Output"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        # USE_STAMP_FILE
        CONFIG_FILE ${DOXYFILE_OUT}
)
