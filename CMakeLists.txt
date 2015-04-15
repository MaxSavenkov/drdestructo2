cmake_minimum_required(VERSION 2.8)

if ( "${CMAKE_SYSTEM_NAME}" STREQUAL "Android" )
    SET( ANDROID 1 )
endif()

# ---------------- Main project properties ----------------

project(DrDestructo2 CXX)

# ---------------- Modules ----------------

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(cotire)
include(CheckCXXSourceCompiles)

# ---------------- Direct3D ----------------

if(WIN32)
    include(FindD3D9)
    
    find_package(D3D9)
    if(D3D9_FOUND)
        include_directories(${D3D9_INCLUDE_DIR})
    endif(D3D9_FOUND)
endif(WIN32)

# ---------------- Paths ----------------

if(WIN32)
    include_directories( ${CMAKE_SOURCE_DIR}/allegro_win32/include )
    link_directories( ${CMAKE_SOURCE_DIR}/allegro_win32/lib )
    link_directories( ${CMAKE_SOURCE_DIR}/libs/win32/breakpad/ )
elseif(UNIX)
    if(ANDROID)
        include_directories( ${CMAKE_SOURCE_DIR}/allegro_ndr/include )
        link_directories( ${CMAKE_SOURCE_DIR}/allegro_ndr/lib )
    else()
        include_directories( ${CMAKE_SOURCE_DIR}/allegro_unix/include )
        link_directories( ${CMAKE_SOURCE_DIR}/allegro_unix/lib /opt/local/lib/ )
    endif()
else()
#    include_directories( ${CMAKE_SOURCE_DIR}/allegro_ems/include )
#    link_directories( ${CMAKE_SOURCE_DIR}/allegro_ems/lib )
endif(WIN32)

# ---------------- Sources ----------------

set( SOURCES_ENGINE
    src/Engine/AllegroImage5.cpp
    src/Engine/AllegroInput5.cpp
    src/Engine/AllegroRender5.cpp
    src/Engine/AllegroSound5.cpp
    src/Engine/AllegroSystem5.cpp
    src/Engine/BitmapManager.cpp
    src/Engine/LogCommon.cpp
    src/Engine/Logger.cpp 
    src/Engine/JoystickMapping.cpp
)

set( HEADERS_ENGINE
    src/Engine/AllegroImage5.h
    src/Engine/AllegroInput5.h
    src/Engine/AllegroRender5.h
    src/Engine/AllegroSound5.h
    src/Engine/AllegroSystem5.h
    src/Engine/AllegroUTFString.h
    src/Engine/BitmapManager.h
    src/Engine/IImage.h
    src/Engine/IImageManager.h
    src/Engine/IInput.h
    src/Engine/ILogger.h
    src/Engine/IRender.h
    src/Engine/ISound.h
    src/Engine/ISystem.h
    src/Engine/IUTFString.h
    src/Engine/LogCommon.h
    src/Engine/Logger.h
    src/Engine/ResourceHandle.h
    src/Engine/ResourceManager.h 
    src/Engine/JoystickMapping.h
)

