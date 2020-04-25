# Try to find allegro 5
#
#  Allegro5Primitives_FOUND - system has allegro5
#  Allegro5Primitives_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5Primitives_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5Primitives_INCLUDE_DIR allegro5/allegro_primitives.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5Primitives_NAMES ${Allegro5Primitives_NAMES} allegro_primitives liballegro_primitives AllegroPrimitives-5.2 allegro_primitives-debug allegro_primitives-static allegro_primitives_static liballegro_primitives_static)
FIND_LIBRARY(Allegro5Primitives_LIBRARY NAMES ${Allegro5Primitives_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5Primitives_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5Primitives DEFAULT_MSG Allegro5Primitives_LIBRARY Allegro5Primitives_INCLUDE_DIR)

IF(Allegro5Primitives_FOUND)
	SET(Allegro5Primitives_LIBRARIES ${Allegro5Primitives_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5Primitives_LIBRARIES})
ENDIF(Allegro5Primitives_FOUND)

MARK_AS_ADVANCED(Allegro5Primitives_LIBRARY Allegro5Primitives_INCLUDE_DIR )
