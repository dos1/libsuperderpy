# Try to find allegro 5
#
#  Allegro5_FOUND - system has allegro5
#  Allegro5_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5_INCLUDE_DIR allegro5/allegro.h)

SET(Allegro5_NAMES ${Allegro5_NAMES} allegro liballegro Allegro-5.2 allegro-debug allegro-static liballegro_static allegro_static)
FIND_LIBRARY(Allegro5_LIBRARY NAMES ${Allegro5_NAMES})

# handle the QUIETLY and REQUIRED arguments and set Allegro5_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5 DEFAULT_MSG Allegro5_LIBRARY Allegro5_INCLUDE_DIR)

IF(Allegro5_FOUND)
	SET(Allegro5_LIBRARIES ${Allegro5_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5_LIBRARIES})
	get_filename_component(Allegro5_DIRECTORY ${Allegro5_LIBRARY} PATH)
ENDIF(Allegro5_FOUND)

MARK_AS_ADVANCED(Allegro5_LIBRARY Allegro5_INCLUDE_DIR Allegro5_DIRECTORY)
