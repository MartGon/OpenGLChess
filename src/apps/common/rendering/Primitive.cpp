#include <Primitive.h>

#include <math/BBMath.h>
#include <glm/gtc/constants.hpp>

using namespace Rendering;

Rendering::Mesh Primitive::GenerateCube()
{
    Rendering::Mesh cube;
    GL::VertexArray& cubeVao = cube.GetVAO();
    cubeVao.GenVBO(std::vector<float>
    {   
        // Down
        -0.5, -0.5, -0.5,
        0.5, -0.5, -0.5, 
        -0.5, -0.5, 0.5,               
        0.5, -0.5, 0.5,         

        // Up
        -0.5, 0.5, -0.5,        
        -0.5, 0.5, 0.5,         
        0.5, 0.5, -0.5,         
        0.5, 0.5, 0.5,          

        // Front
        -0.5, 0.5, 0.5,         
        -0.5, -0.5, 0.5,        
        0.5, 0.5, 0.5,          
        0.5, -0.5, 0.5,         

        // Back
        -0.5, 0.5, -0.5,
        0.5, 0.5, -0.5,
        -0.5, -0.5, -0.5,                
        0.5, -0.5, -0.5,        

        // Left
        -0.5, -0.5, -0.5,
        -0.5, -0.5, 0.5,        
        -0.5, 0.5, -0.5,            
        -0.5, 0.5, 0.5,        

        // Right
        0.5, -0.5, -0.5,      
        0.5, 0.5, -0.5,       
        0.5, -0.5, 0.5,       
        0.5, 0.5, 0.5,        
    }, 3);
    cubeVao.GenVBO(std::vector<float>{
        // Down
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,

        // Up
        0.0, 1.0, 0.0, 
        0.0, 1.0, 0.0, 
        0.0, 1.0, 0.0, 
        0.0, 1.0, 0.0, 

        // Front
        0.0, 0.0, 1.0, 
        0.0, 0.0, 1.0, 
        0.0, 0.0, 1.0, 
        0.0, 0.0, 1.0, 

        // Back
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,

        // Left
        1.0, 0.0, 0.0, 
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,
        -1.0, 0.0, 0.0,

        // Right
        1.0, 0.0, 0.0,  
        -1.0, 0.0, 0.0,  
        -1.0, 0.0, 0.0,  
        -1.0, 0.0, 0.0,                            
    }, 3);
    cubeVao.GenVBO(std::vector<float>{
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    }, 2);
    cubeVao.SetIndices({
        // Down
        0, 1, 2,
        3, 2, 1,

        // Up
        4, 5, 6,
        7, 6, 5,

        // Front
        8, 9, 10,
        11, 10, 9,

        // Back
        12, 13, 14,
        15, 14, 13,

        // Right
        16, 17, 18,
        19, 18, 17,

        // Left
        20, 21, 22,
        23, 22, 21,

    });

    return cube;
}

Rendering::Mesh Primitive::GenerateSlope()
{
    Rendering::Mesh slope;
    GL::VertexArray& slopeVao = slope.GetVAO();
    slopeVao.GenVBO(std::vector<float>
    {
        // Down
        -0.5, -0.5, -0.5,
        0.5, -0.5, -0.5,       
        -0.5, -0.5, 0.5,                
        0.5, -0.5, 0.5,

        // Back
        -0.5, 0.5, -0.5,
        0.5, 0.5, -0.5,           
        -0.5, -0.5, -0.5,             
        0.5, -0.5, -0.5,

        // Slope
        -0.5, -0.5, 0.5,
        0.5, -0.5, 0.5,
        -0.5, 0.5, -0.5,
        0.5, 0.5, -0.5,

        // Left
        -0.5, -0.5, -0.5,
        -0.5, -0.5, 0.5,
        -0.5, 0.5, -0.5,

        // Right
        0.5, -0.5, -0.5,
        0.5, 0.5, -0.5,
        0.5, -0.5, 0.5,
    }, 3);
    slopeVao.GenVBO(std::vector<float>{
        // Down
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, -1.0, 0.0,

        // Back
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,
        0.0, 0.0, -1.0,

        // Slope
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        // Left
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // Right
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
    }, 3);
    slopeVao.GenVBO(std::vector<float>{

        // Down
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        // Back
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        // Slope
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        // Left
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        // Right
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    }, 2);
    slopeVao.SetIndices({
        // Base
        0, 1, 2,
        3, 2, 1,

        // Back
        4, 5, 6,
        7, 6, 5,

        // Slope
        8, 9, 10,
        11, 10, 9,

        // Left
        12, 13, 14,

        // Right
        15, 16, 17
    });

    return slope;
}

Rendering::Mesh Primitive::GenerateCircle(float radius, unsigned int samples)
{
    Rendering::Mesh circle;
    GL::VertexArray& circleVao = circle.GetVAO();

    const glm::vec3 center{0.0f, 0.0f, 0.0f};
    std::vector<glm::vec3> vertices = {center};
    std::vector<unsigned int> indices;

    const float step = glm::two_pi<float>() / (float)samples;
    for(unsigned int i = 0; i < samples; i++)
    {
        auto angle = i * step;
        auto vertex = glm::vec3{glm::cos(angle), glm::sin(angle), 0.0f} * radius;

        vertices.push_back(vertex);

        // Center
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // Last vertex
    indices.push_back(0);
    indices.push_back(samples);
    indices.push_back(1);

    circleVao.GenVBO(vertices, 3);
    circleVao.SetIndices(indices);

    return circle;
}