set( SOURCES_GAME
    src/AICommands.cpp
    src/AICommands.h
    src/AIProcessor.cpp
    src/AIProcessor.h
    src/AIProtoManager.cpp
    src/AIProtoManager.h
    src/AchMenu.cpp
    src/AchMenu.h
    src/Achievements.cpp
    src/Achievements.h
    src/BaseAICommand.h
    src/BaseComponent.h
    src/BaseGameState.h
    src/BaseMenu.cpp
    src/BaseMenu.h
    src/BitArray2D.h
    src/CollisionData.h
    src/Common.h
    src/ComponentContainer.h
    src/ComponentID.h
    src/ControlComponent.h
    src/ControlsProtoManager.cpp
    src/ControlsProtoManager.h
    src/Game.cpp
    src/Game.h
    src/GameContext.h
    src/GameObject.h
    src/GameStateGameOverLives.cpp
    src/GameStateGameOverLives.h
    src/GameStateGameOverTime.cpp
    src/GameStateGameOverTime.h
    src/GameStateIntro.cpp
    src/GameStateIntro.h
    src/GameStateLevel.cpp
    src/GameStateLevel.h
    src/GameStateMenu.cpp
    src/GameStateMenu.h
    src/GameStateNextLevel.cpp
    src/GameStateNextLevel.h
    src/GameStatePause.cpp
    src/GameStatePause.h
    src/GameStateScoreTable.cpp
    src/GameStateScoreTable.h
    src/GameStateTutorial.cpp
    src/GameStateTutorial.h
    src/GameStateWin.cpp
    src/GameStateWin.h
    src/GameStateWinGame.cpp
    src/GameStateWinGame.h
    src/GraphicComponent.h
    src/GraphicsContainer.h
    src/GraphicsProcessor.cpp
    src/GraphicsProcessor.h
    src/GraphicsProtoManager.cpp
    src/GraphicsProtoManager.h
    src/GroundController.cpp
    src/GroundController.h
    src/InputHandler.cpp
    src/InputHandler.h
    src/LevelDirector.cpp
    src/LevelDirector.h
    src/MechanicComponent.h
    src/MechanicsProtoManager.cpp
    src/MechanicsProtoManager.h
    src/MenuHelpers.cpp
    src/MenuHelpers.h
    src/Music.cpp
    src/Music.h
    src/ObjectProtoManager.cpp
    src/ObjectProtoManager.h
    src/ObjectsStorage.h
    src/OptionsMenu.cpp
    src/OptionsMenu.h
    src/PhysicComponent.h
    src/PhysicsContainer.h
    src/PhysicsProcessor.cpp
    src/PhysicsProcessor.h
    src/PhysicsProtoManager.cpp
    src/PhysicsProtoManager.h
    src/Player.h
    src/PlayerProfile.cpp
    src/PlayerProfile.h
    src/PrototypeManager.h
    src/Random.cpp
    src/Random.h
    src/ReflectionController.cpp
    src/ReflectionController.h
    src/SelectLevelMenu.cpp
    src/SelectLevelMenu.h
    src/SkyController.cpp
    src/SkyController.h
    src/SoundComponent.h
    src/SoundProcessor.cpp
    src/SoundProcessor.h
    src/SoundProtoManager.cpp
    src/SoundProtoManager.h
    src/SweepSpheres.cpp
    src/SweepSpheres.h
    src/TextManager.cpp
    src/TextManager.h
    src/TextPrinter.cpp
    src/TextPrinter.h
    src/UIHelpers.h
    src/Vars.cpp
    src/Vars.h
    src/Vector.h
    src/XMLDeserializer.cpp
    src/XMLDeserializer.h
    src/main.cpp
    src/stdafx.cpp
    src/stdafx.h
)

set( RES_FILES 
    src/win_specific/DrDestructo2.rc
    src/win_specific/resource.h
)

set( DOCS
    docs/ReadMe.txt
    docs/ReadMe_Source.txt
)

set( DATA    
    data/Protos/ai.lua
    data/Protos/controls.xml
    data/Protos/gm.xml
    data/Protos/graphics.xml
    data/Protos/physics.xml
    data/Protos/protos.xml
    data/Protos/sounds.xml
    data/Protos/vars.xml 
)

set( ALL_SOURCES ${SOURCES_GAME} )
LIST( APPEND ALL_SOURCES ${SOURCES_ENGINE} )
LIST( APPEND ALL_SOURCES ${HEADERS_ENGINE} )
LIST( APPEND ALL_SOURCES ${DOCS} )
LIST( APPEND ALL_SOURCES ${DATA} )

# ---------------- Visual Studio ----------------

source_group("Game\\" REGULAR_EXPRESSION src/.*)
source_group("Engine" REGULAR_EXPRESSION src/Engine/)
source_group("Game\\Components" REGULAR_EXPRESSION src/.*Component.*)
source_group("Game\\Components" FILES src/CollisionData.h src/GameObject.h src/Music.cpp src/Music.h src/ObjectsStorage.h src/Player.h src/GraphicsContainer.h src/PhysicsContainer.h)
source_group("Game\\States" REGULAR_EXPRESSION src/.*State)
source_group("Game\\Menu" REGULAR_EXPRESSION src/.*Menu)
source_group("Game\\Processor" REGULAR_EXPRESSION src/.*Processor)
source_group("Game\\Prototypes" REGULAR_EXPRESSION src/.*Manager)
source_group("precompiled" REGULAR_EXPRESSION src/.*stdafx)
source_group("Data" FILES ${DATA})
source_group("" FILES src/main.cpp)
source_group("" FILES ${DOCS})

# ---------------- Includes ----------------

include_directories(${CMAKE_SOURCE_DIR}/src/Engine/)
include_directories(${CMAKE_SOURCE_DIR}/src/tinyxml/)
include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${CMAKE_SOURCE_DIR}/src/boost/)
include_directories(${CMAKE_SOURCE_DIR}/src/luabind/)
include_directories(${CMAKE_SOURCE_DIR}/src/lua/)
include_directories(${CMAKE_SOURCE_DIR}/src/FastDelegate/)

