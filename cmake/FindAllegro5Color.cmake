# Try to find allegro 5
#
#  Allegro5Color_FOUND - system has allegro5
#  Allegro5Color_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5Color_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5Color_INCLUDE_DIR allegro5/allegro_color.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5Color_NAMES ${Allegro5Color_NAMES} allegro_color liballegro_color AllegroColor-5.2 allegro_color-debug allegro_color-static allegro_color_static liballegro_color_static)
FIND_LIBRARY(Allegro5Color_LIBRARY NAMES ${Allegro5Color_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5Color_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5Color DEFAULT_MSG Allegro5Color_LIBRARY Allegro5Color_INCLUDE_DIR)

IF(Allegro5Color_FOUND)
	SET(Allegro5Color_LIBRARIES ${Allegro5Color_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5Color_LIBRARIES})
ENDIF(Allegro5Color_FOUND)

MARK_AS_ADVANCED(Allegro5Color_LIBRARY Allegro5Color_INCLUDE_DIR )
