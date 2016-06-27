MACRO(GAMESTATE name)

	add_library("libsuperderpy-ticklemonster-${name}" SHARED "${name}.c")

	SET_TARGET_PROPERTIES("libsuperderpy-ticklemonster-${name}" PROPERTIES PREFIX "")

	target_link_libraries("libsuperderpy-ticklemonster-${name}" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} m libsuperderpy-ticklemonster)

	install(TARGETS "libsuperderpy-ticklemonster-${name}" DESTINATION ${LIB_INSTALL_DIR})

ENDMACRO()

GAMESTATE("dosowisko")
GAMESTATE("menu")
GAMESTATE("info")
GAMESTATE("level")

GAMESTATE("loading")
