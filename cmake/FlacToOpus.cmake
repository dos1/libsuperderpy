set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(AssetCache)

find_program(OPUSENC NAMES opusenc NO_CMAKE_FIND_ROOT_PATH)
if(OPUSENC AND DATADIR)
  file(GLOB_RECURSE FLAC_FILES RELATIVE ${DATADIR} ${DATADIR}/*.flac)
  message(STATUS "FlacToOpus engaging... (using ${OPUSENC})")
  foreach(file IN LISTS FLAC_FILES)
    message(STATUS ${file})
    string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" filename ${file})
    GetFromAssetCache(${CACHE} ${DATADIR}/${file} "opus-${BITRATE}" CACHED HASH)
    if (CACHED)
      file(REMOVE ${DATADIR}/${file})
      configure_file(${CACHED} ${DATADIR}/${filename}.opus COPYONLY)
      unset(CACHED)
    else()
      execute_process(COMMAND ${OPUSENC} --quiet --bitrate ${BITRATE} ${file} ${filename}.opus WORKING_DIRECTORY ${DATADIR} RESULT_VARIABLE OPUSENC_RESULT)
      if(OPUSENC_RESULT)
        message(WARNING "ERROR: ${OPUSENC_RESULT}")
      else()
        file(REMOVE ${DATADIR}/${file})
        AddToAssetCache(${CACHE} ${HASH} "opus-${BITRATE}" "${DATADIR}/${filename}.opus")
      endif()
    endif()
  endforeach(file)
else(OPUSENC AND DATADIR)
  if(NOT OPUSENC)
    message(WARNING "FlacToOpus: can't find opusenc!")
  else()
    message(WARNING "FlacToOpus: no DATADIR specified!")
  endif()
endif(OPUSENC AND DATADIR)
