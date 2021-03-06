function(GetFromAssetCache CACHE FILENAME SALT OUTPUT_PATH_VAR OUTPUT_HASH_VAR)
  file(SHA256 ${FILENAME} HASH)
  string(SHA1 HASHED_SALT ${SALT})
  if (EXISTS ${CACHE}/${HASH}-${HASHED_SALT})
    set(${OUTPUT_PATH_VAR} ${CACHE}/${HASH}-${HASHED_SALT} PARENT_SCOPE)
  endif()
  set(${OUTPUT_HASH_VAR} ${HASH} PARENT_SCOPE)
endfunction()

function(AddToAssetCache CACHE HASH SALT FILENAME)
  string(SHA1 HASHED_SALT ${SALT})
  configure_file(${FILENAME} ${CACHE}/${HASH}-${HASHED_SALT} COPYONLY)
endfunction()
