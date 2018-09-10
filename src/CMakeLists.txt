SET(SRC_LIST
	utils.c
	maths.c
	config.c
	timeline.c
	gamestate.c
	libsuperderpy.c
	mainloop.c
	character.c
	internal.c
	tween.c
	shader.c
)
if (EMSCRIPTEN)
	list(APPEND SRC_LIST emscripten-audio-stream.c)
endif()

add_library("libsuperderpy" SHARED ${SRC_LIST})

SET_TARGET_PROPERTIES("libsuperderpy" PROPERTIES PREFIX "")

target_link_libraries("libsuperderpy" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_VIDEO_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} m dl)

install(TARGETS "libsuperderpy" DESTINATION ${LIB_INSTALL_DIR})
