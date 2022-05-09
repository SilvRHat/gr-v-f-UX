// Gr@v:f/UX
// Gavin:Zimmerman
// lZr // Shader Pipeline // Vertex Subdivision control
#version 410 core



// INPUTS
// Constants
const float TESS_INNER = 17.;
const float TESS_OUTER = 17.;

// Vertex
in vec4 s0_Vertex[];
in vec3 s0_Normal[];
in vec2 s0_UV[];
in mat4 s0_ModelViewMat[];
in mat3 s0_NormalMat[];
in vec4 s0_Color[];
in vec3 s0_Vel[];
in vec3 s0_Acc[];


// OUTPUTS
layout(vertices = 3) out;
out vec4 s1_Vertex[];
out vec3 s1_Normal[];
out vec2 s1_UV[];
out mat4 s1_ModelViewMat[];
out mat3 s1_NormalMat[];
out vec4 s1_Color[];
out vec3 s1_Vel[];
out vec3 s1_Acc[];



// SOURCE
void main() {
    // Vertex Data -> 
    s1_Vertex[gl_InvocationID] = s0_Vertex[gl_InvocationID];
    s1_Normal[gl_InvocationID] = s0_Normal[gl_InvocationID];
    s1_UV[gl_InvocationID] = s0_UV[gl_InvocationID];
    s1_ModelViewMat[gl_InvocationID] = s0_ModelViewMat[gl_InvocationID];
    s1_NormalMat[gl_InvocationID] = s0_NormalMat[gl_InvocationID];
    s1_Color[gl_InvocationID] = s0_Color[gl_InvocationID];
    s1_Vel[gl_InvocationID] = s0_Vel[gl_InvocationID];
    s1_Acc[gl_InvocationID] = s0_Acc[gl_InvocationID];

    // Patch Data ->
    gl_TessLevelInner[0] = TESS_INNER; 
    gl_TessLevelOuter[0] = TESS_OUTER;
    gl_TessLevelOuter[1] = TESS_OUTER;
    gl_TessLevelOuter[2] = TESS_OUTER;
}