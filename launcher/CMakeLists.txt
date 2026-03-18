set(CMAKE_CXX_STANDARD 11)

add_executable(onsyuri_launcher
    main.cpp
    game_list.cpp
    options.cpp
    menu_ui.cpp
    resource_pak.cpp
    resource_loader.cpp
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
        SDL2_image
        -Wl,-Bdynamic
        pthread
        dl
    )
else()
    include(FindPkgConfig)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    target_include_directories(onsyuri_launcher PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
    )
    target_link_libraries(onsyuri_launcher PRIVATE
        ${SDL2_LINK_LIBRARIES}
        ${SDL2_TTF_LINK_LIBRARIES}
        ${SDL2_IMAGE_LINK_LIBRARIES}
    )
endif()

add_executable(onsyuri_pak_builder
    pak_builder.cpp
    resource_pak.cpp
)

