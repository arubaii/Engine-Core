#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "renderer_core/Mesh.h"

namespace PRIMITIVES
{
    // --- Crosshair ---
    inline const float CrosshairVertices[] = {
        // vertical line
        0.0f, -0.02f, 0.0f,
        0.0f,  0.02f, 0.0f,
        // horizontal line
       -0.015f, 0.0f, 0.0f,
        0.015f, 0.0f, 0.0f
    };

    // --- Cube ---
    inline const std::vector<Vertex> CubeVertices = {
        // x   y   z    r  g  b
        {{-1, -1, -1}, {1, 0, 0}},
        {{ 1, -1, -1}, {0, 1, 0}},
        {{ 1,  1, -1}, {0, 0, 1}},
        {{-1,  1, -1}, {1, 1, 0}},
        {{-1, -1,  1}, {1, 0, 1}},
        {{ 1, -1,  1}, {0, 1, 1}},
        {{ 1,  1,  1}, {1, 1, 1}},
        {{-1,  1,  1}, {0.5f, 0.5f, 0.5f}}
    };

    inline const std::vector<uint32_t> CubeIndices = {
        0, 1, 2, 2, 3, 0, // back
        4, 5, 6, 6, 7, 4, // front
        0, 4, 7, 7, 3, 0, // left
        1, 5, 6, 6, 2, 1, // right
        3, 2, 6, 6, 7, 3, // top
        0, 1, 5, 5, 4, 0  // bottom
    };

   inline float CubeTexCoords[] = {
        // back face (z = -1)
        0.0f, 0.0f,   // 0 bottom-left
        1.0f, 0.0f,   // 1 bottom-right
        1.0f, 1.0f,   // 2 top-right
        0.0f, 1.0f,   // 3 top-left

        // front face (z = +1)
        0.0f, 0.0f,   // 4 bottom-left
        1.0f, 0.0f,   // 5 bottom-right
        1.0f, 1.0f,   // 6 top-right
        0.0f, 1.0f    // 7 top-left
    };

    inline const float CubeNormals[] = {
        -1.0f, -1.0f, -1.0f,  // 0
         1.0f, -1.0f, -1.0f,  // 1
         1.0f,  1.0f, -1.0f,  // 2
        -1.0f,  1.0f, -1.0f,  // 3
        -1.0f, -1.0f,  1.0f,  // 4
         1.0f, -1.0f,  1.0f,  // 5
         1.0f,  1.0f,  1.0f,  // 6
        -1.0f,  1.0f,  1.0f   // 7
    };
    // --- Plane ---
    inline const float PlaneVertices[] = {
        -1.0f, 0.0f, -1.0f,
         1.0f, 0.0f, -1.0f,
         1.0f, 0.0f,  1.0f,
        -1.0f, 0.0f,  1.0f
    };

    inline const float PlaneNormals[] = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    inline const float PlaneTexCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    inline const float LightCubeVertices[] = {
        -1.0f, -1.0f, -1.0f, // 0: left,  bottom, back
         1.0f, -1.0f, -1.0f, // 1: right, bottom, back
         1.0f,  1.0f, -1.0f, // 2: right, top,    back
        -1.0f,  1.0f, -1.0f, // 3: left,  top,    back
        -1.0f, -1.0f,  1.0f, // 4: left,  bottom, front
         1.0f, -1.0f,  1.0f, // 5: right, bottom, front
         1.0f,  1.0f,  1.0f, // 6: right, top,    front
        -1.0f,  1.0f,  1.0f  // 7: left,  top,    front
    };

    inline const unsigned int LightCubeIndices[] = {
        0, 1, 2, 2, 3, 0, // back
        4, 5, 6, 6, 7, 4, // front
        0, 4, 7, 7, 3, 0, // left
        1, 5, 6, 6, 2, 1, // right
        3, 2, 6, 6, 7, 3, // top
        0, 1, 5, 5, 4, 0  // bottom
    };



    inline const unsigned int PlaneIndices[] = { 0, 1, 2, 2, 3, 0 };

	// --- Thick Vertical Line ---
	inline const float LineHalfThickness = 0.02f;
	inline const float LineHeight        = 10000.0f;


    inline std::vector<Vertex> GetyAxisVertices(float axisHeight)
    {
        return
        {
            {{0.0f, -axisHeight, 0.0f}, {0.7f, 0.7f, 0.3f}},
            {{0.0f,  axisHeight, 0.0f}, {0.7f, 0.7f, 0.3f}}   // (yellow color)
        };
    }

