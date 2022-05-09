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
vec2 rand2(in vec2 p) {
    const vec3 a = vec3(7.0, 57.0, 113.0);
    vec2 x = vec2(
        dot(p, a.xy),
        dot(p, a.yz)
    );
    return fract(sin(x) *43758.5453);
}
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



float voronoi2d(
    in vec2 i_pos, in float i_scale, const in int i_distance_func_exp,
    out vec2 o_pos
) {
    // Evaluate point
    vec2 pos = i_pos * 2. * i_scale;
    vec2 pos_i = floor(pos);
    vec2 pos_f = fract(pos);

    float m_dist = 8.;
    vec2 pt_pos = vec2(0.);
    for (float x=-1.; x<=1.; x+=1.)
    for (float y=-1.; y<=1.; y+=1.)
    {
        vec2 neighbor = pos_i + vec2(x,y);
        vec2 pt = neighbor + rand2(neighbor);

        // Distance bewteen pixel and point
        vec2 r = pt - pos; 
        vec2 abr = abs(r);
        float d;
        if (i_distance_func_exp==1.)
            d = abr.x+abr.y;
        else if (i_distance_func_exp==2.)
            d = dot(r, r);
        else
            d = max(abr.x, abr.y);

        // Set Values
        if (d<m_dist) {
            pt_pos = pt;
            m_dist = d;
        }
        pt_pos = d<m_dist? pt: pt_pos;
        m_dist = min(d, m_dist);
    }

    if (i_distance_func_exp==2.) m_dist=sqrt(m_dist);
    o_pos = pt_pos;
    return m_dist;
}

void main() {
    //o_Color = texture(t_SNoise, i_ViewPos/23.289/4).rrra;
    vec2 cen;
    vec3 d0 = smoothstep(0, voronoi2d(i_UVMap, .5*20., 2, cen), .025) * vec3(0.8627, 0.8706, 0.7451);
    vec3 d1 = smoothstep(0, voronoi2d(i_UVMap, .5*60., 2, cen), .01) * vec3(0.9882, 1.0, 0.8941);
    vec3 d2 = smoothstep(0, voronoi2d(i_UVMap, .5*10., 2, cen), .015) * vec3(0.4667, 0.9294, 1.0);
    vec3 d3 = smoothstep(0, voronoi2d(i_UVMap, .5*300.,2, cen), .005) * vec3(0.7804, 0.0549, 1.0);
    vec3 d4 = smoothstep(0, voronoi2d(i_UVMap, .5*70., 2, cen), .015) * vec3(0.7412, 0.6196, 0.6196);

    o_Color = vec4(d0+d1+d2+d3+d4,1.);//i_DiffuseColor;
}