
enable_testing()

find_package(Qt5 COMPONENTS Test)

# set(CMAKE_INCLUDE_CURRENT_DIR ON)

#================= Application test library ====================

set(SOURCE_FILES
  src/TestBase.cpp
)

add_library(mdtapplicationcoretest SHARED ${SOURCE_FILES})
target_link_libraries(mdtapplicationcoretest Application_Core Qt5::Test)

#=============== Helper functions ===========

function(addApplicationCoreTest testName)
  set(complteTestName "MdtApplication_${testName}")
  set(sourceFilePath "src/${testName}.cpp")
  string(TOLOWER ${complteTestName} executableName)
  add_executable(${executableName} ${sourceFilePath})
  target_link_libraries(${executableName} mdtapplicationcoretest)
  add_test(NAME ${complteTestName} COMMAND ${executableName})
#  message("Test name: ${complteTestName} , src: ${sourceFilePath} , exec: ${executableName}")
endfunction()

#================= Tests ====================

addApplicationCoreTest("StandardPathsTest")
addApplicationCoreTest("CoreApplicationImplTest")
addApplicationCoreTest("CoreApplicationTest")
