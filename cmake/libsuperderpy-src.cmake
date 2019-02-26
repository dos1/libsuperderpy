set(EXECUTABLE ${LIBSUPERDERPY_GAMENAME})

set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/${LIBSUPERDERPY_GAMENAME}:\$ORIGIN/gamestates:\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/lib:\$ORIGIN/bin")

if(MINGW)
	# resource compilation for MinGW
	if (EXISTS ${CMAKE_SOURCE_DIR}/data/icons/icon.rc)
		set(EXECUTABLE_SRC_LIST ${EXECUTABLE_SRC_LIST} ${CMAKE_SOURCE_DIR}/data/icons/icon.rc)
	endif (EXISTS ${CMAKE_SOURCE_DIR}/data/icons/icon.rc)
endif(MINGW)

if (APPLE)
	if (EXISTS ${CMAKE_SOURCE_DIR}/data/icons/${LIBSUPERDERPY_GAMENAME}.icns)
		set(EXECUTABLE_SRC_LIST ${EXECUTABLE_SRC_LIST} ${CMAKE_SOURCE_DIR}/data/icons/${LIBSUPERDERPY_GAMENAME}.icns)
		set_source_files_properties(${CMAKE_SOURCE_DIR}/data/icons/${LIBSUPERDERPY_GAMENAME}.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	endif (EXISTS ${CMAKE_SOURCE_DIR}/data/icons/${LIBSUPERDERPY_GAMENAME}.icns)
endif(APPLE)

add_libsuperderpy_target("${EXECUTABLE_SRC_LIST}")
target_link_libraries(${EXECUTABLE} "lib${LIBSUPERDERPY_GAMENAME}")
if (APPLE)
	target_link_libraries(${EXECUTABLE} ${ALLEGRO5_MAIN_LIBRARIES})
endif(APPLE)
install(TARGETS ${EXECUTABLE} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_library("lib${LIBSUPERDERPY_GAMENAME}" SHARED "common.c")
set_target_properties("lib${LIBSUPERDERPY_GAMENAME}" PROPERTIES PREFIX "")
target_link_libraries("lib${LIBSUPERDERPY_GAMENAME}" libsuperderpy)
install(TARGETS "lib${LIBSUPERDERPY_GAMENAME}" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})

add_subdirectory("gamestates")

libsuperderpy_copy(${EXECUTABLE})
