MACRO(GAMESTATE name)

        add_library("libsuperderpy-mediator-${name}" SHARED "${name}.c")

        SET_TARGET_PROPERTIES("libsuperderpy-mediator-${name}" PROPERTIES PREFIX "")

				target_link_libraries("libsuperderpy-mediator-${name}" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_COLOR_LIBRARIES} m libsuperderpy-mediator)

        install(TARGETS "libsuperderpy-mediator-${name}" DESTINATION ${LIB_INSTALL_DIR})

ENDMACRO()

GAMESTATE("dosowisko")
GAMESTATE("burndt")

GAMESTATE("menu")

GAMESTATE("info")
GAMESTATE("theend")

GAMESTATE("rockets")
GAMESTATE("lollipop")
GAMESTATE("riots")
GAMESTATE("bonus")

GAMESTATE("loading")
