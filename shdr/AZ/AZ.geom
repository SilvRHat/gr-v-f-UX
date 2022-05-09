// Gr@v:f/UX
// Gavin:Zimmerman
// AZ // AZtroid Shader - Geometry Processing
#version 410 core


// INPUTS
layout(triangles) in;

// OUTPUTS
in vec4 te_Vertex[3];
in mat4 te_ModelViewMat[3];
in mat3 te_NormalMat[3];
in vec3 te_Size[3];
in vec3 te_Norm[3];
in vec3 te_Pos[3];
in float te_ID[3];

in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
} gl_in[];

uniform mat4 l_LightViewMat;


// OUTPUTS
layout(triangle_strip, max_vertices = 3) out;

layout(location=0) out vec3 o_LightOffset;
layout(location=1) out vec3 o_Norm;
layout(location=2) out vec3 o_Pos;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};


// SOURCE
float remap(float iMin, float iMax, float oMin, float oMax, float v) {
    return mix(oMin, oMax, (v - iMin) / (iMax - iMin) );
}

void main() {
    // New Position
    vec3 A = (te_Vertex[2] - te_Vertex[0]).xyz;
    vec3 B = (te_Vertex[1] - te_Vertex[0]).xyz;
    vec3 N = normalize(cross(A, B));

    o_Norm = te_NormalMat[0] * te_Norm[0];
    o_Pos = gl_in[0].gl_Position.xyz;
    o_LightOffset = vec3(l_LightViewMat[3].xyz - o_Pos);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    o_Norm = te_NormalMat[1] * te_Norm[1];
    o_Pos = gl_in[1].gl_Position.xyz;
    o_LightOffset = vec3(l_LightViewMat[3].xyz - o_Pos);
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    o_Norm = te_NormalMat[2] * te_Norm[2];
    o_Pos = gl_in[2].gl_Position.xyz;
    o_LightOffset = vec3(l_LightViewMat[3].xyz - o_Pos);
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}