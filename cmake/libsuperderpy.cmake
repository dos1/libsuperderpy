if (NOT LIBSUPERDERPY_CONFIG_INCLUDED)

	add_definitions(-D_XOPEN_SOURCE=600)

	add_definitions(-DLIBSUPERDERPY_ORIENTATION_${LIBSUPERDERPY_ORIENTATION}=true)

	set(EMSCRIPTEN_TOTAL_MEMORY "32" CACHE STRING "Amount of memory allocated by Emscripten (MB, must be multiple of 16)" )

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fsanitize=leak -DLEAK_SANITIZER=1 -fno-common -fsanitize-recover=all")
	if(APPLE)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
	else(APPLE)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
	endif(APPLE)

	if(APPLE)
		if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
			set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
			set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
			set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}.app/Contents/MacOS/")
		endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")

		set(MACOSX_BUNDLE_ICON_FILE ${LIBSUPERDERPY_GAMENAME})
		set(MACOSX_BUNDLE_BUNDLE_NAME ${LIBSUPERDERPY_GAMENAME_PRETTY})

	endif(APPLE)

	include_directories("libsuperderpy/src")

	if(MINGW)
		# Guess MINGDIR from the value of CMAKE_C_COMPILER if it's not set.
		if(NOT MINGDIR)
			if("$ENV{MINGDIR}" STREQUAL "")
				string(REGEX REPLACE "/bin/[^/]*$" "" MINGDIR "${CMAKE_C_COMPILER}")
				message(STATUS "Guessed MinGW directory: ${MINGDIR}")
			else("$ENV{MINGDIR}" STREQUAL "")
				file(TO_CMAKE_PATH "$ENV{MINGDIR}" MINGDIR)
				message(STATUS "Using MINGDIR: ${MINGDIR}")
			endif("$ENV{MINGDIR}" STREQUAL "")
		endif(NOT MINGDIR)

		# Search in MINGDIR for headers and libraries.
		set(CMAKE_PREFIX_PATH "${MINGDIR}")

	endif(MINGW)

	set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/${LIBSUPERDERPY_GAMENAME}:\$ORIGIN/gamestates:\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/lib:\$ORIGIN/bin")

