################################################################################
# tests/TestSuit.txt
#
# Copyright (C) 2017 Marvin Löbel <loebel.marvin@gmail.com>
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

# Custom test target to run the googletest tests
add_custom_target(check)
add_custom_command(
  TARGET check
  POST_BUILD
  COMMENT "All tests were successful!" VERBATIM
)

# Custom test target to just build the googletest tests
add_custom_target(build_check)
add_custom_command(
  TARGET build_check
  POST_BUILD
  COMMENT "All test builds were successful!" VERBATIM
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stamps)

# will compile and run ${test_target}.cpp
# and add all further arguments as dependencies
macro(generic_run_test
  test_target
  test_file
  driver
  driver_dep
  register_target
  register_build_target
  number_pes)

  set(options)
  set(oneValueArgs)
  set(multiValueArgs DEPS BIN_DEPS)
  cmake_parse_arguments(TEST_TARGET
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${test_target}_testrunner
    EXCLUDE_FROM_ALL
    ${SEQUENTIAL_ALGORITHMS_IMPL}
    ${driver}
    ${test_file}
  )

  target_include_directories(${test_target}_testrunner
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

  target_link_libraries(${test_target}_testrunner
    dsss_string_sorting
    ${driver_dep}
  )

  if(${number_pes} GREATER 0)
    # Runs the test and generates a stamp file on success.
    add_custom_command(
      OUTPUT stamps/${test_target}_testrunner.stamp
      DEPENDS ${test_target}_testrunner
      COMMAND ${MPIEXEC} --oversubscribe ${MPIEXEC_NUMPROC_FLAG} ${number_pes}
        ./${test_target}_testrunner
      COMMAND cmake -E touch
        ${CMAKE_CURRENT_BINARY_DIR}/stamps/${test_target}_testrunner.stamp
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
      COMMENT "Running ${kind_name} ${test_target} ..."
      VERBATIM
    )
  else(${number_pes} GREATER 0)
    # Runs the test and generates a stamp file on success.
    add_custom_command(
      OUTPUT stamps/${test_target}_testrunner.stamp
      DEPENDS ${test_target}_testrunner
      COMMAND ${test_target}_testrunner
      COMMAND cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/stamps/${test_target}_testrunner.stamp
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
      COMMENT "Running ${kind_name} ${test_target} ..."
      VERBATIM
    )
  endif(${number_pes} GREATER 0)

  # The test target. Depends on the stamp file to ensure the
  # test is only run if the source changed
  add_custom_target(
    ${test_target}
    DEPENDS
      stamps/${test_target}_testrunner.stamp
  )

  # Hook into check target
  add_custom_command(
    TARGET ${register_target}
    PRE_BUILD
    COMMAND cmake --build . --target ${test_target}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "TEST ${test_target}" VERBATIM
  )

  # Hook into build_check target
  add_custom_command(
    TARGET ${register_build_target}
    PRE_BUILD
    COMMAND cmake --build . --target ${test_target}_testrunner
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Building TEST ${test_target}" VERBATIM
  )

  # Ensure binary deps of the testrunner are compiled first
  foreach(bin_dep ${TEST_TARGET_BIN_DEPS})
    add_custom_command(
      TARGET ${test_target}_testrunner
      PRE_BUILD
      COMMAND cmake --build . --target ${bin_dep}
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
  endforeach(bin_dep)
endmacro()

macro(run_test test_target)
  string(REPLACE "/" "_" test_name "${test_target}")
  generic_run_test(
    ${test_name}
    "${test_target}.cpp"
    "test_runner.cpp"
    gtest
    check
    build_check
    0
    ${ARGN}
  )
endmacro()

macro(run_mpi_test test_target)
  string(REPLACE "/" "_" test_name "${test_target}")
  generic_run_test(
    ${test_name}
    "${test_target}.cpp"
    "mpi_test_runner.cpp"
    gtest
    check
    build_check
    4
    ${ARGN}
  )
endmacro()

################################################################################
