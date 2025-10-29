cmake_minimum_required(VERSION 3.14)

project(Peer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPONENTS_LIST Core Network)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS ${COMPONENTS_LIST})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${COMPONENTS_LIST})

add_executable(${PROJECT_NAME}
    main.cpp

    lib/abstract_serializer.h
    lib/abstract_serializer.cpp
    lib/serializable.h
    lib/serialization_context.h
    lib/serialization_context_p.h
    lib/serialization_context.cpp
    lib/message.h lib/message.cpp
    lib/message_sender.h
    lib/transceiver.h
    lib/transceiver.cpp

    msg/peer.h
    msg/coupling.h
    msg/actions.h
    msg/data.h

    comm/data_channel.h
    comm/data_channel.cpp
    comm/message_handler.h
    comm/message_router.h

    comm/actions/action_handler.h
    comm/actions/action_handler.cpp
    comm/actions/action_callbacks.h

    comm/client/communication_client.h
    comm/client/communication_client.cpp
    comm/client/client_coupling_handler.h
    comm/client/client_coupling_handler.cpp
    comm/client/client_coupling_handler_callbacks.h

    comm/server/communication_server.h
    comm/server/communication_server.cpp    
    comm/server/server_coupling_handler.h
    comm/server/server_coupling_handler.cpp
    comm/server/server_coupling_handler_callbacks.h

    data_peer.h data_peer.cpp
    client_peer.h client_peer.cpp
    server_peer.h server_peer.cpp
)

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)

target_include_directories(${PROJECT_NAME}
    PRIVATE .
)

# include(GNUInstallDirs)
# install(TARGETS Peer
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