if(EMSCRIPTEN)
	  set(CMAKE_EXECUTABLE_SUFFIX ".bc")
		set(EMSCRIPTEN_USE_FLAGS -s USE_SDL=2 -s USE_FREETYPE=1 -s USE_LIBPNG=1 -s USE_ZLIB=1 -s USE_OGG=1 -s USE_VORBIS=1 -s FULL_ES2=1)
		# FIXME
		set(ALLEGRO5_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_ACODEC_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_FONT_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_TTF_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_PRIMITIVES_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_AUDIO_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_ACODEC_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_IMAGE_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_COLOR_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})
		set(ALLEGRO5_VIDEO_INCLUDE_DIR ${ALLEGRO_INCLUDE_PATH})

		if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
			set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/output")
		endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
		set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
		set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
		set(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

endif()

if(ANDROID OR EMSCRIPTEN)
	add_definitions(-DLIBSUPERDERPY_SINGLE_THREAD=1)
endif()

  find_package(Allegro5 REQUIRED)
	find_package(Allegro5Font REQUIRED)
	find_package(Allegro5TTF REQUIRED)
	find_package(Allegro5Primitives REQUIRED)
	find_package(Allegro5Audio REQUIRED)
	find_package(Allegro5ACodec REQUIRED)
	find_package(Allegro5Image REQUIRED)
	find_package(Allegro5Color REQUIRED)
	find_package(Allegro5Video REQUIRED)
	if(APPLE)
		find_package(Allegro5Main)
	endif(APPLE)

	include_directories(${ALLEGRO5_INCLUDE_DIR} ${ALLEGRO5_FONT_INCLUDE_DIR} ${ALLEGRO5_TTF_INCLUDE_DIR} ${ALLEGRO5_PRIMITIVES_INCLUDE_DIR} ${ALLEGRO5_AUDIO_INCLUDE_DIR} ${ALLEGRO5_ACODEC_INCLUDE_DIR} ${ALLEGRO5_VIDEO_INCLUDE_DIR} ${ALLEGRO5_IMAGE_INCLUDE_DIR})

	MACRO(register_gamestate name)

	  add_library("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" SHARED "${name}")

		set_target_properties("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" PROPERTIES PREFIX "")

		target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_FONT_LIBRARIES} ${ALLEGRO5_TTF_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_AUDIO_LIBRARIES} ${ALLEGRO5_ACODEC_LIBRARIES} ${ALLEGRO5_VIDEO_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_COLOR_LIBRARIES} m libsuperderpy)

		if (TARGET libsuperderpy-${LIBSUPERDERPY_GAMENAME})
			target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" libsuperderpy-${LIBSUPERDERPY_GAMENAME})
		endif()

		install(TARGETS "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" DESTINATION ${LIB_INSTALL_DIR})

		if (ANDROID)
			add_dependencies(${LIBSUPERDERPY_GAMENAME}_apk "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}")
		endif()

		if (EMSCRIPTEN)
			string(TOUPPER "CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}" CFLAGS)
			string(REPLACE " " ";" CFLAGS_L ${CMAKE_C_FLAGS})
			set(CFLAGS_LIST ${CFLAGS_L} ${${CFLAGS}})

			add_custom_command(TARGET "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" POST_BUILD
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/src/gamestates"
				COMMAND "${CMAKE_C_COMPILER}" ${CFLAGS_LIST} libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}.so -s SIDE_MODULE=1 -o libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}.js
				VERBATIM
			)
		install(FILES "${CMAKE_BINARY_DIR}/src/gamestates/libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}.js" DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}/gamestates)
	  endif()

	ENDMACRO()

	MACRO(libsuperderpy_copy EXECUTABLE)

	  if (NOT APPLE AND NOT ANDROID)
			add_custom_command(TARGET "${EXECUTABLE}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "../libsuperderpy/src/libsuperderpy${CMAKE_SHARED_LIBRARY_SUFFIX}" $<TARGET_FILE_DIR:${EXECUTABLE}>)
		endif (NOT APPLE AND NOT ANDROID)

	ENDMACRO()

	include(InstallRequiredSystemLibraries)

	if(LIBSUPERDERPY_GAMENAME)
		configure_file("${CMAKE_SOURCE_DIR}/libsuperderpy/src/defines.h.in" "${CMAKE_BINARY_DIR}/gen/defines.h")
		include_directories("${CMAKE_BINARY_DIR}/gen")
	endif(LIBSUPERDERPY_GAMENAME)

	if (NOT DEFINED LIBSUPERDERPY_VERSION)
		set(LIBSUPERDERPY_VERSION "0.1")
	endif(NOT DEFINED LIBSUPERDERPY_VERSION)

	if (NOT DEFINED LIBSUPERDERPY_RELEASE)
		set(LIBSUPERDERPY_RELEASE "1")
	endif(NOT DEFINED LIBSUPERDERPY_RELEASE)

	if (ANDROID OR EMSCRIPTEN)
		if (ANDROID)
			set(FLACTOOGG_DATADIR "${CMAKE_BINARY_DIR}/android/assets")
			set(FLACTOOGG_DEPEND "")
		else (EMSCRIPTEN)
			set(FLACTOOGG_DATADIR "${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}/data")
			set(FLACTOOGG_DEPEND ${LIBSUPERDERPY_GAMENAME}_install)
		endif()

		add_custom_target(${LIBSUPERDERPY_GAMENAME}_flac_to_ogg
			DEPENDS ${FLACTOOGG_DEPEND}
			COMMAND ${CMAKE_COMMAND} -DDATADIR=${FLACTOOGG_DATADIR} -P ${CMAKE_SOURCE_DIR}/libsuperderpy/cmake/FlacToOgg.cmake)

	endif(ANDROID OR EMSCRIPTEN)

	MACRO(add_libsuperderpy_target EXECUTABLE_SRC_LIST)
	  if(ANDROID)
			set(EXECUTABLE game)
			add_library(${EXECUTABLE} SHARED ${EXECUTABLE_SRC_LIST})

			set(APK_PATH ${CMAKE_BINARY_DIR}/android/bin/${LIBSUPERDERPY_GAMENAME}-debug.apk)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_apk ALL
				DEPENDS ${EXECUTABLE} ${LIBSUPERDERPY_GAMENAME}_flac_to_ogg
				BYPRODUCTS ${APK_PATH}
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/android"
				COMMAND ${ANT_COMMAND} debug
				)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_install_apk
				DEPENDS ${LIBSUPERDERPY_GAMENAME}_apk
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/android"
				COMMAND adb -d install -r ${APK_PATH}
				)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_run_apk
				DEPENDS ${LIBSUPERDERPY_GAMENAME}_install_apk
				COMMAND adb -d shell
				'am start -a android.intent.action.MAIN -n net.dosowisko.${LIBSUPERDERPY_GAMENAME}/.SuperDerpyActivity'
				)

		else(ANDROID)
			add_executable(${EXECUTABLE} WIN32 MACOSX_BUNDLE ${EXECUTABLE_SRC_LIST})
		endif(ANDROID)

		if(EMSCRIPTEN)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_install
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
				COMMAND "${CMAKE_COMMAND}" --build . --target install
				)
			string(TOUPPER "CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}" CFLAGS)
			string(REPLACE " " ";" CFLAGS_L ${CMAKE_C_FLAGS})
			set(CFLAGS_LIST ${CFLAGS_L} ${${CFLAGS}})

			math(EXPR EMSCRIPTEN_TOTAL_MEMORY_BYTES "${EMSCRIPTEN_TOTAL_MEMORY} * 1024 * 1024")


			add_custom_target(${LIBSUPERDERPY_GAMENAME}_js
				DEPENDS ${LIBSUPERDERPY_GAMENAME}_install ${LIBSUPERDERPY_GAMENAME}_flac_to_ogg
				WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}"
				COMMAND "${CMAKE_C_COMPILER}" ${CFLAGS_LIST}  ../${LIBSUPERDERPY_GAMENAME}.bc ../libsuperderpy.so ../libsuperderpy-${LIBSUPERDERPY_GAMENAME}.so ${ALLEGRO_LIBRARY_PATH} ${EMSCRIPTEN_USE_FLAGS} -s MAIN_MODULE=1 -s TOTAL_MEMORY=${EMSCRIPTEN_TOTAL_MEMORY_BYTES} -o ${LIBSUPERDERPY_GAMENAME}.html --preload-file data --preload-file gamestates@/
				COMMAND rm -rf data gamestates
				VERBATIM
				)
		endif(EMSCRIPTEN)
	ENDMACRO()

	if(ANDROID)
		set(ANDROID_TARGET "android-23" CACHE STRING "What Android target to compile for.")
		STRING(REGEX REPLACE "^android-" "" ANDROID_TARGET_VERSION ${ANDROID_TARGET})

		# The android tool on Windows is a batch file wrapper, which cannot be
		# started by MSYS shell directly. We invoke it via cmd.exe instead.
		# We don't use the full path to avoid problems with spaces,
		# and hope that android.bat is somewhere on the PATH.
		if(ANDROID_TOOL MATCHES "[.]bat$")
			set(ANDROID_UPDATE_COMMAND
				cmd.exe /c "android.bat update project -p . -t ${ANDROID_TARGET}")
		else()
			set(ANDROID_UPDATE_COMMAND
				"${ANDROID_TOOL}" update project -p . -t ${ANDROID_TARGET})
		endif()

		file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/android")
		file(COPY "${CMAKE_SOURCE_DIR}/libsuperderpy/android" DESTINATION "${CMAKE_BINARY_DIR}")

		MACRO(configure_android_file PATH)
		  configure_file("${CMAKE_BINARY_DIR}/android/${PATH}.in" "${CMAKE_BINARY_DIR}/android/${PATH}" ${ARGN})
			file(REMOVE "${CMAKE_BINARY_DIR}/android/${PATH}.in")
		ENDMACRO()

		if (LIBSUPERDERPY_ORIENTATION STREQUAL "PORTRAIT")
			set(LIBSUPERDERPY_ANDROID_ORIENTATION "sensorPortrait")
		elseif(LIBSUPERDERPY_ORIENTATION STREQUAL "LANDSCAPE")
			set(LIBSUPERDERPY_ANDROID_ORIENTATION "sensorLandscape")
		else()
			set(LIBSUPERDERPY_ANDROID_ORIENTATION "unspecified")
		endif()

		if (NOT DEFINED LIBSUPERDERPY_ANDROID_DEBUGGABLE)
			set(LIBSUPERDERPY_ANDROID_DEBUGGABLE "true")
		endif(NOT DEFINED LIBSUPERDERPY_ANDROID_DEBUGGABLE)

		configure_android_file("AndroidManifest.xml")
		configure_android_file("localgen.properties")
		configure_android_file("build.xml" @ONLY)
		configure_android_file("project.properties" @ONLY)
		configure_android_file("res/values/strings.xml")
		configure_android_file("jni/localgen.mk")
		if (ALLEGRO5_LIBRARIES MATCHES "^.*-debug.*$")
			set(ALLEGRO_DEBUG_SUFFIX "-debug")
		endif()
		configure_file("${CMAKE_BINARY_DIR}/android/src/net/dosowisko/libsuperderpy/SuperDerpyActivity.java.in" "${CMAKE_BINARY_DIR}/android/src/net/dosowisko/libsuperderpy/SuperDerpyActivity.java")
		file(REMOVE "${CMAKE_BINARY_DIR}/android/src/net/dosowisko/libsuperderpy/SuperDerpyActivity.java.in")

		file(RENAME "${CMAKE_BINARY_DIR}/android/src/net/dosowisko/libsuperderpy" "${CMAKE_BINARY_DIR}/android/src/net/dosowisko/${LIBSUPERDERPY_GAMENAME}")

		file(COPY ${ALLEGRO5_LIBS} DESTINATION ${LIBRARY_OUTPUT_PATH})
		file(COPY "${ANDROID_ALLEGRO_ROOT}/lib/Allegro5.jar" DESTINATION ${LIBRARY_OUTPUT_PATH})

		file(COPY "${CMAKE_SOURCE_DIR}/data/" DESTINATION "${CMAKE_BINARY_DIR}/android/assets/")

		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/48/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-mdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/72/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-hdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/96/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-xhdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/144/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-xxhdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/192/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-xxxhdpi/")

		execute_process(COMMAND ${ANDROID_UPDATE_COMMAND} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/android")

	endif(ANDROID)

	set(LIBSUPERDERPY_CONFIG_INCLUDED 1)

endif (NOT LIBSUPERDERPY_CONFIG_INCLUDED)
