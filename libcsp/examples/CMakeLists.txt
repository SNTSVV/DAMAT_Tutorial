if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_executable(csp_server_client EXCLUDE_FROM_ALL csp_server_client.c csp_server_client_posix.c)
  target_include_directories(csp_server_client PRIVATE ${csp_inc})
  target_link_libraries(csp_server_client PRIVATE libcsp Threads::Threads)
endif()

add_executable(csp_arch EXCLUDE_FROM_ALL csp_arch.c)
target_include_directories(csp_arch PRIVATE ${csp_inc})
target_link_libraries(csp_arch PRIVATE libcsp)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_executable(zmqproxy EXCLUDE_FROM_ALL zmqproxy.c)
  target_include_directories(zmqproxy PRIVATE ${csp_inc} ${LIBZMQ_INCLUDE_DIRS})
  target_link_libraries(zmqproxy PRIVATE libcsp Threads::Threads ${LIBZMQ_LIBRARIES})
endif()
