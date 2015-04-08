set( SRCS "scene.cpp" "objmodel.cpp" "Model.cpp")
set( INCS "scene.hpp" "objmodel.hpp" "Model.hpp" "Vertex.hpp")

add_library(scene ${SRCS} ${INCS})
source_group(headers FILES ${INCS})