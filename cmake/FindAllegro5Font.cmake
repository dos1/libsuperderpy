# Try to find allegro 5
#
#  Allegro5Font_FOUND - system has allegro5
#  Allegro5Font_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5Font_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5Font_INCLUDE_DIR allegro5/allegro_font.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5Font_NAMES ${Allegro5Font_NAMES} allegro_font liballegro_font AllegroFont-5.2 allegro_font-debug allegro_font_static liballegro_font_static allegro_font-static)
FIND_LIBRARY(Allegro5Font_LIBRARY NAMES ${Allegro5Font_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5Font_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5Font DEFAULT_MSG Allegro5Font_LIBRARY Allegro5Font_INCLUDE_DIR)

IF(Allegro5Font_FOUND)
	SET(Allegro5Font_LIBRARIES ${Allegro5Font_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5Font_LIBRARIES})
ENDIF(Allegro5Font_FOUND)

MARK_AS_ADVANCED(Allegro5Font_LIBRARY Allegro5Font_INCLUDE_DIR )
