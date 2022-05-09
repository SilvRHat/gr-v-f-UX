// Multi_verse DeliveryService
// Gavin Zimmerman
// Simple Light Node

#version 410 core

// OONST
const float ambient = .4;


// INPUTS
layout(location = 0) in vec3 i_LightOffset;
layout(location = 1) in vec3 i_Norm;
layout(location = 2) in vec3 i_ViewPos;
layout(location = 3) in float i_Emission;
layout(location = 4) in float i_Shiny;

layout(location = 5) in vec4 i_DiffuseColor;
layout(location = 6) in vec2 i_UVMap;
layout(location = 7) in vec4 i_Vert;
// Here lies an unnamed dragon
layout(location = 9) in vec4 i_SpecularColor;
layout(location =10) in float i_Id;
layout(location =11) in float i_Mix;


uniform mat4 u_ViewMat;
uniform mat4 u_ProjectionMat;

uniform vec4 l_Color;
uniform float u_Time;
uniform vec2 u_Resolution;


// OUTPUTS
layout (location=0) out vec4 o_Color;


// SOURCE
vec4 rand4(in vec4 p) {
    vec4 x = vec4(7.0, 57.0, 113.0, 607.0);
    return fract(sin(vec4(
            dot(p, x.xyzw),
            dot(p, x.ywzx), 
            dot(p, x.wzyx),
            dot(p, x.wyxz)
        )) *
    43758.5453);
}

// You can make fire out of fire
void main() {
    float s = (floor(-i_Norm.g*3.)/3.)*.8+.2;
    o_Color = vec4(s)*vec4(1.0, 0.1843, 0.0, 1.0);//i_DiffuseColor;
}