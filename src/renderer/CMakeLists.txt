set( SRCS "renderer.cpp" "camera.cpp" "Shader.cpp")
set( INCS "renderer.hpp" "camera.hpp" "RendererInitData.hpp" "Shader.hpp")

add_library(renderer ${SRCS} ${INCS})
source_group(headers FILES ${INCS})