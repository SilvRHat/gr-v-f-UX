// Homework 7
// Gavin Zimmerman
// Final Post-Process Stage

#version 410 core


// INPUTS
uniform sampler2D t_Pass0;  // Regular Scene
uniform vec2 u_Resolution;
uniform float u_Time;


// OUTPUTS
layout (location=0) out vec4 o_Color;

vec4 hash4(in vec4 p) {
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
    vec2 st = gl_FragCoord.st / u_Resolution;
    vec4 color = texture(t_Pass0, st);
    vec4 noise = hash4(vec4(st.x*503, st.y*102, 0., 0.));
    o_Color = (
        (color + (noise*.25*(color+.3))) * vec4(1.15,1.,1.,1.)
    );
}