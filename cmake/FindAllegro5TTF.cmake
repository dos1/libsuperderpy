# Try to find allegro 5
#
#  Allegro5TTF_FOUND - system has allegro5
#  Allegro5TTF_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5TTF_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5TTF_INCLUDE_DIR allegro5/allegro_ttf.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5TTF_NAMES ${Allegro5TTF_NAMES} allegro_ttf  liballegro_ttf AllegroTTF-5.2 allegro_ttf-debug allegro_ttf-static allegro_ttf_static liballegro_ttf_static)
FIND_LIBRARY(Allegro5TTF_LIBRARY NAMES ${Allegro5TTF_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5TTF_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5TTF DEFAULT_MSG Allegro5TTF_LIBRARY Allegro5TTF_INCLUDE_DIR)

IF(Allegro5TTF_FOUND)
	SET(Allegro5TTF_LIBRARIES ${Allegro5TTF_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5TTF_LIBRARIES})
ENDIF(Allegro5TTF_FOUND)

MARK_AS_ADVANCED(Allegro5TTF_LIBRARY Allegro5TTF_INCLUDE_DIR )
