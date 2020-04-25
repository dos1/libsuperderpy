# Try to find allegro 5
#
#  Allegro5Video_FOUND - system has allegro5
#  Allegro5Video_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5Video_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5Video_INCLUDE_DIR allegro5/allegro_video.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5Video_NAMES ${Allegro5Video_NAMES} allegro_video liballegro_video AllegroVideo-5.2 allegro_video-debug allegro_video-static allegro_video_static liballegro_video_static)
FIND_LIBRARY(Allegro5Video_LIBRARY NAMES ${Allegro5Video_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5Video_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5Video DEFAULT_MSG Allegro5Video_LIBRARY Allegro5Video_INCLUDE_DIR)

IF(Allegro5Video_FOUND)
	SET(Allegro5Video_LIBRARIES ${Allegro5Video_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5Video_LIBRARIES})
ENDIF(Allegro5Video_FOUND)

MARK_AS_ADVANCED(Allegro5Video_LIBRARY Allegro5Video_INCLUDE_DIR )
