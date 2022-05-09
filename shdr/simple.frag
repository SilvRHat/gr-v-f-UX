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
uniform sampler3D t_SNoise;


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

void main() {
    float lvl=3.;
    float s = mix(floor(dot(i_Norm, vec3(0.,1.,0.)) * lvl)/lvl +(1/(lvl*2)), 1., .2);

    float h = pow(fract(i_Norm/2.125).b, 10.);

    float x=1;
    //o_Color = texture(t_SNoise, i_ViewPos/23.289/4).rrra;
    o_Color = vec4(s)*vec4(0.902, 0.8392, 0.7412, 1.0) + vec4(h)*vec4(0.0, 0.5686, 1.0, 1.0);//i_DiffuseColor;
}