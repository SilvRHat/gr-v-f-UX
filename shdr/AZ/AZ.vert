// Gr@v:f/UX
// Gavin:Zimmerman
// AZ // AZtroid Shader - Vertex
#version 410 core


// INPUTS
// Reserved Mesh Vertex Attribs
layout(location = 0) in vec4 i_Vertex;
layout(location = 1) in vec3 i_Norm;

// Reserved Instanced vertex attribs
layout(location = 3) in mat4 i_ModelViewMat;
layout(location = 7) in mat3 i_NormalMat;
layout(location =10) in vec3 i_Size0;

// Node-special Instaced vertex attribs
layout(location =12) in vec3 i_Size1;
layout(location =13) in vec3 i_Pos;
layout(location =14) in float i_ID;


// OUTPUTS
out mat4 v_ModelViewMat;
out mat3 v_NormalMat;
out vec4 v_Vertex;
out vec3 v_Norm;
out vec3 v_Size0;
out vec3 v_Size1;
out vec3 v_Pos;
out float v_ID;


// SOURCE
void main() {
    // Plug to next stage
    v_Vertex = i_Vertex;
    v_ModelViewMat = i_ModelViewMat;
    v_NormalMat = i_NormalMat;
    v_Norm = i_Norm;
    v_Size0 = i_Size0;
    v_Size1 = i_Size1;
    v_Pos = i_Pos;
    v_ID = i_ID;
}