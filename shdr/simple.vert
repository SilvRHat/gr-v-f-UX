// Gr@v:f/UX
// Gavin:Zimmerman
// Simple Vertex Node

#version 410 core

// INPUTS
// Vertex Attribs
layout(location = 0) in vec4 v_Vertex;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_UV;

// Reserved Instanced vertex attribs
layout(location = 3) in mat4 i_ModelViewMat;
layout(location = 7) in mat3 i_NormalMat;
layout(location =10) in vec3 i_Size;
layout(location =11) in vec4 i_DiffuseColor;

// Node-special Instaced vertex attribs
layout(location =12) in vec4  n_SpecularColor;
layout(location =13) in float n_Mix;
layout(location =14) in vec2 n_ShinyEmission;
//GEN (location =15) in int gl_InstanceId




// Uniforms
uniform mat4 u_ViewMat;
uniform mat4 u_ProjectionMat;
uniform vec4 u_ClipPlane[6];
uniform vec2 u_Resolution;
uniform vec2 u_Mouse;
uniform float u_Time;

// Light Uniforms
uniform mat4 l_LightViewMat;




// OUTPUTS
layout(location = 0) out vec3 o_LightOffset;
layout(location = 1) out vec3 o_Norm;
layout(location = 2) out vec3 o_ViewPos;
layout(location = 3) out float o_Emission;
layout(location = 4) out float o_Shiny;

layout(location = 5) out vec4 o_DiffuseColor;
layout(location = 6) out vec2 o_UVMap;
layout(location = 7) out vec4 o_Vert;
// Here lies an unnamed dragon
layout(location = 9) out vec4 o_SpecularColor;
layout(location =10) out float o_Id;
layout(location =11) out float o_Mix;





out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};


// SOURCE
void main() {
    // Vertex Lighting Calculations
    vec4 vpos = i_ModelViewMat * (v_Vertex* vec4(i_Size,1));
    
    // Vertex -> Frag
    o_LightOffset = vec3(l_LightViewMat[3] - vpos);
    o_Norm = i_NormalMat * v_Normal;
    o_ViewPos = -vpos.xyz;

    o_Shiny = n_ShinyEmission.x;
    o_Emission = n_ShinyEmission.y;
    o_Mix = n_Mix;

    o_DiffuseColor = i_DiffuseColor;
    o_SpecularColor = n_SpecularColor;
    o_UVMap = v_UV;
    o_Vert = v_Vertex * vec4(i_Size, 1);
    o_Id = float(gl_InstanceID);
    o_Mix = n_Mix;
    
    // Core Outputs
    gl_Position = u_ProjectionMat * vpos + 
        vec4(0.,0.,1e-4*(gl_InstanceID %7 ),0.);
}