    inline const std::vector<uint32_t> yAxisIndices = {0, 1};

    inline const std::vector<Vertex> ScreenQuadVertices =
    {
        {{-1.0f, -1.0f, 0.0f}, {1, 1, 1}},
        {{ 1.0f, -1.0f, 0.0f}, {1, 1, 1}},
        {{ 1.0f,  1.0f, 0.0f}, {1, 1, 1}},

        {{-1.0f, -1.0f, 0.0f}, {1, 1, 1}},
        {{ 1.0f,  1.0f, 0.0f}, {1, 1, 1}},
        {{-1.0f,  1.0f, 0.0f}, {1, 1, 1}}
    };

    inline const std::vector<uint32_t> ScreenQuadIndices =
    {
        0, 1, 2,
        3, 4, 5
    };


    inline const std::vector<Vertex> LightVerts = {
        // position                 normal           tex     color
        {{-0.5f,-0.5f,-0.5f}, {0,0,0}, {0,0}, {1,1,1}},
        {{ 0.5f,-0.5f,-0.5f}, {0,0,0}, {0,0}, {1,1,1}},
        {{ 0.5f, 0.5f,-0.5f}, {0,0,0}, {0,0}, {1,1,1}},
        {{-0.5f, 0.5f,-0.5f}, {0,0,0}, {0,0}, {1,1,1}},
        {{-0.5f,-0.5f, 0.5f}, {0,0,0}, {0,0}, {1,1,1}},
        {{ 0.5f,-0.5f, 0.5f}, {0,0,0}, {0,0}, {1,1,1}},
        {{ 0.5f, 0.5f, 0.5f}, {0,0,0}, {0,0}, {1,1,1}},
        {{-0.5f, 0.5f, 0.5f}, {0,0,0}, {0,0}, {1,1,1}},
    };

    inline const std::vector<uint32_t> LightIdx = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        0,4,7, 7,3,0,
        1,5,6, 6,2,1,
        3,2,6, 6,7,3,
        0,1,5, 5,4,0
    };

    inline const float SkyboxVerts[] = {
        -1,  1, -1,  -1, -1, -1,   1, -1, -1,   1, -1, -1,   1,  1, -1,  -1,  1, -1,
        -1, -1,  1,  -1, -1, -1,  -1,  1, -1,  -1,  1, -1,  -1,  1,  1,  -1, -1,  1,
         1, -1, -1,   1, -1,  1,   1,  1,  1,   1,  1,  1,   1,  1, -1,   1, -1, -1,
        -1, -1,  1,  -1,  1,  1,   1,  1,  1,   1,  1,  1,   1, -1,  1,  -1, -1,  1,
        -1,  1, -1,   1,  1, -1,   1,  1,  1,   1,  1,  1,  -1,  1,  1,  -1,  1, -1,
        -1, -1, -1,  -1, -1,  1,   1, -1, -1,   1, -1, -1,  -1, -1,  1,   1, -1,  1
    };


    inline Mesh GenerateSphere(int stacks = 32, int slices = 32)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (int i = 0; i <= stacks; i++) {
            float v = float(i) / stacks;
            float phi = v * glm::pi<float>();

            for (int j = 0; j <= slices; j++) {
                float u = float(j) / slices;
                float theta = u * glm::two_pi<float>();

                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);

                glm::vec3 N(x, y, z);
                glm::vec3 up = (abs(N.y) < 0.999f) ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
                glm::vec3 T = glm::normalize(glm::cross(up, N));
                glm::vec3 B = glm::normalize(glm::cross(N, T));

                Vertex vert{};
                vert.Position = N;
                vert.Normal = N;
                vert.TexCoord = glm::vec2(u, v);
                vert.Tangent = T;
                vert.Bitangent = B;

                vertices.push_back(vert);
            }
        }

        for (int i = 0; i < stacks; i++) {
            for (int j = 0; j < slices; j++) {
                int row1 = i * (slices + 1);
                int row2 = (i + 1) * (slices + 1);

                indices.push_back(row1 + j);
                indices.push_back(row2 + j);
                indices.push_back(row2 + j + 1);

                indices.push_back(row1 + j);
                indices.push_back(row2 + j + 1);
                indices.push_back(row1 + j + 1);
            }
        }

        return Mesh(vertices, indices);
    }

}


