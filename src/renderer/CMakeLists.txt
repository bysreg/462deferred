set( SRCS "renderer.cpp" "camera.cpp" "Shader.cpp" "GeometryBuffer.cpp" "ShadowMap.cpp")
set( INCS "renderer.hpp" "camera.hpp" "RendererInitData.hpp" "Shader.hpp" "GeometryBuffer.hpp" "ShadowMap.hpp")

add_library(renderer ${SRCS} ${INCS})
source_group(headers FILES ${INCS})