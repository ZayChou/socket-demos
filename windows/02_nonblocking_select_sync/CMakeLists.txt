add_executable(win02_server server.c)
target_link_libraries(win02_server PRIVATE ${SOCKET_LIBS})

add_executable(win02_client client.c)
target_link_libraries(win02_client PRIVATE ${SOCKET_LIBS})
