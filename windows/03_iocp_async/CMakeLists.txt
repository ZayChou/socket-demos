add_executable(win03_server server.c)
target_link_libraries(win03_server PRIVATE ${SOCKET_LIBS})

add_executable(win03_client client.c)
target_link_libraries(win03_client PRIVATE ${SOCKET_LIBS})