include_directories(${CMAKE_SOURCE_DIR}/libs/win32/breakpad/)

# ---------------- Externals ----------------

if( ANDROID )
    SET( CMAKE_CXX_FLAGS "-frtti -fexceptions" )
endif()

add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/src/tinyxml )
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/src/lua )
add_subdirectory( ${CMAKE_SOURCE_DIR}/libs/src/luabind )

# ---------------- Targets ----------------

check_cxx_source_compiles(
"#include <memory>
 int main() { std::shared_ptr<int> ptr; return 0; }"
HAS_STD_SHARED_PTR)

if(WIN32)
    add_executable( DrDestructo2 WIN32 ${ALL_SOURCES} ${RES_FILES} )
elseif(APPLE)
    macro( COLLECT_RESOURCES curDir )
    file( GLOB DIR_FILES RELATIVE "${CMAKE_SOURCE_DIR}/data/${curDir}" "${CMAKE_SOURCE_DIR}/data/${curDir}/*" )
    
    foreach( FILETMP ${DIR_FILES} )
	    if( IS_DIRECTORY ${CMAKE_SOURCE_DIR}/data/${curDir}/${FILETMP} )
		    COLLECT_RESOURCES( "${curDir}/${FILETMP}" )
	    else()
	        set_source_files_properties( "data/${curDir}/${FILETMP}" PROPERTIES MACOSX_PACKAGE_LOCATION "MacOS/Data/${curDir}/" )
	        list( APPEND RESOURCE_FILES "data/${curDir}/${FILETMP}" )
	    endif()
    endforeach()

    endmacro()

    COLLECT_RESOURCES( "" )
    
    set_source_files_properties( "data/dd2_icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources" )

    add_executable( DrDestructo2 MACOSX_BUNDLE ${ALL_SOURCES} ${RESOURCE_FILES} )
else(WIN32)
    if( ANDROID )
        SET( CMAKE_ANDROID_GUI YES )
        list( APPEND ALL_SOURCES "${CMAKE_SOURCE_DIR}/android/AndroidManifest.xml" "${CMAKE_SOURCE_DIR}/android/src/org/zxstudio/drdestructo2/Activity.java" )
    endif()

    add_executable( DrDestructo2 ${ALL_SOURCES} )
endif(WIN32)


set_target_properties( DrDestructo2 PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
add_definitions( -DTIXML_USE_STL )
if(HAS_STD_SHARED_PTR)
    add_definitions( -DHAS_STD_SHARED_PTR )
endif(HAS_STD_SHARED_PTR)

add_custom_command( TARGET DrDestructo2 PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/Data )

if(WIN32)
    add_custom_command( TARGET DrDestructo2 POST_BUILD COMMAND COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/allegro_win32/bin/ ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} )

    target_link_libraries( DrDestructo2 
        allegro_monolith.lib 
        tinyxml
        lua51 
        luabind 
        exception_handler.lib
        crash_generation_client.lib
        crash_report_sender.lib
        Wininet.lib 
        common.lib
        )
elseif(APPLE)
    find_library(FRAMEWORK_APPKIT AppKit)
    find_library(FRAMEWORK_IOKIT IOKit)
    find_library(FRAMEWORK_AGL AGL)
    find_library(FRAMEWORK_OGL OpenGL)
    find_library(FRAMEWORK_AUDIO AudioToolbox)
    find_library(FRAMEWORK_OPENAL OpenAL)
    find_library(LIB_DUMB libdumb.a)
    find_library(LIB_OGG libogg.a)
    find_library(LIB_VORBIS libvorbis.a)
    find_library(LIB_VORBISFILE libvorbisfile.a)
    find_library(LIB_VORBISENC libvorbisenc.a)
    find_library(LIB_FREETYPE libfreetype.a)
    find_library(LIB_PNG libpng.a)
    find_library(LIB_BZ2 libbz2.a)
    find_library(LIB_Z libz.a)


    # Static Allegro build
    target_link_libraries( DrDestructo2 
        allegro_monolith-static 
        tinyxml
        lua51 
        luabind 
        pthread 
        ${LIB_DUMB}
        ${LIB_VORBISFILE}
        ${LIB_VORBIS}
        ${LIB_OGG}
        ${LIB_VORBISENC}
        ${LIB_FREETYPE}
        ${LIB_PNG}
        ${LIB_BZ2}
        ${LIB_Z}
        ${FRAMEWORK_APPKIT}
        ${FRAMEWORK_IOKIT}
        ${FRAMEWORK_AGL}
        ${FRAMEWORK_OGL}
        ${FRAMEWORK_AUDIO}
        ${FRAMEWORK_OPENAL}
        )    
