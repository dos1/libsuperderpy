# Try to find allegro 5
#
#  Allegro5Audio_FOUND - system has allegro5
#  Allegro5Audio_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5Audio_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5Audio_INCLUDE_DIR allegro5/allegro_audio.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5Audio_NAMES ${Allegro5Audio_NAMES} allegro_audio liballegro_audio AllegroAudio-5.2 allegro_audio-debug allegro_audio-static liballegro_audio_static allegro_audio_static)
FIND_LIBRARY(Allegro5Audio_LIBRARY NAMES ${Allegro5Audio_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5Audio_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5Audio DEFAULT_MSG Allegro5Audio_LIBRARY Allegro5Audio_INCLUDE_DIR)

IF(Allegro5Audio_FOUND)
	SET(Allegro5Audio_LIBRARIES ${Allegro5Audio_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5Audio_LIBRARIES})
ENDIF(Allegro5Audio_FOUND)

MARK_AS_ADVANCED(Allegro5Audio_LIBRARY Allegro5Audio_INCLUDE_DIR )
