add_executable(linux01_server server.c)
target_link_libraries(linux01_server PRIVATE ${SOCKET_LIBS})

add_executable(linux01_client client.c)
target_link_libraries(linux01_client PRIVATE ${SOCKET_LIBS})
