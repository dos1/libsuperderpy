if (NOT LIBSUPERDERPY_CONFIG_INCLUDED)

add_definitions(-D_XOPEN_SOURCE=600)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11")

if(APPLE)
    if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
	set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
	set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}.app/Contents/MacOS/")
    endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")

    set(MACOSX_BUNDLE_ICON_FILE ${LIBSUPERDERPY_GAMENAME})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${LIBSUPERDERPY_GAMENAME_PRETTY})

endif(APPLE)

include_directories("libsuperderpy/src")

if(MINGW)
    # Guess MINGDIR from the value of CMAKE_C_COMPILER if it's not set.
    if(NOT MINGDIR)
	if("$ENV{MINGDIR}" STREQUAL "")
	    string(REGEX REPLACE "/bin/[^/]*$" "" MINGDIR "${CMAKE_C_COMPILER}")
	    message(STATUS "Guessed MinGW directory: ${MINGDIR}")
	else("$ENV{MINGDIR}" STREQUAL "")
	    file(TO_CMAKE_PATH "$ENV{MINGDIR}" MINGDIR)
	    message(STATUS "Using MINGDIR: ${MINGDIR}")
	endif("$ENV{MINGDIR}" STREQUAL "")
    endif(NOT MINGDIR)

    # Search in MINGDIR for headers and libraries.
    set(CMAKE_PREFIX_PATH "${MINGDIR}")

endif(MINGW)

set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/${LIBSUPERDERPY_GAMENAME}:\$ORIGIN/gamestates:\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/lib:\$ORIGIN/bin")

find_package(Allegro5 REQUIRED)
find_package(Allegro5Font REQUIRED)
find_package(Allegro5TTF REQUIRED)
find_package(Allegro5Primitives REQUIRED)
find_package(Allegro5Audio REQUIRED)
find_package(Allegro5ACodec REQUIRED)
find_package(Allegro5Image REQUIRED)
find_package(Allegro5Color REQUIRED)
if(APPLE)
    find_package(Allegro5Main)
endif(APPLE)

MACRO(register_gamestate name)

    add_library("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" SHARED "${name}")

    set_target_properties("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" PROPERTIES PREFIX "")

    target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_COLOR_LIBRARIES} m libsuperderpy)

    if (TARGET libsuperderpy-${LIBSUPERDERPY_GAMENAME})
	target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" libsuperderpy-${LIBSUPERDERPY_GAMENAME})
    ENDIF()

    install(TARGETS "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" DESTINATION ${LIB_INSTALL_DIR})

ENDMACRO()

MACRO(libsuperderpy_copy EXECUTABLE)

    if (NOT APPLE)
	add_custom_command(TARGET "${EXECUTABLE}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "../libsuperderpy/src/libsuperderpy${CMAKE_SHARED_LIBRARY_SUFFIX}" $<TARGET_FILE_DIR:${EXECUTABLE}>)
    endif (NOT APPLE)

ENDMACRO()

include(InstallRequiredSystemLibraries)

set(LIBSUPERDERPY_CONFIG_INCLUDED 1)

endif (NOT LIBSUPERDERPY_CONFIG_INCLUDED)
