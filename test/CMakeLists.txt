find_package(launch_testing_ament_cmake REQUIRED)
find_package(rmw_implementation_cmake REQUIRED)

set(COMM_TYPE_rmw_cyclonedds_cpp "CycloneDDS")
set(COMM_TYPE_rmw_fastrtps_cpp "FastRTPS")

function(add_performance_test COMM)
  set(MAX_RUNTIME 30)
  set(TOPIC "Array1k")
  get_filename_component(
    PERFORMANCE_REPORT_FILE
    "${AMENT_TEST_RESULTS_DIR}/${PROJECT_NAME}/performance_test_results_${COMM}.png"
    ABSOLUTE
  )
  configure_file(
    test_performance.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_performance_${COMM}.py.configure
    @ONLY
  )
  file(GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test/test_performance_${COMM}.py"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/test_performance_${COMM}.py.configure"
  )
  add_launch_test(
    "${CMAKE_CURRENT_BINARY_DIR}/test/test_performance_${COMM}.py"
    TARGET test_performance_${COMM}
    ENV "PERFORMANCE_REPORT_FILE=${PERFORMANCE_REPORT_FILE}"
  )
endfunction()

macro(try_add_performance_test)
  if(COMM_TYPE_${rmw_implementation})
    add_performance_test(${COMM_TYPE_${rmw_implementation}})
  else()
    message(STATUS "Skipping RMW implementation: ${rmw_implementation}")
  endif()
endmacro()

call_for_each_rmw_implementation(try_add_performance_test)
add_performance_test(ROS2)