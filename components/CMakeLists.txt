add_library(Cel 
    cel/celfile.cpp cel/celfile.h  
    cel/celframe.h cel/celframe.cpp
    cel/pal.cpp cel/pal.h 
    cel/tiledecoding.cpp cel/tiledecoding.h 
    cel/celdecoding.cpp cel/celdecoding.h
    cel/cl2decoding.cpp cel/cl2decoding.h)

add_library(FAIO faio/faio.cpp faio/faio.h)
target_link_libraries(FAIO StormLib ${Boost_LIBRARIES})
set_target_properties(FAIO PROPERTIES COMPILE_FLAGS "${FA_COMPILER_FLAGS}")

add_library(Levels 
    level/dun.cpp  
    level/dun.h
    level/min.cpp  
    level/min.h  
    level/tileset.cpp  
    level/tileset.h
    level/level.h
    level/level.cpp
    level/sol.cpp
    level/sol.h)
target_link_libraries(Levels FAIO)
set_target_properties(Levels PROPERTIES COMPILE_FLAGS "${FA_COMPILER_FLAGS}")

add_library(Misc 
    misc/stringops.h
    misc/helper2d.h
    misc/md5.h
    misc/md5.cpp
    misc/fareadini.h
    misc/fareadini.cpp
    misc/misc.h
    misc/misc.cpp
    misc/disablewarn.h
    misc/enablewarn.h
)
SET_TARGET_PROPERTIES(Misc PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(Misc PROPERTIES COMPILE_FLAGS "${FA_COMPILER_FLAGS}")


set(RenderFiles 
    render/render.h 
    render/levelobjects.cpp 
    render/levelobjects.h
    render/rocketglue/RenderInterfaceSDL2.cpp  
    render/rocketglue/RenderInterfaceSDL2.h 
    render/rocketglue/SystemInterfaceSDL2.cpp
    render/rocketglue/SystemInterfaceSDL2.h
    render/rocketglue/FAIOFileInterface.h
    render/rocketglue/FAIOFileInterface.cpp
    render/sdl2backend.cpp
)
add_library(Render ${RenderFiles})
target_link_libraries(Render Cel Levels ${SDL2_LIBRARY} ${SDL2IMAGE_LIBRARY} ${OPENGL_LIBRARIES})
set_target_properties(Render PROPERTIES COMPILE_FLAGS "${FA_COMPILER_FLAGS}")

add_library(Input
    input/inputmanager.h
    input/sdl2backend.cpp)
set_target_properties(Input PROPERTIES COMPILE_FLAGS "${FA_COMPILER_FLAGS}")

add_library(DiabloExe
    diabloexe/diabloexe.h
    diabloexe/diabloexe.cpp
    diabloexe/monster.h
    diabloexe/monster.cpp
    diabloexe/npc.h
    diabloexe/npc.cpp)
target_link_libraries(DiabloExe ${Boost_Libraries} Misc)
set_target_properties(DiabloExe PROPERTIES COMPILE_FLAGS "${FA_COMPILER_FLAGS}")
