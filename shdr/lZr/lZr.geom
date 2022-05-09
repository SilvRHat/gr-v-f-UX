// Gr@v:f/UX
// Gavin:Zimmerman
// lZr // Shader Pipeline // Geometry
#version 410 core



// INPUTS
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec3 s2_tescoord[];
in vec2 s2_UV[];
in float s2_ID[];

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];


// OUTPUTS
layout(location = 0) out vec3 o_tesscoord;
layout(location = 1) out float o_ID;
layout(location = 2) out vec2 o_UV;

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

// SOURCE
void main() {  
    gl_Position = gl_in[0].gl_Position;
    o_tesscoord =  s2_tescoord[0];
    o_UV = s2_UV[0];
    o_ID = s2_ID[0];
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    o_tesscoord =  s2_tescoord[1];
    o_ID = s2_ID[1];
    o_UV = s2_UV[1];
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    o_tesscoord =  s2_tescoord[2];
    o_ID = s2_ID[2];
    o_UV = s2_UV[2];
    EmitVertex();
}