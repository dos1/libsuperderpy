SET(SRC_LIST
	character.c
	config.c
	gamestate.c
	internal.c
	libsuperderpy.c
	mainloop.c
	maths.c
	particle.c
	shader.c
	timeline.c
	tween.c
	utils.c
)
if (EMSCRIPTEN)
	list(APPEND SRC_LIST emscripten-audio-stream.c)
endif()

if (LIBSUPERDERPY_IMGUI)
	set(IMGUI_STATIC ON CACHE STRING "" FORCE)
	add_subdirectory(3rdparty/cimgui EXCLUDE_FROM_ALL)
	set_property(TARGET "cimgui" PROPERTY POSITION_INDEPENDENT_CODE ON)

	list(APPEND SRC_LIST imgui/imgui_impl_allegro5.c)
endif (LIBSUPERDERPY_IMGUI)

if (LIBSUPERDERPY_STATIC)
	add_library("libsuperderpy" STATIC ${SRC_LIST})
	set_property(TARGET "libsuperderpy" PROPERTY POSITION_INDEPENDENT_CODE ON)
else (LIBSUPERDERPY_STATIC)
	add_library("libsuperderpy" SHARED ${SRC_LIST})
endif (LIBSUPERDERPY_STATIC)

SET_TARGET_PROPERTIES("libsuperderpy" PROPERTIES PREFIX "")

target_link_libraries("libsuperderpy" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_VIDEO_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} m dl)

if (LIBSUPERDERPY_IMGUI)
	target_link_libraries("libsuperderpy" cimgui)
endif (LIBSUPERDERPY_IMGUI)

if (ANDROID)
	target_link_libraries("libsuperderpy" log)
endif (ANDROID)

install(TARGETS "libsuperderpy" DESTINATION ${LIB_INSTALL_DIR})
