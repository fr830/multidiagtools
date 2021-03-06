
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type") 
set(MDT_CXX_FLAGS_TO_APPEND "-O3 -Wall -Wextra" CACHE STRING "")

# On *NIX we use mult-arch install (using GNUInstallDirs)
# This only works if CMAKE_INSTALL_PREFIX is set to /usr
# Because default is to build installations package, default this to /usr
if(UNIX)
  set(CMAKE_INSTALL_PREFIX /usr CACHE FILE "Installation prefix")
endif()
