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

if (LIBSUPERDERPY_EMBEDDED_ALLEGRO)
	option(SHARED "Build embedded Allegro as a shared library" OFF) # Allegro doesn't namespace this...
	add_subdirectory("3rdparty/allegro5" EXCLUDE_FROM_ALL)
	if (NOT VITA)
		set_property(TARGET "allegro" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_font" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_ttf" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_primitives" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_audio" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_acodec" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_image" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_color" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_video" PROPERTY POSITION_INDEPENDENT_CODE ON)
		set_property(TARGET "allegro_main" PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif (NOT VITA)
endif (LIBSUPERDERPY_EMBEDDED_ALLEGRO)

if (LIBSUPERDERPY_IMGUI)
	set(IMGUI_STATIC ON CACHE STRING "" FORCE)
	add_subdirectory(3rdparty/cimgui EXCLUDE_FROM_ALL)
	if (NOT VITA)
		set_property(TARGET "cimgui" PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif (NOT VITA)

	list(APPEND SRC_LIST imgui/imgui_impl_allegro5.c)
endif (LIBSUPERDERPY_IMGUI)

if (LIBSUPERDERPY_STATIC)
	add_library("libsuperderpy" STATIC ${SRC_LIST})
	if (NOT VITA)
		set_property(TARGET "libsuperderpy" PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif (NOT VITA)
else (LIBSUPERDERPY_STATIC)
	add_library("libsuperderpy" SHARED ${SRC_LIST})
endif (LIBSUPERDERPY_STATIC)

SET_TARGET_PROPERTIES("libsuperderpy" PROPERTIES PREFIX "")

if (NOT LIBSUPERDERPY_STATIC_GAMESTATES)
	if (WIN32 AND LIBSUPERDERPY_DLFCN)
		set(BUILD_SHARED_LIBS OFF CACHE STRING "" FORCE)
		add_subdirectory(3rdparty/dlfcn-win32 EXCLUDE_FROM_ALL)
		set_property(TARGET "dl" PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif(WIN32 AND LIBSUPERDERPY_DLFCN)
	target_link_libraries("libsuperderpy" dl)
endif()

target_link_libraries("libsuperderpy" ${Allegro5TTF_LIBRARIES} ${Allegro5Font_LIBRARIES} ${Allegro5Primitives_LIBRARIES} ${Allegro5ACodec_LIBRARIES} ${Allegro5Audio_LIBRARIES} ${Allegro5Video_LIBRARIES} ${Allegro5Color_LIBRARIES} ${Allegro5Image_LIBRARIES} ${Allegro5_LIBRARIES} m)

if (LIBSUPERDERPY_IMGUI)
	target_link_libraries("libsuperderpy" cimgui)
endif (LIBSUPERDERPY_IMGUI)

if (WIN32 AND LIBSUPERDERPY_DLFCN)
	target_link_libraries("libsuperderpy" psapi)
endif (WIN32 AND LIBSUPERDERPY_DLFCN)

if (ANDROID)
	target_link_libraries("libsuperderpy" log)
endif (ANDROID)

if (NOT LIBSUPERDERPY_STATIC)
	install(TARGETS "libsuperderpy" DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIB_DIR})
endif (NOT LIBSUPERDERPY_STATIC)
