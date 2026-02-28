# Integration tests — Linux only (fork + exec)
# Windows integration tests are run manually; see docs/verification.md.

if(NOT WIN32)
    add_executable(test_echo_integration test_echo_integration.c)
    add_dependencies(test_echo_integration
        linux01_server linux01_client
        linux02_server linux02_client
        linux03_server linux03_client)
    target_compile_definitions(test_echo_integration PRIVATE
        SERVER_01="$<TARGET_FILE:linux01_server>"
        CLIENT_01="$<TARGET_FILE:linux01_client>"
        SERVER_02="$<TARGET_FILE:linux02_server>"
        CLIENT_02="$<TARGET_FILE:linux02_client>"
        SERVER_03="$<TARGET_FILE:linux03_server>"
        CLIENT_03="$<TARGET_FILE:linux03_client>"
    )
    add_test(NAME integration_echo COMMAND test_echo_integration)
    set_tests_properties(integration_echo PROPERTIES TIMEOUT 30)
endif()
