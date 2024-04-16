#pragma once

#include <vector>

#include <Render/mesh.h>

namespace gdp1 {

class LODLevel {
public:
    LODLevel(int lod_index, Mesh* mesh)
        : lod_index(lod_index)
        , mesh(mesh) {}

    int lod_index;
    Mesh* mesh;
};

}  // namespace gdp1