set(CMAKE_CXX_STANDARD 11)

add_executable(onsyuri_launcher
    main.cpp
    game_list.cpp
    options.cpp
    menu_ui.cpp
)

if(STATIC_PORT_ROOT)
    target_include_directories(onsyuri_launcher PRIVATE
        ${STATIC_PORT_ROOT}/include
    )
    target_link_directories(onsyuri_launcher PRIVATE
        ${STATIC_PORT_ROOT}/lib
    )
    target_link_libraries(onsyuri_launcher PRIVATE
        -Wl,-Bstatic
        SDL2
        SDL2_ttf
        -Wl,-Bdynamic
        pthread
        dl
    )
else()
    include(FindPkgConfig)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    target_include_directories(onsyuri_launcher PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
    )
    target_link_libraries(onsyuri_launcher PRIVATE
        ${SDL2_LINK_LIBRARIES}
        ${SDL2_TTF_LINK_LIBRARIES}
    )
endif()

