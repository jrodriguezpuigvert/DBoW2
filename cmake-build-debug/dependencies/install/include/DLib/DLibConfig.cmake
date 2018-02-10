FIND_LIBRARY(DLib_LIBRARY DLib
    PATHS "/Users/jrodriguez/LS/DBoW2/cmake-build-debug/dependencies/install/lib"
)
FIND_PATH(DLib_INCLUDE_DIR DLibConfig.cmake
    PATHS "/Users/jrodriguez/LS/DBoW2/cmake-build-debug/dependencies/install/include/DLib" 
)
LIST(APPEND DLib_INCLUDE_DIR
  ${DLib_INCLUDE_DIR}/../ ${DLib_INCLUDE_DIR}/../DUtils
  ${DLib_INCLUDE_DIR}/../DUtilsCV ${DLib_INCLUDE_DIR}/../DVision)
SET(DLib_LIBRARIES ${DLib_LIBRARY})
SET(DLib_LIBS ${DLib_LIBRARY})
SET(DLib_INCLUDE_DIRS ${DLib_INCLUDE_DIR})
