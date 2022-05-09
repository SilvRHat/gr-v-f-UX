// Gr@v:f/UX
// Gavin:Zimmerman
// lZr // Shader Pipeline // Vertex Realization
#version 410 core

// INPUTS
// Reserved Mesh Vertex Attribs
layout(location = 0) in vec4 v_Vertex;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_UV;

// Reserved Instanced vertex attribs
layout(location = 3) in mat4 i_ModelViewMat;
layout(location = 7) in mat3 i_NormalMat;
layout(location =10) in vec3 i_Size;
layout(location =11) in vec4 i_Color;

// Node-special Instaced vertex attribs
layout(location =12) in vec3 n_Vel;
layout(location =13) in vec3 n_Acc;


// OUTPUTS
out vec4 s0_Vertex;
out vec3 s0_Normal;
out vec2 s0_UV;
out mat4 s0_ModelViewMat;
out mat3 s0_NormalMat;
out vec4 s0_Color;
out vec3 s0_Vel;
out vec3 s0_Acc;


// SOURCE
void main() {
    // Vert -> Tesselation
    s0_Vertex = (v_Vertex - vec4(0.,0.,.5,0.)) * vec4(i_Size,1.);
    s0_Normal = v_Normal;
    s0_UV = v_UV;
    s0_ModelViewMat = i_ModelViewMat;
    s0_NormalMat = i_NormalMat;
    s0_Color = i_Color;
    s0_Vel = n_Vel;
    s0_Acc = n_Acc;
}