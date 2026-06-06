if(NOT EXISTS "${CMAKE_BINARY_DIR}/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: ${CMAKE_BINARY_DIR}/install_manifest.txt")
endif()

file(READ "${CMAKE_BINARY_DIR}/install_manifest.txt" FILES)
string(REGEX REPLACE "\n" ";" FILES "${FILES}")
foreach(FILE ${FILES})
  message(STATUS "Uninstalling $ENV{DESTDIR}${FILE}")
  if(IS_SYMLINK "$ENV{DESTDIR}${FILE}" OR EXISTS "$ENV{DESTDIR}${FILE}")
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E remove "$ENV{DESTDIR}${FILE}"
      OUTPUT_VARIABLE rm_out
      RESULT_VARIABLE rm_retval
    )
    if(NOT "${rm_retval}" STREQUAL 0)
      message(FATAL_ERROR "Problem when removing $ENV{DESTDIR}${FILE}")
    endif()
  else(IS_SYMLINK "$ENV{DESTDIR}${FILE}" OR EXISTS "$ENV{DESTDIR}${FILE}")
    message(STATUS "File $ENV{DESTDIR}${FILE} does not exist.")
  endif()
endforeach()

