SET(SRC_LIST
  about.c
  allegro_utils.c
  config.c
  disclaimer.c
  intro.c
  level.c
  loading.c
  main.c
  map.c
  menu.c
  moonwalk.c
  pause.c
  timeline.c
  levels/actions.c
  levels/callbacks.c
)

find_package(Allegro5 REQUIRED)
find_package(Allegro5Font REQUIRED)
find_package(Allegro5TTF REQUIRED)
find_package(Allegro5Primitives REQUIRED)
find_package(Allegro5Audio REQUIRED)
find_package(Allegro5ACodec REQUIRED)
find_package(Allegro5Image REQUIRED)
if(APPLE)
  find_package(Allegro5Main)
endif(APPLE)

if(MINGW)
	# resource compilation for MinGW
	add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_SOURCE_DIR} -i${CMAKE_SOURCE_DIR}/data/icons/icon.rc -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
	set(SRC_LIST ${SRC_LIST} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
	set(LINK_FLAGS -Wl,-subsystem,windows)
endif(MINGW)

IF(${PACKAGE_BUILD})
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/superderpy:\$ORIGIN")
ENDIF(${PACKAGE_BUILD})

add_executable(superderpy ${SRC_LIST})

include_directories(${ALLEGRO5_INCLUDE_DIR} ${ALLEGRO5_FONT_INCLUDE_DIR} ${ALLEGRO5_TTF_INCLUDE_DIR} ${ALLEGRO5_PRIMITIVES_INCLUDE_DIR} ${ALLEGRO5_AUDIO_INCLUDE_DIR} ${ALLEGRO5_ACODEC_INCLUDE_DIR} ${ALLEGRO5_IMAGE_INCLUDE_DIR})

target_link_libraries(superderpy ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_MAIN_LIBRARIES} m)

if(ALLEGRO5_MAIN_FOUND)
  include_directories(${ALLEGRO5_MAIN_INCLUDE_DIR})
  target_link_libraries(superderpy ${ALLEGRO5_MAIN_INCLUDE_DIR})
endif(ALLEGRO5_MAIN_FOUND)

install(TARGETS superderpy DESTINATION ${BIN_INSTALL_DIR})