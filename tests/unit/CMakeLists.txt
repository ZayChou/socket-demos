add_executable(test_placeholder test_placeholder.c)
add_test(NAME placeholder_unit_test COMMAND test_placeholder)

add_executable(test_echo_helpers test_echo_helpers.c)
if(WIN32)
    target_link_libraries(test_echo_helpers PRIVATE ${SOCKET_LIBS})
endif()
add_test(NAME unit_echo_helpers COMMAND test_echo_helpers)

