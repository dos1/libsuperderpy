# Try to find allegro 5
#
#  Allegro5ACodec_FOUND - system has allegro5
#  Allegro5ACodec_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5ACodec_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5ACodec_INCLUDE_DIR allegro5/allegro_acodec.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5ACodec_NAMES ${Allegro5ACodec_NAMES} allegro_acodec liballegro_acodec AllegroAcodec-5.2 allegro_acodec-debug allegro_acodec_static liballegro_acodec_static allegro_acodec-static)
FIND_LIBRARY(Allegro5ACodec_LIBRARY NAMES ${Allegro5ACodec_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5ACodec_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5ACodec DEFAULT_MSG Allegro5ACodec_LIBRARY Allegro5ACodec_INCLUDE_DIR)

IF(Allegro5ACodec_FOUND)
	SET(Allegro5ACodec_LIBRARIES ${Allegro5ACodec_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5ACodec_LIBRARIES})
ENDIF(Allegro5ACodec_FOUND)

MARK_AS_ADVANCED(Allegro5ACodec_LIBRARY Allegro5ACodec_INCLUDE_DIR )
