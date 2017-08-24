# Try to find allegro 5
#
#  ALLEGRO5_FONT_FOUND - system has allegro5
#  ALLEGRO5_FONT_INCLUDE_DIR - the allrgo5 include directory
#  ALLEGRO5_FONT_LIBRARIES - Link these to use allegro5
#

FIND_PATH(ALLEGRO5_FONT_INCLUDE_DIR allegro5/allegro_font.h HINTS ${ALLEGRO5_INCLUDE_DIR})

SET(ALLEGRO5_FONT_NAMES ${ALLEGRO5_FONT_NAMES} allegro_font allegro_font_static liballegro_font liballegro_font_static AllegroFont-5.2 allegro_font-debug)
FIND_LIBRARY(ALLEGRO5_FONT_LIBRARY NAMES ${ALLEGRO5_FONT_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set ALLEGRO5_FONT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ALLEGRO5_FONT DEFAULT_MSG ALLEGRO5_FONT_LIBRARY ALLEGRO5_FONT_INCLUDE_DIR)

IF(ALLEGRO5_FONT_FOUND)
  SET(ALLEGRO5_FONT_LIBRARIES ${ALLEGRO5_FONT_LIBRARY})
  SET(ALLEGRO5_LIBS ${ALLEGRO5_LIBS} ${ALLEGRO5_FONT_LIBRARIES})
ENDIF(ALLEGRO5_FONT_FOUND)

MARK_AS_ADVANCED(ALLEGRO5_FONT_LIBRARY ALLEGRO5_FONT_INCLUDE_DIR )