else()
    if ( ANDROID )
        # Dynamic Allegro build
        target_link_libraries( DrDestructo2 
            allegro_monolith 
            tinyxml
            lua51 
            luabind 
            )
    else()
        # Static Allegro build
        target_link_libraries( DrDestructo2 
            allegro_monolith-static 
            tinyxml
            lua51 
            luabind 
            pthread 
            Xcursor 
            X11
            Xi
            GL
            png
            asound
            dumb
            vorbisfile
            vorbis
            ogg
            vorbisenc
            freetype
            Xrandr
            Xinerama
            )
    endif()
endif(WIN32)

# ---------------- Packaging ----------------

SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "DrDestructo2")
INCLUDE(InstallRequiredSystemLibraries)

if(WIN32)
    SET( INSTALLED_LIBS ${CMAKE_SOURCE_DIR}/allegro_win32/bin/allegro_monolith-5.1.dll )
else(WIN32)
    #SET( INSTALLED_LIBS ${CMAKE_SOURCE_DIR}/allegro_unix/lib/liballegro_monolith.so.5.1.10 )
endif(WIN32)

SET( INSTALLED_DOCS ${CMAKE_SOURCE_DIR}/docs/ReadMe.txt ${CMAKE_SOURCE_DIR}/docs/LICENSE-ART.txt ${CMAKE_SOURCE_DIR}/docs/LICENSE-CODE.txt )

SET( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/docs/LICENSE-CODE.txt )

INSTALL(TARGETS DrDestructo2 DESTINATION "DrDestructo2" )

if ( NOT APPLE )
    INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION "DrDestructo2/Data" )
endif()
    
INSTALL(FILES ${INSTALLED_DOCS} DESTINATION "DrDestructo2" )
INSTALL(FILES ${INSTALLED_LIBS} DESTINATION "DrDestructo2" )

SET(CPACK_GENERATOR "ZIP")
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0 )
SET(CPACK_PROJECT_CONFIG_FILE ${CMAKE_SOURCE_DIR}/CPackOptions.cmake )
SET(CPACK_PACKAGE_NAME "Return of Dr. Destructo")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Remake of Island of Dr. Destructo")
SET(CPACK_PACKAGE_VENDOR "ZX Studio")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/docs/ReadMe.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
#SET(CPACK_PACKAGE_VERSION_PATCH "")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Return of Dr. Destructo")
SET(CPACK_PACKAGE_EXECUTABLES "DrDestructo2" "Return of Dr. Destructo")
SET(CPACK_PACKAGE_FILE_NAME "DrDestructo2_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}${CPACK_PACKAGE_VERSION_PATCH}")
SET( CPACK_PACKAGE_CONTACT "Max Savenkov (maxed@zxstudio.org)" )


# NSIS-specifics:
SET( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )
SET( CPACK_NSIS_DISPLAY_NAME "Return of Dr. Destructo" )
SET( CPACK_NSIS_URL_INFO_ABOUT "http://zxstudio.org" )
SET( CPACK_NSIS_EXECUTABLES_DIRECTORY "DrDestructo2" )
SET( CPACK_NSIS_MUI_FINISHPAGE_RUN ON )

# RPM-specifics
SET( CPACK_RPM_PACKAGE_NAME "drdestructo2" )
SET( CPACK_RPM_PACKAGE_LICENSE "Code: MIT Art: CC-BY-SA" )
SET( CPACK_RPM_PACKAGE_URL "http://zxstudio.org/projects/drdestructo/" )
SET( CPACK_RPM_PACKAGE_RELOCATABLE ON )
SET( CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )

# DEB-specifics
SET( CPACK_DEBIAN_PACKAGE_NAME "drdestructo2" )
SET( CPACK_DEBIAN_PACKAGE_SECTION "games" )
SET( CPACK_DEBIAN_PACKAGE_URL "http://zxstudio.org/projects/drdestructo/" )
SET( CPACK_DEBIAN_PACKAGE_ARCHITECTURE_SYSTEM_PROCESSOR i386 )

# DMG-specifics
SET( MACOSX_BUNDLE_ICON_FILE "dd2_icon.icns" )

include(CPack)