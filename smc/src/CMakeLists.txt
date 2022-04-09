cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(smc LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


set(CMAKE_CXX_STANDARD 20)



set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_PREFIX_PATH "/home/ludovic/Qt/6.2.4/gcc_64")

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL2  REQUIRED)
find_package(SDL2_IMAGE  REQUIRED) 
find_package(SDL2_TTF  REQUIRED)
find_package(SDL2_MIXER  REQUIRED)
find_package(X11 REQUIRED)
find_package(CEGUI-0  REQUIRED)
find_package(CEGUI-0-OPENGL  REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CEGUI_OPENGL_INCLUDE_DIRS})
include_directories(${CEGUI_INCLUDE_DIRS})
include_directories(${X11_INCLUDE_DIR})


add_executable(smc core/main.cpp
	net/httplib.h 
	net/httpClient.cpp 
	net/httpClient.h
	audio/audio.cpp 
	audio/audio.h 
	audio/random_sound.cpp 
	audio/random_sound.h 
	audio/sound_manager.cpp 
	audio/sound_manager.h 
	core/camera.cpp 
	core/camera.h 
	core/campaign_manager.cpp 
	core/campaign_manager.h 
	core/collision.cpp 
	core/collision.h 
	core/editor.cpp 
	core/editor.h 
	core/file_parser.cpp 
	core/file_parser.h 
	core/filesystem/filesystem.cpp 
	core/filesystem/filesystem.h 
	core/filesystem/resource_manager.cpp 
	core/filesystem/resource_manager.h 
	core/framerate.cpp 
	core/framerate.h 
	core/game_core.cpp 
	core/game_core.h 
	core/global_basic.h 
	core/global_game.h 
	core/i18n.cpp 
	core/i18n.h 
	core/main.h 
	core/math/line.h 
	core/math/point.h 
	core/math/rect.h 
	core/math/size.h 
	core/math/utilities.cpp 
	core/math/utilities.h 
	core/math/vector.cpp 
	core/math/vector.h 
	core/obj_manager.h 
	core/property_helper.cpp 
	core/property_helper.h 
	core/sprite_manager.cpp 
	core/sprite_manager.h 
	enemies/bosses/turtle_boss.cpp 
	enemies/bosses/turtle_boss.h 
	enemies/eato.cpp 
	enemies/eato.h 
	enemies/enemy.cpp 
	enemies/enemy.h 
	enemies/flyon.cpp 
	enemies/flyon.h 
	enemies/furball.cpp 
	enemies/furball.h 
	enemies/gee.cpp 
	enemies/gee.h 
	enemies/krush.cpp 
	enemies/krush.h 
	enemies/rokko.cpp 
	enemies/rokko.h 
	enemies/spika.cpp 
	enemies/spika.h 
	enemies/spikeball.cpp 
	enemies/spikeball.h 
	enemies/static.cpp 
	enemies/static.h 
	enemies/thromp.cpp 
	enemies/thromp.h 
	enemies/turtle.cpp 
	enemies/turtle.h 
	gui/generic.cpp 
	gui/generic.h 
	gui/hud.cpp 
	gui/hud.h 
	gui/menu.cpp 
	gui/menu_data.cpp 
	gui/menu_data.h 
	gui/menu.h 
	gui/spinner.cpp 
	gui/spinner.h 
	input/joystick.cpp 
	input/joystick.h 
	input/keyboard.cpp 
	input/keyboard.h 
	input/mouse.cpp 
	input/mouse.h 
	level/level_background.cpp 
	level/level_background.h 
	level/level.cpp 
	level/level_editor.cpp 
	level/level_editor.h 
	level/level.h 
	level/level_manager.cpp 
	level/level_manager.h 
	level/level_player.cpp 
	level/level_player.h 
	level/level_settings.cpp 
	level/level_settings.h 
	objects/animated_sprite.cpp 
	objects/animated_sprite.h 
	objects/ball.cpp 
	objects/ball.h 
	objects/bonusbox.cpp 
	objects/bonusbox.h 
	objects/box.cpp 
	objects/box.h 
	objects/enemystopper.cpp 
	objects/enemystopper.h 
	objects/goldpiece.cpp 
	objects/goldpiece.h 
	objects/level_entry.cpp 
	objects/level_entry.h 
	objects/level_exit.cpp 
	objects/level_exit.h 
	objects/moving_platform.cpp 
	objects/moving_platform.h 
	objects/movingsprite.cpp 
	objects/movingsprite.h 
	objects/path.cpp 
	objects/path.h 
	objects/powerup.cpp 
	objects/powerup.h 
	objects/spinbox.cpp 
	objects/spinbox.h 
	objects/sprite.cpp 
	objects/sprite.h 
	objects/star.cpp 
	objects/star.h 
	objects/text_box.cpp 
	objects/text_box.h 
	overworld/overworld.cpp 
	overworld/overworld.h 
	overworld/world_editor.cpp 
	overworld/world_editor.h 
	overworld/world_layer.cpp 
	overworld/world_layer.h 
	overworld/world_manager.cpp 
	overworld/world_manager.h 
	overworld/world_player.cpp 
	overworld/world_player.h 
	overworld/world_sprite_manager.cpp 
	overworld/world_sprite_manager.h 
	overworld/world_waypoint.cpp 
	overworld/world_waypoint.h 
	user/preferences.cpp 
	user/preferences.h 
	user/savegame.cpp 
	user/savegame.h 
	video/animation.cpp 
	video/animation.h 
	video/color.h 
	video/font.cpp 
	video/font.h 
	video/gl_surface.cpp 
	video/gl_surface.h 
	video/img_manager.cpp 
	video/img_manager.h 
	video/img_settings.cpp 
	video/img_settings.h 
	video/renderer.cpp 
	video/renderer.h 
	video/video.cpp 
	video/video.h

)



target_link_libraries(smc Qt6::Core ${X11_LIBRARIES}  ${SDL2_LIBRARIES} ${CEGUI_OPENGL_LIBRARIES} ${CEGUI_LIBRARIES} ${SDL2MIXER_LIBRARIES} ${SDL2TTF_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_IMG_LIBRAIES} ${PNG_LIBRARY}  )

if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_definitions(smc PUBLIC "_DEBUG")
message(STATUS "Add _DEBUG macro")
endif()


# copy data
 add_custom_command(
 OUTPUT
 ${CMAKE_CURRENT_BINARY_DIR}/data
 COMMAND
 ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../data
 ${CMAKE_CURRENT_BINARY_DIR}/data
 DEPENDS
 ${CMAKE_CURRENT_SOURCE_DIR}/../data
 )


 target_sources(smc
 PRIVATE
 ${CMAKE_CURRENT_BINARY_DIR}/data
)
