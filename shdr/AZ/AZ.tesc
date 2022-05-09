// Gr@v:f/UX
// Gavin:Zimmerman
// AZ // AZtroid Shader - Tesselation Control
#version 410 core


// INPUTS
in mat4 v_ModelViewMat[];
in mat3 v_NormalMat[];
in vec4 v_Vertex[];
in vec3 v_Norm[];
in vec3 v_Size0[];
in vec3 v_Size1[];
in vec3 v_Pos[];
in float v_ID[];

uniform mat4 u_ProjectionMat;


// OUTPUTS
layout(vertices = 3) out;

out vec4 tc_Vertex[];
out mat4 tc_ModelViewMat[];
out mat3 tc_NormalMat[];
out vec3 tc_Size0[];
out vec3 tc_Size1[];
out vec3 tc_Pos[];
out float tc_ID[];
out float tc_TLvl[];



// SOURCE
float remap(float iMin, float iMax, float oMin, float oMax, float v) {
    return mix(oMin, oMax, (v - iMin) / (iMax - iMin) );
}
float invLerpC(float a, float b, float v) {
    return clamp(
        (v - a) / (b - a),
        0,
        1
    );
}


#define MIN_TLVL 1.
#define MAX_TLVL 14.
float getTessLevel(float dist) {
    float t = invLerpC(30.01, 700.01, dist); // [0-1]
    t = pow(t, .6);
    float tl = mix(MAX_TLVL, MIN_TLVL, t);

    return clamp(
        tl,
        MIN_TLVL,
        MAX_TLVL
    );
}


void main() {
    // Plug to next stage
    tc_Vertex[gl_InvocationID] = v_Vertex[gl_InvocationID];

    tc_ModelViewMat[gl_InvocationID] = v_ModelViewMat[gl_InvocationID];
    tc_NormalMat[gl_InvocationID] = v_NormalMat[gl_InvocationID];
    tc_Size0[gl_InvocationID] = v_Size0[gl_InvocationID];
    tc_Size1[gl_InvocationID] = v_Size1[gl_InvocationID];
    tc_Pos[gl_InvocationID] = v_Pos[gl_InvocationID];
    tc_ID[gl_InvocationID] = v_ID[gl_InvocationID];


    // Tess Levels
    float inner_tlvl = getTessLevel( length(tc_ModelViewMat[gl_InvocationID][3].xyz) );
    float outer_tlvl = inner_tlvl;

    // Cull
    vec4 s = vec4(v_Size0[gl_InvocationID]*v_Size1[gl_InvocationID], 1.);
    mat4 ndc = u_ProjectionMat * v_ModelViewMat[gl_InvocationID];
    vec3 ndc_pos = ndc[3].xyz / ndc[3].w;
    vec3 diff = (ndc * s).xyz/ndc[3].w;

    if (
        (clamp(ndc_pos, vec3(-1.5,-1.5,-.1), vec3(1.5,1.5,1.)) != ndc_pos) ||                   // Outside Camera
        (dot(v_NormalMat[gl_InvocationID] * v_Norm[gl_InvocationID], vec3(0.,0.,-1.)) > 0.6)    // Back face
    )
    {
        inner_tlvl = 0;
        outer_tlvl = 0;
    }

    tc_TLvl[gl_InvocationID] = inner_tlvl;
    gl_TessLevelInner[0] = inner_tlvl; 
    gl_TessLevelOuter[0] = outer_tlvl;
    gl_TessLevelOuter[1] = outer_tlvl;
    gl_TessLevelOuter[2] = outer_tlvl;
}