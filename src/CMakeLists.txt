SET(SRC_LIST
  utils.c
  config.c
  timeline.c
  gamestate.c
)

SET(EXECUTABLE_SRC_LIST
  main.c
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
  set(EXECUTABLE_SRC_LIST ${EXECUTABLE_SRC_LIST} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
  set(LINK_FLAGS -Wl,-subsystem,windows)
endif(MINGW)

SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/ticklemonster:\$ORIGIN/gamestates:\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/lib:\$ORIGIN/bin")

if(APPLE)
  set(EXECUTABLE "TickleMonster")
else(APPLE)
  set(EXECUTABLE "ticklemonster")
endif(APPLE)

add_library("libsuperderpy-ticklemonster" SHARED ${SRC_LIST})
add_executable(${EXECUTABLE} WIN32 MACOSX_BUNDLE ${EXECUTABLE_SRC_LIST})

SET_TARGET_PROPERTIES("libsuperderpy-ticklemonster" PROPERTIES PREFIX "")

include_directories(${ALLEGRO5_INCLUDE_DIR} ${ALLEGRO5_FONT_INCLUDE_DIR} ${ALLEGRO5_TTF_INCLUDE_DIR} ${ALLEGRO5_PRIMITIVES_INCLUDE_DIR} ${ALLEGRO5_AUDIO_INCLUDE_DIR} ${ALLEGRO5_ACODEC_INCLUDE_DIR} ${ALLEGRO5_IMAGE_INCLUDE_DIR})

target_link_libraries(${EXECUTABLE} ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_MAIN_LIBRARIES} m dl libsuperderpy-ticklemonster)

target_link_libraries("libsuperderpy-ticklemonster" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_MAIN_LIBRARIES} m)

if(ALLEGRO5_MAIN_FOUND)
  target_link_libraries(${EXECUTABLE} ${ALLEGRO5_MAIN_LIBRARIES})
endif(ALLEGRO5_MAIN_FOUND)

add_subdirectory(gamestates)

install(TARGETS ${EXECUTABLE} DESTINATION ${BIN_INSTALL_DIR})
install(TARGETS "libsuperderpy-ticklemonster" DESTINATION ${LIB_INSTALL_DIR})
