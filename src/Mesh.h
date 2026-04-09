#pragma once
#include <vector>

struct Vertex
{
    float px, py, pz;
    float nx, ny, nz;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    static MeshData cube()
    {
        MeshData m;
        m.vertices = {
            {-1,-1,-1, 0,0,-1}, { 1,-1,-1, 0,0,-1}, { 1, 1,-1, 0,0,-1}, {-1, 1,-1, 0,0,-1},
            {-1,-1, 1, 0,0, 1}, { 1,-1, 1, 0,0, 1}, { 1, 1, 1, 0,0, 1}, {-1, 1, 1, 0,0, 1},
        };
        m.indices = {
            0,1,2, 2,3,0,
            4,6,5, 6,4,7,
            0,3,7, 7,4,0,
            1,5,6, 6,2,1,
            3,2,6, 6,7,3,
            0,4,5, 5,1,0
        };
        return m;
    }
};
