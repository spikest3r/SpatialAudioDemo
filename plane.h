#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "vertex.h"

std::vector<Vertex> planeVertices = {
    {
        { -0.5f, 0.0f, -0.5f },   // Position
        {  0.0f, 1.0f,  0.0f },   // Normal
        {  0.0f, 0.0f }           // TexCoords
    },
    {
        {  0.5f, 0.0f, -0.5f },
        {  0.0f, 1.0f,  0.0f },
        {  10.0f, 0.0f }
    },
    {
        {  0.5f, 0.0f,  0.5f },
        {  0.0f, 1.0f,  0.0f },
        {  10.0f, 10.0f }
    },
    {
        { -0.5f, 0.0f,  0.5f },
        {  0.0f, 1.0f,  0.0f },
        {  0.0f, 10.0f }
    }
};

std::vector<unsigned int> planeIndices = {
    0, 1, 2,
    2, 3, 0
};