if (NOT LIBSUPERDERPY_CONFIG_INCLUDED)

	# Set a default build type for single-configuration
	# CMake generators if no build type is set.
	if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE RelWithDebInfo)
	endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

	if (EXISTS "${CMAKE_SOURCE_DIR}/libsuperderpy")

		set(LIBSUPERDERPY_DIR "${CMAKE_SOURCE_DIR}/libsuperderpy")
		set(LIBSUPERDERPY_BINARY_DIR "${CMAKE_BINARY_DIR}/libsuperderpy")

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

		set(LIBSUPERDERPY_DIR ${CMAKE_SOURCE_DIR})
		set(LIBSUPERDERPY_BINARY_DIR ${CMAKE_BINARY_DIR})

	endif (EXISTS "${CMAKE_SOURCE_DIR}/libsuperderpy")

	execute_process(
		COMMAND git rev-parse --short HEAD
		WORKING_DIRECTORY ${LIBSUPERDERPY_DIR}
		OUTPUT_VARIABLE LIBSUPERDERPY_GIT_REV
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	add_definitions(-DLIBSUPERDERPY_GIT_REV="${LIBSUPERDERPY_GIT_REV}")

	set(SHARE_DIR "share" CACHE STRING "A directory under CMAKE_INSTALL_PREFIX where data files are installed to")
	set(BIN_DIR "bin" CACHE STRING "A directory under CMAKE_INSTALL_PREFIX where binary files are installed to")
	set(LIB_DIR "lib${LIB_SUFFIX}" CACHE STRING "A directory under CMAKE_INSTALL_PREFIX where library files are installed to")

	add_definitions(-D_POSIX_C_SOURCE=200809L)
	if (MAEMO5)
		add_definitions(-D_GNU_SOURCE)
	endif (MAEMO5)

	add_definitions(-DLIBSUPERDERPY_ORIENTATION_${LIBSUPERDERPY_ORIENTATION}=true)

	set(EMSCRIPTEN_TOTAL_MEMORY "128" CACHE STRING "Amount of memory allocated by Emscripten in asm.js builds (MB, must be multiple of 16)" )
	set(VITA_HEAP_SIZE "64" CACHE STRING "Amount of memory allocated by newlib for heap on PS Vita (MB)" )
	option(LIBSUPERDERPY_IMGUI "Compile with Dear ImGui support." OFF)
	if (LIBSUPERDERPY_IMGUI)
		enable_language(CXX)
		add_definitions(-DLIBSUPERDERPY_IMGUI)
	endif (LIBSUPERDERPY_IMGUI)

	set(CMAKE_C_STANDARD 99)
	set(CMAKE_C_STANDARD_REQUIRED ON)
	set(CMAKE_C_EXTENSIONS OFF)
	set(CMAKE_CXX_STANDARD 98)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)

	# TODO: add -fvisibility=hidden, but only to libsuperderpy target
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -ffast-math")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ffast-math")

	if (NOT MAEMO5 AND NOT VITA)
		# stack protector causes segfaults on Maemo and hangs on Vita
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
	endif(NOT MAEMO5 AND NOT VITA)

	if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage")
	endif()

	if(CMAKE_C_STANDARD_COMPUTED_DEFAULT EQUAL 90 OR NOT CMAKE_C_STANDARD_COMPUTED_DEFAULT)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
	endif(CMAKE_C_STANDARD_COMPUTED_DEFAULT EQUAL 90 OR NOT CMAKE_C_STANDARD_COMPUTED_DEFAULT)

	if(WIN32)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -municode")
		add_definitions(-DWIN32_LEAN_AND_MEAN)
		option(LIBSUPERDERPY_DLFCN "Use built-in dlfcn with UTF-8 support" ON)
	endif(WIN32)

	if(ANDROID)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
	endif(ANDROID)

	set(SANITIZERS "address,undefined" CACHE STRING "List of code sanitizers enabled for Debug builds")
	set_property(CACHE SANITIZERS PROPERTY STRINGS "" address undefined leak thread address,undefined leak,undefined thread,undefined memory memory,undefined)
	# leak sanitizer is a subset of address sanitizer
	# address/leak, memory and thread sanitizers are mutually exclusive (there can be only one at once)
	# there's also fuzzer, but it doesn't seem particularly interesting for us

	if (SANITIZERS)
		set(SANITIZERS_ARGS "-fsanitize=${SANITIZERS}")
		if ("${SANITIZERS}" MATCHES "(address)|(leak)")
			set(SANITIZERS_ARGS "${SANITIZERS_ARGS} -DLEAK_SANITIZER=1")
		endif()
		if ("${SANITIZERS}" MATCHES "address")
			set(SANITIZERS_ARGS "${SANITIZERS_ARGS} -fsanitize-address-use-after-scope")
		endif()
		if (NOT "${SANITIZERS}" MATCHES "memory")
			if ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
				set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -shared-libsan")
			endif()
			if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
				set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -shared-libsan")
			endif()
		endif()
	endif(SANITIZERS)

	if (EMSCRIPTEN)
		if(SANITIZERS_ARGS)
			message(STATUS "Sanitizers unavailable under Emscripten, disabling...")
			set(SANITIZERS_ARGS "")
		endif()
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -s ASSERTIONS=2 -s SAFE_HEAP=1")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -s ASSERTIONS=2 -s SAFE_HEAP=1")
	else()
		if(MAEMO5 AND SANITIZERS_ARGS)
			message(STATUS "Sanitizers unavailable on Maemo, disabling...")
			set(SANITIZERS_ARGS "")
		endif()
		if(VITA AND SANITIZERS_ARGS)
			message(STATUS "Sanitizers unavailable on Vita, disabling...")
			set(SANITIZERS_ARGS "")
		endif()
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb3")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
	endif()
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fno-common ${SANITIZERS_ARGS}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fno-common ${SANITIZERS_ARGS}")

	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D_FORTIFY_SOURCE=2")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -D_FORTIFY_SOURCE=2")


	if (NOT "${SANITIZERS}" MATCHES "memory")
		if(APPLE)
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-undefined,error")
		else(APPLE)
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")
		endif(APPLE)
	endif()

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

	if (EMSCRIPTEN)
		option(LIBSUPERDERPY_LTO "Use link-time optimization" ON)
	else()
		option(LIBSUPERDERPY_LTO "Use link-time optimization" OFF)
	endif()
	if(POLICY CMP0069 AND LIBSUPERDERPY_LTO)
		if(NOT USE_CLANG_TIDY AND NOT MINGW) # clang-tidy + GCC + LTO = errors; also, MinGW crashes
			cmake_policy(SET CMP0069 NEW)
			include(CheckIPOSupported)
			check_ipo_supported(RESULT IPO_SUPPORTED)
			message(STATUS "Link time optimization: ${IPO_SUPPORTED}")
			if(IPO_SUPPORTED)
				if(NOT EMSCRIPTEN)
					set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto")
					set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
					set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
					set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
				else()
					set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --llvm-lto 3")
					set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --llvm-lto 3")
				endif()
			endif()
		endif()
	endif()

	option(BUILD_SHARED_LIBS "Build as dynamic libraries" ON)

	option(LIBSUPERDERPY_STATIC "Compile and link libsuperderpy as a static library" OFF)
	option(LIBSUPERDERPY_STATIC_COMMON "Compile and link common routines as a static library" OFF)
	option(LIBSUPERDERPY_STATIC_GAMESTATES "Compile and link gamestates as static libraries" OFF)

	option(LIBSUPERDERPY_EMBEDDED_ALLEGRO "Compile an embedded copy of Allegro" OFF)

	if (NOT BUILD_SHARED_LIBS)
		set(LIBSUPERDERPY_STATIC ON)
		set(LIBSUPERDERPY_STATIC_COMMON ON)
		set(LIBSUPERDERPY_STATIC_GAMESTATES ON)
	endif()

	if(LIBSUPERDERPY_STATIC_GAMESTATES)
		add_definitions("-DLIBSUPERDERPY_STATIC_GAMESTATES")
	endif(LIBSUPERDERPY_STATIC_GAMESTATES)

	option(LIBSUPERDERPY_STATIC_DEPS "Link dependencies (e.g. Allegro) statically." OFF)
	if(LIBSUPERDERPY_STATIC_DEPS)
		SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
	endif(LIBSUPERDERPY_STATIC_DEPS)

	set(LIBSUPERDERPY_EXTRA_LIBS "${LIBSUPERDERPY_EXTRA_LIBS_INIT}" CACHE STRING "Additional platform libraries to link to")

	if(MAEMO5)
		add_definitions(-DMAEMO5=1)
		add_definitions(-D_Noreturn=)
		set(LIBSUPERDERPY_PLATFORM_OVERRIDE "maemo5")
	endif(MAEMO5)

	if(POCKETCHIP)
		add_definitions(-DPOCKETCHIP=1)
		set(LIBSUPERDERPY_PLATFORM_OVERRIDE "pocketchip")
	endif(POCKETCHIP)

	if(RASPBERRYPI)
		add_definitions(-DRASPBERRYPI=1)
		set(LIBSUPERDERPY_PLATFORM_OVERRIDE "raspberrypi")
	endif(RASPBERRYPI)

	if(VITA)
		add_definitions(-DLIBSUPERDERPY_VITA_HEAP_SIZE=${VITA_HEAP_SIZE})
		set(LIBSUPERDERPY_PLATFORM_OVERRIDE "vita")
	endif(VITA)

	if(STEAMLINK)
		set(LIBSUPERDERPY_PLATFORM_OVERRIDE "steamlink")
	endif(STEAMLINK)

	if(MAEMO5 OR POCKETCHIP)
		add_definitions(-DLIBSUPERDERPY_EMULATE_TOUCH=1)
	endif(MAEMO5 OR POCKETCHIP)

	if(EMSCRIPTEN OR SWITCH OR VITA)
		add_definitions(-DLIBSUPERDERPY_NO_RESTART=1)
	endif(EMSCRIPTEN OR SWITCH OR VITA)

	set(GAMESTATE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${LIB_DIR}")

	if(APPLE)
		if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local") # HACK
			set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
		endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")

		set(GAMESTATE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${BIN_DIR}/${LIBSUPERDERPY_GAMENAME}.app/Contents/MacOS/")

		set(MACOSX_BUNDLE_ICON_FILE ${LIBSUPERDERPY_GAMENAME})
		set(MACOSX_BUNDLE_BUNDLE_NAME ${LIBSUPERDERPY_GAMENAME_PRETTY})

	endif(APPLE)

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

	set(CMAKE_INSTALL_RPATH "\$ORIGIN/../${LIB_DIR}/${LIBSUPERDERPY_GAMENAME}:\$ORIGIN/gamestates:\$ORIGIN:\$ORIGIN/../${LIB_DIR}:\$ORIGIN/${LIB_DIR}:\$ORIGIN/${BIN_DIR}")

	if(EMSCRIPTEN)
		set(LIBSUPERDERPY_PLATFORM_OVERRIDE "emscripten")

		set(CMAKE_EXECUTABLE_SUFFIX ".bc")
		set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")

		# GNU extensions are needed for things like EM_ASM
		set(CMAKE_C_EXTENSIONS ON)
		set(CMAKE_CXX_EXTENSIONS ON)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
		set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O3")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")

		set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s SIDE_MODULE=2 -s EXPORTED_FUNCTIONS=[\"_Gamestate_ProgressCount\"]")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --ignore-dynamic-linking")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --ignore-dynamic-linking")
		set(EMSCRIPTEN_FLAGS -s TOTAL_MEMORY=${EMSCRIPTEN_TOTAL_MEMORY}MB --emit-symbol-map --use-preload-plugins --use-preload-cache -s FULL_ES2=1 -s EMTERPRETIFY=1 -s EMTERPRETIFY_FILE=\"${LIBSUPERDERPY_GAMENAME}.emterpret.js\" -s EMTERPRETIFY_ASYNC=1 -s EMTERPRETIFY_WHITELIST=[\"_main\",\"_libsuperderpy_emscripten_mainloop\",\"_libsuperderpy_mainloop\",\"_MainloopTick\",\"_GamestateLoadingThread\"] -s INCLUDE_FULL_LIBRARY=1 -s ERROR_ON_MISSING_LIBRARIES=1)

		set(LIBSUPERDERPY_EMSCRIPTEN_MODE "wasm" CACHE STRING "Emscripten compilation mode (JavaScript or WebAssembly)")
		set_property(CACHE LIBSUPERDERPY_EMSCRIPTEN_MODE PROPERTY STRINGS "asm.js;wasm")
		if("${LIBSUPERDERPY_EMSCRIPTEN_MODE}" STREQUAL "wasm")
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -s WASM=1")
			set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 --no-heap-copy -s BINARYEN_TRAP_MODE=\"clamp\")

			option(EMSCRIPTEN_DCE "Enable dead code elimination in WebAssembly build" ON)
			if (EMSCRIPTEN_DCE)
				set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s MAIN_MODULE=2 -s EXPORTED_FUNCTIONS=@${CMAKE_BINARY_DIR}/emscripten-imports.json)
			else (EMSCRIPTEN_DCE)
				set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s MAIN_MODULE=1 -s EXPORT_ALL=1)
			endif(EMSCRIPTEN_DCE)

			set(CMAKE_SHARED_MODULE_SUFFIX ".wasm.so")
			add_definitions(-DLIBSUPERDERPY_WASM=1)
		else()
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -s WASM=0")
			set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s WASM=0 -s PRECISE_F32=1 -s MAIN_MODULE=1 -s EXPORT_ALL=1)
			set(CMAKE_SHARED_MODULE_SUFFIX ".js")
		endif()

		option(LIBSUPERDERPY_USE_WEBGL2 "Use WebGL 2 context" OFF)
		if(LIBSUPERDERPY_USE_WEBGL2)
			set(EMSCRIPTEN_FLAGS ${EMSCRIPTEN_FLAGS} -s USE_WEBGL2=1)
		endif(LIBSUPERDERPY_USE_WEBGL2)

		if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local") # HACK
			set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/output")
		endif(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
		set(GAMESTATE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")

	endif()

	if(ANDROID OR EMSCRIPTEN OR VITA)
		add_definitions(-DLIBSUPERDERPY_SINGLE_THREAD)
	endif()

	if (LIBSUPERDERPY_EMBEDDED_ALLEGRO)
		set(Allegro5_LIBRARIES "allegro")
		set(Allegro5Font_LIBRARIES "allegro_font")
		set(Allegro5TTF_LIBRARIES "allegro_ttf")
		set(Allegro5Primitives_LIBRARIES "allegro_primitives")
		set(Allegro5Audio_LIBRARIES "allegro_audio")
		set(Allegro5ACodec_LIBRARIES "allegro_acodec")
		set(Allegro5Image_LIBRARIES "allegro_image")
		set(Allegro5Color_LIBRARIES "allegro_color")
		set(Allegro5Video_LIBRARIES "allegro_video")
		set(Allegro5Main_LIBRARIES "allegro_main")
		set(Allegro5_INCLUDE_DIR "")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/include")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/font")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/ttf")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/primitives")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/audio")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/acodec")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/image")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/color")
		include_directories("${LIBSUPERDERPY_DIR}/src/3rdparty/allegro5/addons/video")
		include_directories("${LIBSUPERDERPY_BINARY_DIR}/src/3rdparty/allegro5/include")
	else (LIBSUPERDERPY_EMBEDDED_ALLEGRO)
		find_package(Allegro5 REQUIRED)
		if(NOT EMSCRIPTEN)
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
		endif(NOT EMSCRIPTEN)
	endif()

	include_directories(${Allegro5_INCLUDE_DIR} ${Allegro5Font_INCLUDE_DIR} ${Allegro5TTF_INCLUDE_DIR} ${Allegro5Primitives_INCLUDE_DIR} ${Allegro5Audio_INCLUDE_DIR} ${Allegro5Acodec_INCLUDE_DIR} ${Allegro5Video_INCLUDE_DIR} ${Allegro5Image_INCLUDE_DIR} ${Allegro5Color_INCLUDE_DIR})

	MACRO(register_gamestate name sources)

		if (LIBSUPERDERPY_STATIC_GAMESTATES)
			add_library("lib${LIBSUPERDERPY_GAMENAME}-${name}" STATIC ${sources})
		else(LIBSUPERDERPY_STATIC_GAMESTATES)
			add_library("lib${LIBSUPERDERPY_GAMENAME}-${name}" MODULE ${sources})
		endif(LIBSUPERDERPY_STATIC_GAMESTATES)

		target_compile_definitions("lib${LIBSUPERDERPY_GAMENAME}-${name}" PRIVATE LIBSUPERDERPY_GAMESTATE=${name})

		set_target_properties("lib${LIBSUPERDERPY_GAMENAME}-${name}" PROPERTIES PREFIX "")

		if (NOT EMSCRIPTEN)
			if (TARGET lib${LIBSUPERDERPY_GAMENAME})
				target_link_libraries("lib${LIBSUPERDERPY_GAMENAME}-${name}" lib${LIBSUPERDERPY_GAMENAME} ${LIBSUPERDERPY_EXTRA_LIBS})
			else (TARGET lib${LIBSUPERDERPY_GAMENAME})
				if (NOT LIBSUPERDERPY_STATIC)
					target_link_libraries("lib${LIBSUPERDERPY_GAMENAME}-${name}" libsuperderpy ${LIBSUPERDERPY_EXTRA_LIBS})
				endif (NOT LIBSUPERDERPY_STATIC)
			endif(TARGET lib${LIBSUPERDERPY_GAMENAME})
		endif (NOT EMSCRIPTEN)

		install(TARGETS "lib${LIBSUPERDERPY_GAMENAME}-${name}" DESTINATION ${GAMESTATE_INSTALL_DIR})

		if (ANDROID)
			add_dependencies(${LIBSUPERDERPY_GAMENAME}_apk "lib${LIBSUPERDERPY_GAMENAME}-${name}")
		endif()

		if (EMSCRIPTEN)
			string(TOUPPER "CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}" CFLAGS)
			string(REPLACE " " ";" CFLAGS_L ${CMAKE_C_FLAGS})
			set(CFLAGS_LIST ${CFLAGS_L} ${${CFLAGS}})

			install(FILES "${CMAKE_BINARY_DIR}/src/gamestates/lib${LIBSUPERDERPY_GAMENAME}-${name}${CMAKE_SHARED_MODULE_SUFFIX}" DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}/gamestates)
		endif()

	ENDMACRO()

	MACRO(libsuperderpy_copy EXECUTABLE)

		if (NOT APPLE AND NOT ANDROID AND NOT LIBSUPERDERPY_STATIC)
			add_custom_command(TARGET "${EXECUTABLE}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "../libsuperderpy/src/libsuperderpy${CMAKE_SHARED_LIBRARY_SUFFIX}" $<TARGET_FILE_DIR:${EXECUTABLE}>)
		endif (NOT APPLE AND NOT ANDROID AND NOT LIBSUPERDERPY_STATIC)

	ENDMACRO()

	include(InstallRequiredSystemLibraries)

	if(LIBSUPERDERPY_GAMENAME)
		configure_file("${LIBSUPERDERPY_DIR}/src/defines.h.in" "${CMAKE_BINARY_DIR}/gen/defines.h")
		configure_file("${LIBSUPERDERPY_DIR}/src/emscripten.html.in" "${CMAKE_BINARY_DIR}/gen/emscripten.html")
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

	if (ANDROID)
		set(ASSET_PIPELINE_DATADIR "${CMAKE_BINARY_DIR}/android/app/src/main/assets/data")
		set(ASSET_PIPELINE_DEPEND "")
	else (EMSCRIPTEN)
		set(ASSET_PIPELINE_DATADIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/${LIBSUPERDERPY_GAMENAME}/data")
		set(ASSET_PIPELINE_DEPEND ${LIBSUPERDERPY_GAMENAME}_install)
	endif()

	set(FLACTOLOSSY_DEFAULT OFF)
	set(FLACTOLOSSY_FORMAT_DEFAULT "Opus")
	set(IMGTOWEBP_DEFAULT OFF)

	if (ANDROID OR EMSCRIPTEN)
		set(FLACTOLOSSY_DEFAULT ON)
		set(IMGTOWEBP_DEFAULT ON)
	endif()

	if (MAEMO5)
		set(FLACTOLOSSY_DEFAULT ON)
		set(FLACTOLOSSY_FORMAT_DEFAULT "Vorbis")
	endif()

	if (ANDROID OR EMSCRIPTEN)
		# restrict to Android or Emscripten for now, because only those platforms have ASSET_PIPELINE_DATADIR at this moment

		option(FLACTOLOSSY "Compress FLAC audio assets to lossy format" ${FLACTOLOSSY_DEFAULT})
		set(FLACTOLOSSY_BITRATE "192" CACHE STRING "Bitrate of resulting Vorbis/Opus files (kbps)")
		set(FLACTOLOSSY_SAMPLERATE "48000" CACHE STRING "Sample rate of resulting Vorbis files (does not apply to Opus) (Hz)")
		set(FLACTOLOSSY_FORMAT ${FLACTOLOSSY_FORMAT_DEFAULT} CACHE STRING "Lossy codec to use when encoding audio files")
		set_property(CACHE FLACTOLOSSY_FORMAT PROPERTY STRINGS "Opus;Vorbis")

		if (FLACTOLOSSY)
			if (${FLACTOLOSSY_FORMAT} STREQUAL "Vorbis")
				add_definitions(-DLIBSUPERDERPY_FLACTOLOSSY_EXT="ogg")
			else()
				add_definitions(-DLIBSUPERDERPY_FLACTOLOSSY_EXT="opus")
			endif()

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_flac_to_lossy
				DEPENDS ${ASSET_PIPELINE_DEPEND}
				COMMAND ${CMAKE_COMMAND} -DDATADIR=${ASSET_PIPELINE_DATADIR} -DCACHE="${CMAKE_SOURCE_DIR}/.assetcache" -DBITRATE=${FLACTOLOSSY_BITRATE} -DSAMPLERATE=${FLACTOLOSSY_SAMPLERATE} -P ${LIBSUPERDERPY_DIR}/cmake/FlacTo${FLACTOLOSSY_FORMAT}.cmake
				USES_TERMINAL)

		else(FLACTOLOSSY)
			add_custom_target(${LIBSUPERDERPY_GAMENAME}_flac_to_lossy
				DEPENDS ${ASSET_PIPELINE_DEPEND})
		endif(FLACTOLOSSY)

		option(IMGTOWEBP "Compress image assets to WebP format" ${IMGTOWEBP_DEFAULT})
		option(IMGTOWEBP_LOSSLESS "Use lossless WebP compression" OFF)
		if(IMGTOWEBP_LOSSLESS)
			set(IMGTOWEBP_QUALITY "100" CACHE STRING "Quality of resulting WebP files")
		else(IMGTOWEBP_LOSSLESS)
			set(IMGTOWEBP_QUALITY "75" CACHE STRING "Quality of resulting WebP files")
		endif(IMGTOWEBP_LOSSLESS)
		set(IMGTOWEBP_SCALE "100" CACHE STRING "Scaling factor (percentage) used when converting image assets to WebP")
		set(IMGTOWEBP_PARAMS "" CACHE STRING "Additional ImageMagick parameters")

		if(IMGTOWEBP)
			add_custom_target(${LIBSUPERDERPY_GAMENAME}_img_to_webp
				DEPENDS ${ASSET_PIPELINE_DEPEND}
				COMMAND ${CMAKE_COMMAND} -DQUALITY="${IMGTOWEBP_QUALITY}" -DRESIZE="${IMGTOWEBP_SCALE}%" -DPARAMS="${IMGTOWEBP_PARAMS}" -DCACHE="${CMAKE_SOURCE_DIR}/.assetcache" -DLOSSLESS="${IMGTOWEBP_LOSSLESS}" -DDATADIR=${ASSET_PIPELINE_DATADIR} -P ${LIBSUPERDERPY_DIR}/cmake/ImgToWebp.cmake
				USES_TERMINAL)
			add_definitions(-DLIBSUPERDERPY_IMGTOWEBP)
			add_definitions("-DLIBSUPERDERPY_IMAGE_SCALE=(${IMGTOWEBP_SCALE} / 100.0)")
		else(IMGTOWEBP)
			add_custom_target(${LIBSUPERDERPY_GAMENAME}_img_to_webp
				DEPENDS ${ASSET_PIPELINE_DEPEND})
			add_definitions(-DLIBSUPERDERPY_IMAGE_SCALE=1.0F)
		endif(IMGTOWEBP)

	else (ANDROID OR EMSCRIPTEN)
		add_definitions(-DLIBSUPERDERPY_IMAGE_SCALE=1.0F)
	endif (ANDROID OR EMSCRIPTEN)

	MACRO(add_libsuperderpy_target EXECUTABLE_SRC_LIST)
		if (LIBSUPERDERPY_STATIC_GAMESTATES)
			set(SRC_LIST ${EXECUTABLE_SRC_LIST} "${LIBSUPERDERPY_DIR}/src/gamestates-force-inclusion.c")
		else()
			set(SRC_LIST ${EXECUTABLE_SRC_LIST})
		endif()

		if(ANDROID)
			set(EXECUTABLE superderpy-game)
			add_library(${EXECUTABLE} SHARED ${SRC_LIST})

			set(APK_PATH ${CMAKE_BINARY_DIR}/android/bin/${LIBSUPERDERPY_GAMENAME}-debug.apk)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_apk ALL
				DEPENDS ${EXECUTABLE} ${LIBSUPERDERPY_GAMENAME}_flac_to_lossy ${LIBSUPERDERPY_GAMENAME}_img_to_webp
				BYPRODUCTS ${APK_PATH}
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/android"
				COMMAND ./gradlew assembleDebug
				USES_TERMINAL
				)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_install_apk
				DEPENDS ${LIBSUPERDERPY_GAMENAME}_apk
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/android"
				COMMAND adb -d install -r ${APK_PATH}
				USES_TERMINAL
				)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_run_apk
				DEPENDS ${LIBSUPERDERPY_GAMENAME}_install_apk
				COMMAND adb -d shell
				'am start -a android.intent.action.MAIN -n ${LIBSUPERDERPY_APPID}/net.dosowisko.libsuperderpy.Activity'
				USES_TERMINAL
				)

		else(ANDROID)
			add_executable(${EXECUTABLE} WIN32 MACOSX_BUNDLE ${SRC_LIST})
		endif(ANDROID)

		if(EMSCRIPTEN)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_install
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
				COMMAND "${CMAKE_COMMAND}" --build . --target install
				)
			string(TOUPPER "CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}" CFLAGS)
			string(REPLACE " " ";" CFLAGS_L ${CMAKE_C_FLAGS} " " ${${CFLAGS}})
			set(CFLAGS_LIST ${CFLAGS_L})

			if("${LIBSUPERDERPY_EMSCRIPTEN_MODE}" STREQUAL "wasm")
				add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/emscripten-imports.json COMMAND bash -c "(for file in ${CMAKE_INSTALL_PREFIX}/*.wasm.so; do wasm-dis $file; done) | grep \"(import \\\"env\\\" \" | awk '{print $3}' | awk -F '$' '{ print $2?$2:$0}' | sort -u | awk 'BEGIN {printf \"[\\\"_main\\\"\" } END {print \"]\"} {printf \",%s\", $1}' > ${CMAKE_BINARY_DIR}/emscripten-imports.json" DEPENDS ${LIBSUPERDERPY_GAMENAME}_install WORKING_DIRECTORY ${CMAKE_BINARY_DIR} USES_TERMINAL VERBATIM)
			else()
				# not implemented yet for asm.js
				add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/emscripten-imports.json COMMAND bash -c "echo '[\"_main\"]' > ${CMAKE_BINARY_DIR}/emscripten-imports.json" DEPENDS ${LIBSUPERDERPY_GAMENAME}_install WORKING_DIRECTORY ${CMAKE_BINARY_DIR} USES_TERMINAL VERBATIM)
			endif()

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_js
				DEPENDS ${LIBSUPERDERPY_GAMENAME}_install ${LIBSUPERDERPY_GAMENAME}_flac_to_lossy ${LIBSUPERDERPY_GAMENAME}_img_to_webp ${CMAKE_BINARY_DIR}/emscripten-imports.json
				WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${LIBSUPERDERPY_GAMENAME}"
				COMMAND "${CMAKE_C_COMPILER}" ${CFLAGS_LIST} ../${BIN_DIR}/${LIBSUPERDERPY_GAMENAME}${CMAKE_EXECUTABLE_SUFFIX} ../lib/libsuperderpy${CMAKE_SHARED_LIBRARY_SUFFIX} ../lib/lib${LIBSUPERDERPY_GAMENAME}${CMAKE_SHARED_LIBRARY_SUFFIX} ${Allegro5_LIBS} ${EMSCRIPTEN_FLAGS} -o ${LIBSUPERDERPY_GAMENAME}.html --shell-file ${CMAKE_BINARY_DIR}/gen/emscripten.html --pre-js ${LIBSUPERDERPY_DIR}/src/emscripten-pre-js.js --preload-file ../${SHARE_DIR}/${LIBSUPERDERPY_GAMENAME}/data@/data --preload-file gamestates@/
				USES_TERMINAL
				VERBATIM
				)

		endif(EMSCRIPTEN)
	ENDMACRO()

	if(ANDROID)
		set(ANDROID_TARGET "android-33" CACHE STRING "What Android target to compile for.")
		STRING(REGEX REPLACE "^android-" "" ANDROID_TARGET_VERSION ${ANDROID_TARGET})

		set(LIBSUPERDERPY_PLATFORM_OVERRIDE "android")

		file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/android")
		file(COPY "${LIBSUPERDERPY_DIR}/android" DESTINATION "${CMAKE_BINARY_DIR}")

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

		configure_android_file("app/src/main/AndroidManifest.xml")
		configure_android_file("local.properties")
		configure_android_file("app/build.gradle")
		configure_android_file("app/src/main/res/values/strings.xml")
		if (Allegro5_LIBRARIES MATCHES "^.*-debug.*$")
			set(ALLEGRO_DEBUG_SUFFIX "-debug")
		endif()
		configure_android_file("app/src/main/java/net/dosowisko/libsuperderpy/Activity.java")

		file(INSTALL ${Allegro5_LIBS} DESTINATION ${LIBRARY_OUTPUT_PATH})
		configure_file("${ANDROID_ALLEGRO_ROOT}/lib/allegro-release.aar" ${CMAKE_BINARY_DIR}/android/app/libs/allegro.aar COPYONLY)

		file(INSTALL "${CMAKE_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/assets/"
			PATTERN "stuff" EXCLUDE
			PATTERN ".git" EXCLUDE
			PATTERN ".gitignore" EXCLUDE
			PATTERN ".directory" EXCLUDE
			PATTERN "CMakeLists.txt" EXCLUDE
			PATTERN "android" EXCLUDE
			PATTERN "maemo5" EXCLUDE
			PATTERN "pocketchip" EXCLUDE
			PATTERN "raspberrypi" EXCLUDE
			PATTERN "steamlink" EXCLUDE
			PATTERN "emscripten" EXCLUDE
			PATTERN "vita" EXCLUDE
		)

		if(EXISTS "${CMAKE_SOURCE_DIR}/data/android")
			file(INSTALL "${CMAKE_SOURCE_DIR}/data/android/" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/assets/data/"
				PATTERN ".git" EXCLUDE
				PATTERN ".gitignore" EXCLUDE
				PATTERN ".directory" EXCLUDE
			)
		endif()

		file(INSTALL "${CMAKE_SOURCE_DIR}/data/icons/48/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-mdpi/")
		file(INSTALL "${CMAKE_SOURCE_DIR}/data/icons/72/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-hdpi/")
		file(INSTALL "${CMAKE_SOURCE_DIR}/data/icons/96/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xhdpi/")
		file(INSTALL "${CMAKE_SOURCE_DIR}/data/icons/144/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxhdpi/")
		file(INSTALL "${CMAKE_SOURCE_DIR}/data/icons/192/${LIBSUPERDERPY_GAMENAME}.png" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxxhdpi/")

	endif(ANDROID)

	if(VITA)
		include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

		if(NOT LIBSUPERDERPY_VITA_TITLEID)
			message(FATAL_ERROR "No TitleID for Vita provided (LIBSUPERDERPY_VITA_TITLEID)")
		endif()

		file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/vita")
		file(INSTALL "${CMAKE_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}/vita/"
			PATTERN "stuff" EXCLUDE
			PATTERN ".git" EXCLUDE
			PATTERN ".gitignore" EXCLUDE
			PATTERN ".directory" EXCLUDE
			PATTERN "CMakeLists.txt" EXCLUDE
			PATTERN "android" EXCLUDE
			PATTERN "maemo5" EXCLUDE
			PATTERN "pocketchip" EXCLUDE
			PATTERN "raspberrypi" EXCLUDE
			PATTERN "steamlink" EXCLUDE
			PATTERN "emscripten" EXCLUDE
			PATTERN "vita" EXCLUDE
		)

		if(EXISTS "${CMAKE_SOURCE_DIR}/data/vita")
			file(INSTALL "${CMAKE_SOURCE_DIR}/data/vita/" DESTINATION "${CMAKE_BINARY_DIR}/vita/data/"
				PATTERN ".git" EXCLUDE
				PATTERN ".gitignore" EXCLUDE
				PATTERN ".directory" EXCLUDE
				PATTERN "sce_sys" EXCLUDE
			)
		endif()


		if(EXISTS "${CMAKE_SOURCE_DIR}/data/vita/sce_sys")
			file(INSTALL "${CMAKE_SOURCE_DIR}/data/vita/sce_sys" DESTINATION "${CMAKE_BINARY_DIR}/vita/")
		else()
			file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/vita/sce_sys")
		endif()

		vita_create_self(eboot.bin ${CMAKE_BINARY_DIR}/src/${LIBSUPERDERPY_GAMENAME} UNSAFE)

		vita_create_vpk(${LIBSUPERDERPY_GAMENAME}.vpk "${LIBSUPERDERPY_VITA_TITLEID}" eboot.bin
		  NAME "${LIBSUPERDERPY_GAMENAME_PRETTY}"
		  FILE "${CMAKE_BINARY_DIR}/vita/sce_sys" sce_sys
		  FILE "${CMAKE_BINARY_DIR}/vita/data" data
		)

		if(NOT EXISTS "${CMAKE_SOURCE_DIR}/data/vita/sce_sys/icon0.png")
			add_custom_target(${LIBSUPERDERPY_GAMENAME}_vita_icon BYPRODUCTS ${CMAKE_BINARY_DIR}/vita/sce_sys/icon0.png
				COMMAND convert ${CMAKE_SOURCE_DIR}/data/icons/128/${LIBSUPERDERPY_GAMENAME}.png -background white -flatten ${CMAKE_BINARY_DIR}/vita/icon.png
				COMMAND pngquant 256 -f -o ${CMAKE_BINARY_DIR}/vita/icon2.png ${CMAKE_BINARY_DIR}/vita/icon.png
				COMMAND convert ${CMAKE_BINARY_DIR}/vita/icon2.png png8:${CMAKE_BINARY_DIR}/vita/sce_sys/icon0.png
			)
			add_dependencies(${LIBSUPERDERPY_GAMENAME}.vpk ${LIBSUPERDERPY_GAMENAME}_vita_icon)
		endif()

		if(DEFINED ENV{VITA_IP})

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_deploy_vpk
				DEPENDS ${LIBSUPERDERPY_GAMENAME}.vpk
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
				COMMAND curl --ftp-method nocwd -T ${LIBSUPERDERPY_GAMENAME}.vpk ftp://$ENV{VITA_IP}:1337/ux0:/data/
				COMMAND echo destroy | nc $ENV{VITA_IP} 1338
				COMMAND echo screen on | nc $ENV{VITA_IP} 1338
				COMMAND echo vpk ux0:/data/${LIBSUPERDERPY_GAMENAME}.vpk | nc $ENV{VITA_IP} 1338
				COMMAND echo screen on | nc $ENV{VITA_IP} 1338
				COMMAND echo launch ${LIBSUPERDERPY_VITA_TITLEID} | nc $ENV{VITA_IP} 1338
				USES_TERMINAL
					)

			add_custom_target(${LIBSUPERDERPY_GAMENAME}_deploy_eboot
				DEPENDS eboot.bin
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
				COMMAND echo destroy | nc $ENV{VITA_IP} 1338
				COMMAND sleep 1
				COMMAND echo screen on | nc $ENV{VITA_IP} 1338
				COMMAND curl --ftp-method nocwd -T eboot.bin ftp://$ENV{VITA_IP}:1337/ux0:/app/${LIBSUPERDERPY_VITA_TITLEID}/
				COMMAND echo launch ${LIBSUPERDERPY_VITA_TITLEID} | nc $ENV{VITA_IP} 1338
				USES_TERMINAL
					)

			if(EXISTS "${CMAKE_SOURCE_DIR}/data/vita/shaders")
				add_custom_target(${LIBSUPERDERPY_GAMENAME}_deploy_shaders
					WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
					COMMAND echo destroy | nc $ENV{VITA_IP} 1338
					COMMAND sleep 1
					COMMAND echo screen on | nc $ENV{VITA_IP} 1338
					COMMAND lftp -e 'mirror -R "${CMAKE_SOURCE_DIR}/data/vita/shaders/" ux0:/app/${LIBSUPERDERPY_VITA_TITLEID}/data/vita/shaders/ && exit' $ENV{VITA_IP} -p 1337
					COMMAND echo launch ${LIBSUPERDERPY_VITA_TITLEID} | nc $ENV{VITA_IP} 1338
					USES_TERMINAL
						)
			endif()
		else()
			message(STATUS "No VITA_IP found; set this env var for ability to deploy straight to Vita (using loaderCompanion)")
		endif()
	endif(VITA)

	# setup default RPATH/install_name handling
	# default is to build with RPATH for the install dir, so it doesn't need to relink
	if (UNIX)
		if (APPLE)
			set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${LIB_DIR})
		else (APPLE)
			# use the RPATH figured out by cmake when compiling
			set(CMAKE_SKIP_BUILD_RPATH TRUE)
			set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
			set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
		endif (APPLE)
	endif (UNIX)

	# uninstall target
	configure_file("${LIBSUPERDERPY_DIR}/cmake/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
	add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

	if (LIBSUPERDERPY_PLATFORM_OVERRIDE)
		add_definitions(-DLIBSUPERDERPY_PLATFORM_OVERRIDE=\"${LIBSUPERDERPY_PLATFORM_OVERRIDE}\")
	endif()

	set(LIBSUPERDERPY_CONFIG_INCLUDED 1)

endif (NOT LIBSUPERDERPY_CONFIG_INCLUDED)
