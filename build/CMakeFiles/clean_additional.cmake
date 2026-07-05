# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/FFGui_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/FFGui_autogen.dir/ParseCache.txt"
  "FFGui_autogen"
  )
endif()
