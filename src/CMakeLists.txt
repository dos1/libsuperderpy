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

if (LIBSUPERDERPY_STATIC)
	add_library("libsuperderpy" STATIC ${SRC_LIST})
	set_property(TARGET libsuperderpy PROPERTY POSITION_INDEPENDENT_CODE ON)
else (LIBSUPERDERPY_STATIC)
	add_library("libsuperderpy" SHARED ${SRC_LIST})
endif (LIBSUPERDERPY_STATIC)

SET_TARGET_PROPERTIES("libsuperderpy" PROPERTIES PREFIX "")

target_link_libraries("libsuperderpy" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_VIDEO_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} m dl)

if (ANDROID)
	target_link_libraries("libsuperderpy" log)
endif (ANDROID)

install(TARGETS "libsuperderpy" DESTINATION ${LIB_INSTALL_DIR})
