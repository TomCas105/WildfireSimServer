cmake_minimum_required(VERSION 3.18)
project(WildfireServer C)

set(CMAKE_C_STANDARD 11)

add_executable(WildfireServer main.c buffer.h
        pos_sockets/passive_socket.c pos_sockets/passive_socket.h
        pos_sockets/active_socket.c pos_sockets/active_socket.h
        pos_sockets/char_buffer.c pos_sockets/char_buffer.h
        pos_sockets/linked_list.h)

target_link_libraries(WildfireServer pthread)