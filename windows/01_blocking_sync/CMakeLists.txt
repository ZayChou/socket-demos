add_executable(win01_server server.c)
target_link_libraries(win01_server PRIVATE ${SOCKET_LIBS})

add_executable(win01_client client.c)
target_link_libraries(win01_client PRIVATE ${SOCKET_LIBS})
