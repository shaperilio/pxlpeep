cmake_minimum_required(VERSION 3.0.0)
project(pxlpeep)

# From here: https://doc.qt.io/qt-5/cmake-get-started.html#build-a-gui-executable
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# This doesn't actually work; you need to use that set anyway.
message("CMake version: ${CMAKE_VERSION}")
# if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
# endif()

# You must have this set here or find_package will fail.
SET(CMAKE_PREFIX_PATH "/Users/barf/Qt/5.14.2/clang_64")
find_package(Qt5 COMPONENTS Widgets REQUIRED)
if (Qt5Widgets_FOUND)
    message("Qt version ${Qt5Widgets_VERSION}")
    message("Include path: ${Qt5Widgets_INCLUDE_DIRS}")
endif()

# These are from my .pro file
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Xpreprocessor -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp")

link_directories("/usr/local/opt/freeimage/lib"
                 "/usr/local/opt/libomp/lib"
                 # "/usr/local/lib" # appears to be unnecessary
                 )

include_directories("/usr/local/opt/freeimage/include"
                    "/usr/local/opt/libomp/include"
                    )

set(sources
    ImageData.cpp
    ImageData.h
    ImageWindow.cpp
    ImageWindow.h
    MainDialog.cpp
    MainDialog.h
    MainDialog.ui
    pxlpeep.qrc
    main.cpp
    colormapper.h
    definitions.h
)

# How to set the icon for Mac: from https://stackoverflow.com/a/24313061/149506
# NOTE: Don't include the path in MACOSX_BUNDLE_ICON_FILE -- this is
# the property added to Info.plist
set(MACOSX_BUNDLE_ICON_FILE loupe.icns)

# And this part tells CMake where to find and install the file itself
set(myApp_ICON ${CMAKE_CURRENT_SOURCE_DIR}/loupe.icns)
set_source_files_properties(${myApp_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

#add_executable(${CMAKE_PROJECT_NAME} ${sources})

# By making this a MACOSX_BUNDLE you get anti-aliased shit. See:
# https://stackoverflow.com/questions/45048882/how-can-i-set-nshighresolutioncapable-in-info-plist-via-cmake 
add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE ${myApp_ICON} ${sources})
# And this so called template doesn't exist on my system:
# https://cmake.org/cmake/help/v3.17/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html

# This has to be after add_executable
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    Qt5::Widgets
    freeimage
    omp
    )

# Thanks for not telling me this here: https://cmake.org/cmake/help/v3.7/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
# But this is how you actually set which plist template to use, and it has to come after the target_link_libraries.
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/template.plist")