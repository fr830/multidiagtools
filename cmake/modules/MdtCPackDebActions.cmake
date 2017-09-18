# To generate Debian packages that has dependencies external to Mdt,
# dpkg-shlibdeps is used on allready compiled Mdt libraries.
#
# This file was created to be callable from a add_custom_target() with cmake -P


# Discover external dependencies for library
#
# Input arguments:
#  COMPONENT:
#   Name of the component to which library belongs.
#  LIBRARY_FILE:
#   A full path to the library.
#  DEPENDENCIES_DIRECTORY:
#   A full path to the directory that will contain dependencies files.
#   For each component, 2 files exists:
#    <component>.mdtdebdep : file that contains Mdt dependencies. This is a input file for this function.
#    <component>.debdep : file that contains all the dependencies. This file is generated by this function.
#
# If COMPONENT does not contain a binary library,
# This function does nothing.
function(mdt_cpack_deb_find_library_external_dependencies)
  # Parse arguments
  set(oneValueArgs COMPONENT LIBRARY_FILE DEPENDENCIES_DIRECTORY)
  cmake_parse_arguments(VAR "" "${oneValueArgs}" "" ${ARGN})
  # Set our local variables and check the mandatory ones
  set(component ${VAR_COMPONENT})
  if(NOT component)
    message(FATAL_ERROR "COMPONENT argument is missing.")
  endif()
  string(TOUPPER ${component} component_upper)
  set(library_file ${VAR_LIBRARY_FILE})
  if(NOT library_file)
    message(FATAL_ERROR "LIBRARY_FILE argument is missing.")
  endif()
  string(REGEX REPLACE "^\"|\"$" "" dependencies_directory "${VAR_DEPENDENCIES_DIRECTORY}")
  if(NOT dependencies_directory)
    message(FATAL_ERROR "DEPENDENCIES_DIRECTORY argument is missing.")
  endif()

  # Check that dpkg-shlibdeps works
  find_program(shlibdeps_executable dpkg-shlibdeps)
  if(NOT shlibdeps_executable)
    message(WARNING "dpkg-shlibdeps seems not to be installed. External dependencies will not be added for component ${component}")
    return()
  endif()
  # dpkg-shlibdeps needs a debian control file
  execute_process(
    COMMAND mktemp -d
    OUTPUT_VARIABLE tmp_root
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  file(MAKE_DIRECTORY "${tmp_root}/debian")
  file(WRITE "${tmp_root}/debian/control" "")
  # Find dependencies for each library
  message(STATUS "Searching external dependencies for component '${component}'")
  set(external_dependencies_string)
  # Execute dpkg-shlibdeps
  # --ignore-missing-info : allow dpkg-shlibdeps to run even if some libs do not belong to a package
  # -O : print to STDOUT
  execute_process(
    COMMAND ${shlibdeps_executable} --ignore-missing-info -O "${library_file}"
    WORKING_DIRECTORY "${tmp_root}"
    OUTPUT_VARIABLE shlibdeps_output
    RESULT_VARIABLE retval
    ERROR_VARIABLE shlibdeps_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT retval EQUAL 0)
    message(FATAL_ERROR
      "MdtCPackDeb: dpkg-shlibdeps: '${shlibdeps_error}'"
      "executed command: '${shlibdeps_executable} --ignore-missing-info -O ${file}'"
    )
  endif()
  # Get rid of prefix generated by dpkg-shlibdeps and create a list
  string(REGEX REPLACE "^.*Depends=" "" shlibdeps_dependencies_string "${shlibdeps_output}")
  message("shlibdeps_dependencies_string: ${shlibdeps_dependencies_string}")
  if(shlibdeps_dependencies_string)
    string(REPLACE "," ";" shlibdeps_dependencies ${shlibdeps_dependencies_string})
    # Remove Mdt dependencies that are possibly found on previously installed Mdt package
    set(external_dependencies)
    foreach(dep_raw ${shlibdeps_dependencies})
      string(STRIP ${dep_raw} dep)
      if(NOT dep MATCHES "^libmdt")
        list(APPEND external_dependencies ${dep_raw})
      endif()
    endforeach()
    string(REPLACE ";" "," external_dependencies_string "${external_dependencies}")
  endif()
  # Remove debian control file and directory
  file(REMOVE_RECURSE "${tmp_root}")
  # Set complete depenencies string
  set(mdt_dep_file "${dependencies_directory}/${component}.mdtdebdep")
  if(EXISTS "${mdt_dep_file}")
    file(READ "${mdt_dep_file}" mdt_dependencies)
  endif()
  if(mdt_dependencies AND external_dependencies)
    set(dependencies_string "${mdt_dependencies}, ${external_dependencies_string}")
  elseif(mdt_dependencies)
    set(dependencies_string "${mdt_dependencies}")
  elseif(external_dependencies)
    set(dependencies_string "${external_dependencies_string}")
  else()
    set(dependencies_string)
  endif()
  # Generate dependencies files
  set(dep_file "${dependencies_directory}/${component}.debdep")
  if(dependencies_string)
    file(WRITE "${dep_file}" "CPACK_DEBIAN_${component_upper}_PACKAGE_DEPENDS \"${dependencies_string}\"")
  endif()
endfunction()

# Add Debian dependencies to CPackConfig.cmake
#
#  DEPENDENCIES_DIRECTORY:
#   A full path to the directory that will contain dependencies files.
#   For each file in this directory, that has .debdep extension,
#   will be used to add dependencies to CPackConfig.cmake
#
# Note: it is assumed that working directory is the one containing CPackConfig.cmake
function(mdt_cpack_deb_add_dependencies_to_config_file)
  # Parse arguments
  set(oneValueArgs COMPONENT DEPENDENCIES_DIRECTORY)
  cmake_parse_arguments(VAR "" "${oneValueArgs}" "" ${ARGN})
  # Set our local variables and check the mandatory ones
  string(REGEX REPLACE "^\"|\"$" "" dependencies_directory "${VAR_DEPENDENCIES_DIRECTORY}")
  if(NOT dependencies_directory)
    message(FATAL_ERROR "DEPENDENCIES_DIRECTORY argument is missing.")
  endif()
  if(NOT EXISTS "${dependencies_directory}")
    message(FATAL_ERROR "Directory does not exist: '${dependencies_directory}'")
  endif()

  # Add dependecies
  file(GLOB files "${dependencies_directory}/*.debdep")
  foreach(f ${files})
    file(READ ${f} var)
    file(APPEND CPackConfig.cmake "SET(${var})\n")
  endforeach()
endfunction()

# Generate Packages.gz file
function(mdt_cpack_deb_generate_package_index_file)
  # Check that dpkg-scanpackages works
  find_program(scanpackages_executable dpkg-scanpackages)
  if(NOT scanpackages_executable)
    message(WARNING "dpkg-scanpackages seems not to be installed. The Packages.gz will not be generated.")
    return()
  endif()
  # Check that gzip is installed
  find_program(gzip_executable gzip)
  if(NOT gzip_executable)
    message(WARNING "gzip seems not to be installed. The Packages.gz will not be generated.")
    return()
  endif()
  message(STATUS "Creating Packages file")
  # Generate Packages
  execute_process(
    COMMAND ${scanpackages_executable} "."
    RESULT_VARIABLE retval
    ERROR_VARIABLE scanpackages_error
    OUTPUT_VARIABLE packages
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT retval EQUAL 0)
    message(FATAL_ERROR "MdtCPackDeb: dpkg-scanpackages failed: '${scanpackages_error}'")
  endif()
  file(WRITE Packages "${packages}")
  # Compress Packages -> Packages.gz
  # gzip will automatically compress and rename the file
  # -f : else, gzip will query about confirmation to overwrite the file
  execute_process(
    COMMAND ${gzip_executable} -f Packages
    RESULT_VARIABLE retval
    ERROR_VARIABLE gzip_error
  )
  if(NOT retval EQUAL 0)
    message(FATAL_ERROR
      "MdtCPackDeb: gzip failed: '${gzip_error}'"
    )
  endif()
endfunction()

# Main
#
# Input arguments:
#  ACTION:
#   The action to perform.
#   If ACTION is find_library_dependencies, mdt_cpack_deb_find_library_component_external_dependencies() is called.
#   If ACTION is add_dependencies_to_config_file , mdt_cpack_deb_add_dependencies_to_config_file() is called.
#   If ACTION is generate_packages_file, mdt_cpack_deb_generate_package_index_file() is called.
#  COMPONENT:
#   Name of the component.
#   This argument is required when ACTION is find_library_dependencies
#  LIBRARY_FILE:
#   A full path to the library.
#   This argument is required when ACTION is find_library_dependencies
#  DEPENDENCIES_DIRECTORY:
#   A full path to the directory that will contain dependencies files.
#   For each component, 2 files exists:
#    <component>.mdtdebdep : file that contains Mdt dependencies.
#    <component>.debdep : file that contains all the dependencies.
#   This argument is required when ACTION is find_library_dependencies or add_dependencies_to_config_file.
if(NOT ACTION)
  message(FATAL_ERROR "ACTION argument is missing")
endif()
if(${ACTION} STREQUAL find_library_dependencies)
  if(NOT COMPONENT)
    message(FATAL_ERROR "For '${ACTION}' action, COMPONENT argument is required.")
  endif()
  if(NOT LIBRARY_FILE)
    message(FATAL_ERROR "For '${ACTION}' action, LIBRARY_FILE argument is required.")
  endif()
  if(NOT DEPENDENCIES_DIRECTORY)
    message(FATAL_ERROR "For '${ACTION}' action, DEPENDENCIES_DIRECTORY argument is required.")
  endif()

  mdt_cpack_deb_find_library_external_dependencies(
    COMPONENT ${COMPONENT}
    LIBRARY_FILE ${LIBRARY_FILE}
    DEPENDENCIES_DIRECTORY "${DEPENDENCIES_DIRECTORY}"
  )
elseif(${ACTION} STREQUAL add_dependencies_to_config_file)
  if(NOT DEPENDENCIES_DIRECTORY)
    message(FATAL_ERROR "For '${ACTION}' action, DEPENDENCIES_DIRECTORY argument is required.")
  endif()
  mdt_cpack_deb_add_dependencies_to_config_file(
    DEPENDENCIES_DIRECTORY "${DEPENDENCIES_DIRECTORY}"
  )
elseif(${ACTION} STREQUAL generate_packages_file)
  mdt_cpack_deb_generate_package_index_file()
else()
  message(FATAL_ERROR "Unknown action: ${ACTION}")
endif()
