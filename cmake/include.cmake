add_definitions(-D_XOPEN_SOURCE=600)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11")
if(APPLE)
  if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
    set(CMAKE_INSTALL_PREFIX "/Applications")
    set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
  endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
endif(APPLE)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/libsuperderpy/cmake ${PROJECT_SOURCE_DIR}/cmake)

include_directories(".")

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

SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/${LIBSUPERDERPY_GAMENAME}:\$ORIGIN/gamestates:\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/lib:\$ORIGIN/bin")


MACRO(GAMESTATE name)

        add_library("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" SHARED "${name}.c")

        SET_TARGET_PROPERTIES("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" PROPERTIES PREFIX "")

        target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_COLOR_LIBRARIES} m libsuperderpy)

        install(TARGETS "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" DESTINATION ${LIB_INSTALL_DIR})

        #add_custom_command(TARGET "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}.so" $<TARGET_FILE_DIR:${LIBSUPERDERPY_GAMENAME}>)

ENDMACRO()

MACRO(libsuperderpy_copy EXECUTABLE)

add_custom_command(TARGET "${EXECUTABLE}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "../libsuperderpy/src/libsuperderpy.so" $<TARGET_FILE_DIR:${EXECUTABLE}>)

ENDMACRO()

include(InstallRequiredSystemLibraries)
