# Try to find allegro 5
#
#  ALLEGRO5_MAIN_FOUND - system has allegro5
#  ALLEGRO5_MAIN_LIBRARIES - Link these to use allegro5
#

SET(ALLEGRO5_MAIN_NAMES ${ALLEGRO5_MAIN_NAMES} allegro_main allegro_main_static liballegro_main liballegro_main_static allegro_main-debug)
FIND_LIBRARY(ALLEGRO5_MAIN_LIBRARY NAMES ${ALLEGRO5_MAIN_NAMES} HINTS ${ALLEGRO5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set ALLEGRO5_MAIN_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ALLEGRO5_MAIN DEFAULT_MSG ALLEGRO5_MAIN_LIBRARY)

IF(ALLEGRO5_MAIN_FOUND)
	SET(ALLEGRO5_MAIN_LIBRARIES ${ALLEGRO5_MAIN_LIBRARY})
	SET(ALLEGRO5_LIBS ${ALLEGRO5_LIBS} ${ALLEGRO5_MAIN_LIBRARIES})
ENDIF(ALLEGRO5_MAIN_FOUND)

MARK_AS_ADVANCED(ALLEGRO5_MAIN_LIBRARY )
