set(SOURCES
    main.cpp
    Engine/Raycaster.cpp
    Engine/Renderer.cpp
    Engine/Player.cpp
    Engine/Enemy.cpp
    Engine/EnemyManager.cpp
    Engine/SpriteRenderer.cpp
    Engine/pItemRenderer.cpp
)

add_executable(RaycasterGame ${SOURCES})

# SDL2 include directory
target_include_directories(RaycasterGame PRIVATE /opt/homebrew/include)

# SDL2 libraries (Homebrew)
find_library(SDL2_LIBRARY SDL2 PATHS /opt/homebrew/lib)
find_library(SDL2MAIN_LIBRARY SDL2main PATHS /opt/homebrew/lib)
find_library(SDL2IMAGE_LIBRARY SDL2_image PATHS /opt/homebrew/lib)  # <-- Add this

if(NOT SDL2_LIBRARY)
    message(FATAL_ERROR "SDL2 library not found. Install with: brew install sdl2")
endif()
if(NOT SDL2IMAGE_LIBRARY)
    message(FATAL_ERROR "SDL2_image library not found. Install with: brew install sdl2_image")
endif()

target_link_libraries(RaycasterGame PRIVATE ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ${SDL2IMAGE_LIBRARY})

# Automatically copy enemy.png into bin/
configure_file(
    ${CMAKE_SOURCE_DIR}/src/enemy.png
    ${CMAKE_BINARY_DIR}/enemy.png
    COPYONLY
)

# Automatically copy Pistol.png into build folder
configure_file(
    ${CMAKE_SOURCE_DIR}/src/Pistol.png
    ${CMAKE_BINARY_DIR}/Pistol.png
    COPYONLY
)

# Automatically copy Shotgun.png into build folder
configure_file(
    ${CMAKE_SOURCE_DIR}/src/Shotgun.png
    ${CMAKE_BINARY_DIR}/Shotgun.png
    COPYONLY
)
