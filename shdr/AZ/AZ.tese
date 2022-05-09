// Gr@v:f/UX
// Gavin:Zimmerman
// AZ // AZtroid Shader - Tesselation Evaluation
#version 410 core


// CONF
#define NORM_SAMPLE 3
#define NORM_RADIUS .1
#define HEIGHT_MULT 2.


// INPUTS
layout(triangles, equal_spacing) in;


in vec4 tc_Vertex[];
in mat4 tc_ModelViewMat[];
in mat3 tc_NormalMat[];
in vec3 tc_Size0[];
in vec3 tc_Size1[];
in vec3 tc_Pos[];
in float tc_ID[];
in float tc_TLvl[];

uniform mat4 u_ProjectionMat;
uniform float u_Time;


// OUTPUTS
out vec4 te_Vertex;
out mat4 te_ModelViewMat;
out mat3 te_NormalMat;
out vec3 te_Size;
out vec3 te_Norm;
out vec3 te_Pos;
out float te_ID;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};


// SOURCE
float ridgedMultiFractal(vec3 point, float H, float lacunarity, float frequency, const float octaves, float offset, float gain);

// Credit: https://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
#define TO_RAD 3.1415/180.
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


vec3 AZtroidPos(vec3 lpos, vec3 wpos, vec3 norm, vec3 size0) {
    float h = ridgedMultiFractal(
        wpos,   // point - Eval pos
        0.8,    // H - Dimension; Diff of magnitude between to octaves
        .76,    // lacunarity - Lacunarity; Diff of scale between octaves (interesting when below 1.)
        0.6,   // frequency - Scale
        7.5,   // octaves - Detail
        1.0,    // offset - Offset
        6.1     // gain - Gain
    );
    return (norm*h*HEIGHT_MULT +lpos) / (size0+(28.*HEIGHT_MULT));
}


void main() {
    // New Position
    vec3 pos = normalize((
        gl_TessCoord.x * tc_Vertex[0] +
        gl_TessCoord.y * tc_Vertex[1] +
        gl_TessCoord.z * tc_Vertex[2]
    ).xyz)/2.;


    // Normal
    vec3 s0 = tc_Size0[0];
    vec3 norm = normalize(pos);
    vec3 lpos = norm/2. * s0;  // Local Position
    vec3 wpos = lpos + tc_Pos[0] + vec3((u_Time/1000.),0.,0.).xyz;       // World Position

    // AZtroid position
    vec3 bump = AZtroidPos(lpos, wpos, norm, s0);

    
    // Get Normal
    vec3 pts[NORM_SAMPLE];
    vec3 nml[NORM_SAMPLE];

    for (int i=0; i<NORM_SAMPLE; i++) {
        float a = i * 360./NORM_SAMPLE;  // Angle to rotate

        vec3 p0 = (abs(dot(norm, vec3(1.,0.,0.)))>0.5)? vec3(0.,1.,0.) : vec3(1.,0.,0.);        // Random Vector
        vec3 p1 = normalize(cross(norm, p0));                                           // Perpendicular to norm
        vec3 p2 = (rotationMatrix(norm, a*TO_RAD) * vec4(p1, 1.)).xyz;                  // Rotated around norm

        vec3 new_norm = normalize((p2 * NORM_RADIUS / s0) + norm);   // New point normal
        vec3 new_lpos = new_norm/2. * s0;                            // New local position
        vec3 new_wpos = new_lpos + (wpos-lpos);                      // New world position

        pts[i] = AZtroidPos(new_lpos, new_wpos, new_norm, s0);
    }

    vec3 WN = vec3(0.);
    for (int i=0; i<NORM_SAMPLE-1; i++) {
        int j=i+1%NORM_SAMPLE;
        vec3 A = pts[i] - bump;
        vec3 B = pts[j] - bump;
        WN += normalize(cross(A,B));
    }
    te_Norm = -normalize(WN);
    


    // Plug to next stage
    te_Vertex = vec4(bump, 1.);
    te_ModelViewMat = tc_ModelViewMat[0];
    te_NormalMat = tc_NormalMat[0];
    te_Size = tc_Size0[0]*tc_Size1[0];
    te_ID = tc_ID[0];
    te_Pos = tc_Pos[0];

    gl_Position = u_ProjectionMat * tc_ModelViewMat[0] * (vec4(te_Size,1.) * te_Vertex);
}