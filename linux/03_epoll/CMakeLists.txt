add_executable(linux03_server server.c)
target_link_libraries(linux03_server PRIVATE ${SOCKET_LIBS})

add_executable(linux03_client client.c)
target_link_libraries(linux03_client PRIVATE ${SOCKET_LIBS})
