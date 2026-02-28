add_executable(linux02_server server.c)
target_link_libraries(linux02_server PRIVATE ${SOCKET_LIBS})

add_executable(linux02_client client.c)
target_link_libraries(linux02_client PRIVATE ${SOCKET_LIBS})
