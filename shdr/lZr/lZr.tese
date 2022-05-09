// Gr@v:f/UX
// Gavin:Zimmerman
// lZr // Shader Pipeline // Vertex Evaluation
#version 410 core



// INPUTS
layout(triangles, equal_spacing) in;


in vec4 s1_Vertex[];
in vec3 s1_Normal[];
in vec2 s1_UV[];
in mat4 s1_ModelViewMat[];
in mat3 s1_NormalMat[];
in vec4 s1_Color[];
in vec3 s1_Vel[];
in vec3 s1_Acc[];

uniform mat4 u_ProjectionMat;
uniform float u_Time;

// OUTPUTS
out vec3 s2_tescoord;
out vec2 s2_UV;
out float s2_ID;


out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

vec3 getPosAt(float t) {
    float x = (t*5.)/(2*3.14);
    return vec3(10*cos(t), 5*sin(t/3.) + sin(x*8), 8*sin(t) + 2*cos(x*2));
}

// SOURCE
void main() {
    // Position
    vec4 size = vec4(1.,1.,10.,1.);
    vec4 vertex = (
        gl_TessCoord.x * s1_Vertex[0] +
        gl_TessCoord.y * s1_Vertex[1] +
        gl_TessCoord.z * s1_Vertex[2]
    ) * size;
    s2_UV = vertex.xy*(vertex.z);
    s2_ID = gl_PrimitiveID;

    // Seperate components
    float t = (u_Time*2.1)+(vertex.z/2.);    // Use z as time component
    vec2 xy = vertex.xy;

    // Get position 
    vec3 c0 = getPosAt(t);
    vec3 c1 = getPosAt(t-.05);
    vec3 d = normalize(c0-c1);

    // Get plane perpendicular to travel
    vec3 u = cross(d, vec3(0.,1.,0.));
    vec3 v = cross(d, u);

    // Scale
    float sc = 1-pow(vertex.z/size.z,2.);

    // Combine new position with former xy offsets
    vec3 pos = c0+u*xy.x*sc +v*xy.y*sc;

    // Pass some data
    s2_tescoord=gl_TessCoord.xyz;
    gl_Position = u_ProjectionMat * s1_ModelViewMat[0] * vec4(pos,1.);
}