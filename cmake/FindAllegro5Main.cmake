# Try to find allegro 5
#
#  Allegro5Main_FOUND - system has allegro5
#  Allegro5Main_LIBRARIES - Link these to use allegro5
#

SET(Allegro5Main_NAMES ${Allegro5Main_NAMES} allegro_main allegro_main_static liballegro_main liballegro_main_static allegro_main-debug)
FIND_LIBRARY(Allegro5Main_LIBRARY NAMES ${Allegro5Main_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5Main_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5Main DEFAULT_MSG Allegro5Main_LIBRARY)

IF(Allegro5Main_FOUND)
	SET(Allegro5Main_LIBRARIES ${Allegro5Main_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5Main_LIBRARIES})
ENDIF(Allegro5Main_FOUND)

MARK_AS_ADVANCED(Allegro5Main_LIBRARY )
