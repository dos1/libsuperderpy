file(GLOB_RECURSE FLAC_FILES RELATIVE ${DATADIR} ${DATADIR}/*.flac)

foreach(file IN LISTS FLAC_FILES)
  message(${file})
  execute_process(COMMAND oggenc -b 192 --resample 44100 ${file} WORKING_DIRECTORY ${DATADIR})
  file(REMOVE ${DATADIR}/${file})
endforeach(file)
