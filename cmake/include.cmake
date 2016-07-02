add_definitions(-D_XOPEN_SOURCE=600)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11")
if(APPLE)
    if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
	set(CMAKE_INSTALL_PREFIX "/Applications")
	set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
    endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
endif(APPLE)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/libsuperderpy/cmake ${PROJECT_SOURCE_DIR}/cmake)

include_directories("." "libsuperderpy/src")

if(MINGW)
    # Guess MINGDIR from the value of CMAKE_C_COMPILER if it's not set.
    if("$ENV{MINGDIR}" STREQUAL "")
	string(REGEX REPLACE "/bin/[^/]*$" "" MINGDIR "${CMAKE_C_COMPILER}")
	message(STATUS "Guessed MinGW directory: ${MINGDIR}")
    else("$ENV{MINGDIR}" STREQUAL "")
	file(TO_CMAKE_PATH "$ENV{MINGDIR}" MINGDIR)
	message(STATUS "Using MINGDIR: ${MINGDIR}")
    endif("$ENV{MINGDIR}" STREQUAL "")

    # Search in MINGDIR for headers and libraries.
    set(CMAKE_PREFIX_PATH "${MINGDIR}")

    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -mwindows")

endif(MINGW)

set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/${LIBSUPERDERPY_GAMENAME}:\$ORIGIN/gamestates:\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/lib:\$ORIGIN/bin")


MACRO(register_gamestate name)

    add_library("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" SHARED "${name}.c")

    set_target_properties("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" PROPERTIES PREFIX "")

    target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_COLOR_LIBRARIES} m libsuperderpy)

    if (TARGET libsuperderpy-${LIBSUPERDERPY_GAMENAME})
	target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" libsuperderpy-${LIBSUPERDERPY_GAMENAME})
    ENDIF()

    install(TARGETS "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" DESTINATION ${LIB_INSTALL_DIR})

ENDMACRO()

MACRO(libsuperderpy_copy EXECUTABLE)

    add_custom_command(TARGET "${EXECUTABLE}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "../libsuperderpy/src/libsuperderpy.so" $<TARGET_FILE_DIR:${EXECUTABLE}>)

ENDMACRO()

include(InstallRequiredSystemLibraries)
