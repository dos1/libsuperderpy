if (NOT LIBSUPERDERPY_CONFIG_INCLUDED)

	# Set a default build type for single-configuration
	# CMake generators if no build type is set.
	if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE RelWithDebInfo)
	endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

	add_definitions(-D_XOPEN_SOURCE=600)

	add_definitions(-DLIBSUPERDERPY_ORIENTATION_${LIBSUPERDERPY_ORIENTATION}=true)

	set(EMSCRIPTEN_TOTAL_MEMORY "128" CACHE STRING "Amount of memory allocated by Emscripten (MB, must be multiple of 16)" )
	option(LIBSUPERDERPY_IMGUI "Compile with Dear ImGui support." OFF)
	if (LIBSUPERDERPY_IMGUI)
		enable_language(CXX)
		add_definitions(-DLIBSUPERDERPY_IMGUI)
	endif (LIBSUPERDERPY_IMGUI)

	set(CMAKE_C_STANDARD 99)
	set(CMAKE_C_STANDARD_REQUIRED true)
	set(CMAKE_CXX_STANDARD 98)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ffast-math")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ffast-math")

	if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage")
	endif()

	if(MAEMO5)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++98")
	endif(MAEMO5)

	if(WIN32)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -municode")
		add_definitions(-DWIN32_LEAN_AND_MEAN)
		option(LIBSUPERDERPY_DLFCN "Use built-in dlfcn with UTF-8 support" ON)
	endif(WIN32)

	if(ANDROID)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
	endif(ANDROID)

	set(DEFAULT_SANITIZERS "address,undefined")
	if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		set(DEFAULT_SANITIZERS "leak")
	endif()
	set(SANITIZERS ${DEFAULT_SANITIZERS} CACHE STRING "List of code sanitizers enabled for Debug builds")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fsanitize=${SANITIZERS} -DLEAK_SANITIZER=1 -fno-common -fsanitize-recover=all")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fsanitize=${SANITIZERS} -DLEAK_SANITIZER=1 -fno-common -fsanitize-recover=all")

	if(APPLE)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-undefined,error")
	else(APPLE)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")
	endif(APPLE)

	option(USE_CLANG_TIDY "Analyze the code with clang-tidy" OFF)
	if(USE_CLANG_TIDY AND NOT MINGW)
		find_program(CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable")
		mark_as_advanced(CLANG_TIDY_EXE)
		if(NOT CLANG_TIDY_EXE)
			message(STATUS "clang-tidy not found, analysis disabled")
		else()
			if (EXISTS "${CMAKE_SOURCE_DIR}/.clang-tidy")
				file(READ "${CMAKE_SOURCE_DIR}/.clang-tidy" CLANG_TIDY_CONFIG)
			else (EXISTS "${CMAKE_SOURCE_DIR}/.clang-tidy")
				file(READ "${CMAKE_SOURCE_DIR}/libsuperderpy/.clang-tidy" CLANG_TIDY_CONFIG)
			endif (EXISTS "${CMAKE_SOURCE_DIR}/.clang-tidy")
			string(STRIP "${CLANG_TIDY_CONFIG}" CLANG_TIDY_CONFIG)
			set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}" "-config=${CLANG_TIDY_CONFIG}")
		endif()
	endif()

	option(LIBSUPERDERPY_STATIC "Compile and link libsuperderpy as a static library." OFF)

	option(LIBSUPERDERPY_STATIC_DEPS "Link dependencies (e.g. Allegro) statically." OFF)
	if(LIBSUPERDERPY_STATIC_DEPS)
		SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
	endif(LIBSUPERDERPY_STATIC_DEPS)

	if(MAEMO5)
		add_definitions(-DMAEMO5=1)
		add_definitions(-D_Noreturn=)
	endif(MAEMO5)

	set(GAMESTATE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")

	if(APPLE)
		if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local") # HACK
			set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
		endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")

		set(GAMESTATE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin/${LIBSUPERDERPY_GAMENAME}.app/Contents/MacOS/")

		set(MACOSX_BUNDLE_ICON_FILE ${LIBSUPERDERPY_GAMENAME})
		set(MACOSX_BUNDLE_BUNDLE_NAME ${LIBSUPERDERPY_GAMENAME_PRETTY})

	endif(APPLE)

	if (EXISTS "${CMAKE_SOURCE_DIR}/libsuperderpy")

		execute_process(
			COMMAND git rev-parse --short HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libsuperderpy
			OUTPUT_VARIABLE LIBSUPERDERPY_GIT_REV
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		add_definitions(-DLIBSUPERDERPY_GIT_REV="${LIBSUPERDERPY_GIT_REV}")

		if(NOT DEFINED LIBSUPERDERPY_NO_GAME_GIT_REV)
			execute_process(
				COMMAND git rev-parse --short HEAD
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
				OUTPUT_VARIABLE LIBSUPERDERPY_GAME_GIT_REV
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
		endif(NOT DEFINED LIBSUPERDERPY_NO_GAME_GIT_REV)

		include_directories("libsuperderpy/src")

	else (EXISTS "${CMAKE_SOURCE_DIR}/libsuperderpy")

		execute_process(
			COMMAND git rev-parse --short HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE LIBSUPERDERPY_GIT_REV
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		add_definitions(-DLIBSUPERDERPY_GIT_REV="${LIBSUPERDERPY_GIT_REV}")

	endif (EXISTS "${CMAKE_SOURCE_DIR}/libsuperderpy")

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
		set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
		set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s SIDE_MODULE=1")
		set(EMSCRIPTEN_FLAGS -s USE_SDL=2 -s USE_FREETYPE=1 -s USE_LIBPNG=1 -s USE_ZLIB=1 -s USE_OGG=1 -s USE_VORBIS=1 -s FULL_ES2=1 -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s NO_EXIT_RUNTIME=0 -s PRECISE_F32=1)

		set(LIBSUPERDERPY_EMSCRIPTEN_MODE "asm.js" CACHE STRING "Emscripten compilation mode (JavaScript or WebAssembly)")
		set_property(CACHE LIBSUPERDERPY_EMSCRIPTEN_MODE PROPERTY STRINGS "asm.js;wasm")
		if("${LIBSUPERDERPY_EMSCRIPTEN_MODE}" STREQUAL "wasm")
			# https://github.com/kripken/emscripten/issues/5436
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -s WASM=1")
			set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s WASM=1)
			set(CMAKE_SHARED_MODULE_SUFFIX ".wasm")
			add_definitions(-DLIBSUPERDERPY_WASM=1)
		else()
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -s WASM=0")
			set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s WASM=0)
			set(CMAKE_SHARED_MODULE_SUFFIX ".js")
		endif()

		option(LIBSUPERDERPY_USE_WEBGL2 "Use WebGL 2 context" ON)
		if(LIBSUPERDERPY_USE_WEBGL2)
			set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s USE_WEBGL2=1)
		endif(LIBSUPERDERPY_USE_WEBGL2)

		if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local") # HACK
			set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/output")
		endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
		set(GAMESTATE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")

	endif()

	if(ANDROID OR EMSCRIPTEN)
		add_definitions(-DLIBSUPERDERPY_SINGLE_THREAD)
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

	MACRO(register_gamestate name sources)

		add_library("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" MODULE ${sources})

		set_target_properties("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" PROPERTIES PREFIX "")

		if (NOT EMSCRIPTEN)
			target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" ${ALLEGRO5_LIBS} m)

			if (TARGET libsuperderpy-${LIBSUPERDERPY_GAMENAME})
				target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" libsuperderpy-${LIBSUPERDERPY_GAMENAME})
			else (TARGET libsuperderpy-${LIBSUPERDERPY_GAMENAME})
				if (NOT LIBSUPERDERPY_STATIC)
					target_link_libraries("libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" libsuperderpy)
				endif (NOT LIBSUPERDERPY_STATIC)
			endif(TARGET libsuperderpy-${LIBSUPERDERPY_GAMENAME})
		endif (NOT EMSCRIPTEN)

		install(TARGETS "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}" DESTINATION ${GAMESTATE_INSTALL_DIR})

		if (ANDROID)
			add_dependencies(${LIBSUPERDERPY_GAMENAME}_apk "libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}")
		endif()

		if (EMSCRIPTEN)
			string(TOUPPER "CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}" CFLAGS)
			string(REPLACE " " ";" CFLAGS_L ${CMAKE_C_FLAGS})
			set(CFLAGS_LIST ${CFLAGS_L} ${${CFLAGS}})

			install(FILES "${CMAKE_BINARY_DIR}/src/gamestates/libsuperderpy-${LIBSUPERDERPY_GAMENAME}-${name}${CMAKE_SHARED_MODULE_SUFFIX}" DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}/gamestates)
		endif()

	ENDMACRO()

	MACRO(libsuperderpy_copy EXECUTABLE)

		if (NOT APPLE AND NOT ANDROID AND NOT LIBSUPERDERPY_STATIC)
			add_custom_command(TARGET "${EXECUTABLE}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "../libsuperderpy/src/libsuperderpy${CMAKE_SHARED_LIBRARY_SUFFIX}" $<TARGET_FILE_DIR:${EXECUTABLE}>)
		endif (NOT APPLE AND NOT ANDROID AND NOT LIBSUPERDERPY_STATIC)

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

	if (NOT DEFINED LIBSUPERDERPY_APPID)
		set(LIBSUPERDERPY_APPID "net.dosowisko.${LIBSUPERDERPY_GAMENAME}")
	endif(NOT DEFINED LIBSUPERDERPY_APPID)

	if (ANDROID OR EMSCRIPTEN)
		if (ANDROID)
			set(FLACTOOGG_DATADIR "${CMAKE_BINARY_DIR}/android/assets/data")
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
				'am start -a android.intent.action.MAIN -n ${LIBSUPERDERPY_APPID}/net.dosowisko.libsuperderpy.Activity'
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
			string(REPLACE " " ";" CFLAGS_L ${CMAKE_C_FLAGS} " " ${${CFLAGS}})
			set(CFLAGS_LIST ${CFLAGS_L})

			math(EXPR EMSCRIPTEN_TOTAL_MEMORY_BYTES "${EMSCRIPTEN_TOTAL_MEMORY} * 1024 * 1024")


			add_custom_target(${LIBSUPERDERPY_GAMENAME}_js
				DEPENDS ${LIBSUPERDERPY_GAMENAME}_install ${LIBSUPERDERPY_GAMENAME}_flac_to_ogg
				WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}"
				COMMAND "${CMAKE_C_COMPILER}" ${CFLAGS_LIST} ../bin/${LIBSUPERDERPY_GAMENAME}${CMAKE_EXECUTABLE_SUFFIX} ../lib/libsuperderpy${CMAKE_SHARED_LIBRARY_SUFFIX} ../lib/libsuperderpy-${LIBSUPERDERPY_GAMENAME}${CMAKE_SHARED_LIBRARY_SUFFIX} ${ALLEGRO5_LIBS} ${EMSCRIPTEN_FLAGS} -s MAIN_MODULE=1 -s TOTAL_MEMORY=${EMSCRIPTEN_TOTAL_MEMORY_BYTES} -o ${LIBSUPERDERPY_GAMENAME}.html --preload-file ../share/${LIBSUPERDERPY_GAMENAME}/data --preload-file gamestates@/
				VERBATIM
				)
		endif(EMSCRIPTEN)
	ENDMACRO()

	if(ANDROID)
		set(ANDROID_TARGET "android-26" CACHE STRING "What Android target to compile for.")
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
		configure_file("${CMAKE_BINARY_DIR}/android/src/net/dosowisko/libsuperderpy/Activity.java.in" "${CMAKE_BINARY_DIR}/android/src/net/dosowisko/libsuperderpy/Activity.java")
		file(REMOVE "${CMAKE_BINARY_DIR}/android/src/net/dosowisko/libsuperderpy/Activity.java.in")

		file(COPY ${ALLEGRO5_LIBS} DESTINATION ${LIBRARY_OUTPUT_PATH})
		file(COPY "${ANDROID_ALLEGRO_ROOT}/lib/Allegro5.jar" DESTINATION ${LIBRARY_OUTPUT_PATH})

		file(COPY "${CMAKE_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}/android/assets/" PATTERN "stuff" EXCLUDE
			PATTERN ".git" EXCLUDE
			PATTERN ".gitignore" EXCLUDE
			PATTERN ".directory" EXCLUDE
			PATTERN "CMakeLists.txt" EXCLUDE)

		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/48/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-mdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/72/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-hdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/96/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-xhdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/144/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-xxhdpi/")
		file(COPY "${CMAKE_SOURCE_DIR}/data/icons/192/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/res/mipmap-xxxhdpi/")

		execute_process(COMMAND ${ANDROID_UPDATE_COMMAND} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/android")

	endif(ANDROID)

	# setup default RPATH/install_name handling
	# default is to build with RPATH for the install dir, so it doesn't need to relink
	if (UNIX)
		if (APPLE)
			set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
		else (APPLE)
			# use the RPATH figured out by cmake when compiling
			set(CMAKE_SKIP_BUILD_RPATH TRUE)
			set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
			set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
		endif (APPLE)
	endif (UNIX)

	# uninstall target
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libsuperderpy/cmake/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
	add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

	set(LIBSUPERDERPY_CONFIG_INCLUDED 1)

endif (NOT LIBSUPERDERPY_CONFIG_INCLUDED)
