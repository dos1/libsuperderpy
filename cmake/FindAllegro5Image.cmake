# Try to find allegro 5
#
#  Allegro5Image_FOUND - system has allegro5
#  Allegro5Image_INCLUDE_DIR - the allrgo5 include directory
#  Allegro5Image_LIBRARIES - Link these to use allegro5
#

FIND_PATH(Allegro5Image_INCLUDE_DIR allegro5/allegro_image.h HINTS ${Allegro5_INCLUDE_DIR})

SET(Allegro5Image_NAMES ${Allegro5Image_NAMES} allegro_image liballegro_image AllegroImage-5.2 allegro_image-debug allegro_image-static liballegro_image_static allegro_image_static)
FIND_LIBRARY(Allegro5Image_LIBRARY NAMES ${Allegro5Image_NAMES} HINTS ${Allegro5_DIRECTORY})

# handle the QUIETLY and REQUIRED arguments and set Allegro5Image_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Allegro5Image DEFAULT_MSG Allegro5Image_LIBRARY Allegro5Image_INCLUDE_DIR)

IF(Allegro5Image_FOUND)
	SET(Allegro5Image_LIBRARIES ${Allegro5Image_LIBRARY})
	SET(Allegro5_LIBS ${Allegro5_LIBS} ${Allegro5Image_LIBRARIES})
ENDIF(Allegro5Image_FOUND)

MARK_AS_ADVANCED(Allegro5Image_LIBRARY Allegro5Image_INCLUDE_DIR )
