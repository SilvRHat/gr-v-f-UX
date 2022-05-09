// Multi_verse: RE(Rendering Engine)
// Gavin Zimmerman

// Mv:RE Objects Source
// Provides instances for rendering Multi_verse scenes
#include "objects.h"



// STATIC DECLARATIONS
static void _OriginDefault(OriginInstance* o);
static int _OriginAddPart(OriginInstance* o, PartInstance* p);
static int _OriginRemovePart(OriginInstance* o, PartInstance* p);
static int _OriginAddLight(OriginInstance* o, LightInstance* l);
static int _OriginRemoveLight(OriginInstance* o, LightInstance* l);
static int _OriginAddOrigin(OriginInstance* o, OriginInstance* l);
static int _OriginRemoveOrigin(OriginInstance* o, OriginInstance* l);

static int _VerseAddJump(VerseInstance* v, JumpInstance* j);
static int _VerseRemoveJump(VerseInstance* v, JumpInstance* j);

static void _PrintShaderLog(GLuint shader, char* filename);
static void _PrintProgramLog(GLuint prog);



// SOURCE
// ////////////////////////////////////////////

// buildVAOVBO
    // @brief Helper function for setting up a VAO/VBO pair (unique per vertice set)
    // @param vao_ptr Address of VAO variable to set
    // @param vbo_ptr Address of VBO variable to set
    // @param vertex_data Array of geometery data
    // @param n of geometery data array
void buildVAOVBO(GLuint* vao_ptr, GLuint *vbo_ptr, float vertex_data[], int n) {
    // Create Buffer
    glGenVertexArrays(1, vao_ptr);
    glBindVertexArray(*vao_ptr);

    glGenBuffers(1, vbo_ptr);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo_ptr);
    glBufferData(GL_ARRAY_BUFFER, n, vertex_data, GL_STATIC_DRAW);


    // Per-vertex Attributes
    // Vertex
    glVertexAttribPointer(ATTRIB_POS_VERTEX, 4, GL_FLOAT, GL_FALSE, VERT_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) 0);
    glEnableVertexAttribArray(ATTRIB_POS_VERTEX);
    // Normal
    glVertexAttribPointer(ATTRIB_POS_NORMAL, 3, GL_FLOAT, GL_FALSE, VERT_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) (4*sizeof(float)));
    glEnableVertexAttribArray(ATTRIB_POS_NORMAL);
    // UV Map
    glVertexAttribPointer(ATTRIB_POS_UV_MAP, 2, GL_FLOAT, GL_FALSE, VERT_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) (7*sizeof(float)));
    glEnableVertexAttribArray(ATTRIB_POS_UV_MAP);

    
    // Per-instance Attributes
    glBindBuffer(GL_ARRAY_BUFFER, getInstancedAttribVBO());
    // ModelViewMatrix
    for (int i=0; i<4; i++) {
        glEnableVertexAttribArray(ATTRIB_POS_MODELVIEW+i);
        glVertexAttribPointer(ATTRIB_POS_MODELVIEW+i, 4, GL_FLOAT, GL_FALSE, INST_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) 0 + (i*sizeof(vec4)));
        glVertexAttribDivisor(ATTRIB_POS_MODELVIEW+i, 1);
    }
    // Normal Matrix
    for (int i=0; i<3; i++) {
        glVertexAttribPointer(ATTRIB_POS_NORMALMAT+i, 3, GL_FLOAT, GL_FALSE, INST_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) 0 + ((16 + (i*3))*sizeof(float)));
        glEnableVertexAttribArray(ATTRIB_POS_NORMALMAT+i);
        glVertexAttribDivisor(ATTRIB_POS_NORMALMAT+i, 1);
    }
    // Color
    glVertexAttribPointer(ATTRIB_POS_COLOR, 4, GL_FLOAT, GL_FALSE, INST_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) 0 + (25*sizeof(float)));
    glVertexAttribDivisor(ATTRIB_POS_COLOR, 1);
    glEnableVertexAttribArray(ATTRIB_POS_COLOR);
    // Size
    glVertexAttribPointer(ATTRIB_POS_SIZE, 3, GL_FLOAT, GL_FALSE, INST_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) 0 + (29*sizeof(float)));
    glVertexAttribDivisor(ATTRIB_POS_SIZE, 1);
    glEnableVertexAttribArray(ATTRIB_POS_SIZE);
    // Node Attrib ins
    for (int i=0; i<4; i++) {
        glEnableVertexAttribArray(ATTRIB_POS_NODE+i);
        glVertexAttribPointer(ATTRIB_POS_NODE+i, 4, GL_FLOAT, GL_FALSE, INST_ATTRIB_ROW*sizeof(GL_FLOAT), (void*) 0 + ((32 + (i*4))*sizeof(float)));
        glVertexAttribDivisor(ATTRIB_POS_NODE+i, 1);
    }
    
    // Release VAO / VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint getUniqueLightGroup() {
    static GLuint lg = GLOBAL_LIGHTGROUP;
    lg++;
    return lg;
}

GLuint getInstancedAttribVBO() {
    float attribbuff[INST_ATTRIB_ROW * INST_ATTRIB_COL] = {0};
    static GLuint vbo = 0;
    static int init = 0;
    
    if (!init) {
        init=1;
        // Generate Buffer
        glGenBuffers(1, &vbo);
        // Allocate space
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(attribbuff), attribbuff, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    return vbo;
}









// GAME INSTANCES
// ////////////////////////////////////////////

// PART INSTANCE
// Generic Constructor/Deconstructor

// NewPartInstance
    // @brief Creates a PartInstance object using malloc. 
    // All instance properties are set to their defaults.
    // @return Returns the created PartInstance
PartInstance* NewPartInstance() {
    PartInstance* p = (PartInstance*) malloc(sizeof(PartInstance));
    // Book Keeping
    p->Name="Part";
    p->ClassName="BasePart";
    p->Parent = NULL;
    p->CanRender = GL_TRUE;

    // Positional & Scaling
    vec3_set(p->Size, 1.f);
    vec3_zero(p->Pivot);
    mat4x4_identity(p->CFrame);

    // Shader stages (dubbed Nodes)
    for (int i=0; i<3; i++) {
        p->VertNodes[i] = 0;
        p->FragNodes[i] = 0;
        for (int j=0; j<4; j++) vec4_dup(p->NodeAttrs[i][j], UNIT4_W);
    }
    p->VertNodes[ENUM_NODE0] = simpleVertNode() ;
    p->FragNodes[ENUM_NODE0] = simpleFragNode();
    PartSetNodeAttribByName(p, ENUM_NODE0, "n_EmissionColor", (vec4){0,0,0,1});
    PartSetNodeAttribByName(p, ENUM_NODE0, "n_SpecularColor", (vec4){.6,0.6,0.6,1});
    PartSetNodeAttribByName(p, ENUM_NODE0, "n_Shiny", (vec4){2, 0,0,0});
    PartSetNodeAttribByName(p, ENUM_NODE0, "n_Emission", (vec4){0, 0,0,0});
    
    // Textures
    vec4_set(p->Color, 1.f);
    p->Texture = 0;
    for (int i=0; i<MAX_LIGHTGROUPS; i++) p->LightGroups[i] = 0;
    PartAddLightGroup(p, GLOBAL_LIGHTGROUP);

    // Vertex Data
    p->Vao = 0;
    p->Vbo = 0;
    p->Vertices  = 0;
    p->DrawMode = GL_LINE_STRIP;
    p->VboReused = GL_TRUE;

    // Internal
    p->_Malloced = GL_TRUE;
    return p;
}

// DeletePartInstanceBuffers
    // @brief Deletes the VAO/VBO assosiated with a part instance if it exists.
    // @return Returns NULL
void* DeletePartInstanceBuffers(PartInstance* p) {
    if (glIsBuffer(p->Vbo))
        glDeleteBuffers(1, &p->Vbo);
    if (glIsVertexArray(p->Vao))
        glDeleteVertexArrays(1, &p->Vao);
    p->Vbo = 0;
    p->Vao = 0;
    p->VboReused = GL_TRUE;

    return NULL;
}

// DestroyPartInstance
    // @brief Will free Part Instance from memory. This should only be called once per part instance.
    // @return Returns NULL
void* DestroyPartInstance(PartInstance* p) {
    // Disconnect references
    PartSetParent(p, NULL);
    if (p->_Malloced) {
        p->_Malloced = GL_FALSE;
        free(p);
    }
    return NULL;
}


// Primitive Object constructors
// point
    // @brief Creates a point
    // @param pt The location of the point
    // @return Just a point :)
PartInstance* point(vec4 pt) {
    static GLuint vao, vbo;
    static int init = 0;

    if (!init) {
        init = 1;
        float vert_data[] = {0,0,0,1,  0,0,0,  0,0};
        buildVAOVBO(&vao, &vbo, vert_data, sizeof(VERT_ATTRIB_ROW));
    }

    PartInstance* p = NewPartInstance();
    p->ClassName = "PointPart";
    PartSetPosition(p, pt);
    p->Vao = vao;
    p->Vbo = vbo;
    p->Vertices = 1;
    p->DrawMode = GL_POINTS;
    p->VboReused = GL_TRUE;
    
    return p;
}

// primitive
    // @brief Vaguely construct any object from the data and primitive passed in.
    // ALWAYS creates a new VAO/VBO pair.
    // @param n Number of points in line
    // @pts Array of vec4 3d positions
    // @return Primitive Instance
PartInstance* primitive(int n, vec4 pts[], vec3 norms[], vec2 uvs[], GLenum DrawType) {
    // Build vertex data
    GLuint vao, vbo;
    float verts[VERT_ATTRIB_ROW * n];
    {
        for (int i=0; i<n; i++) {
            int off = i * VERT_ATTRIB_ROW;
            vec4_dup((float*) (verts+off), pts[i]);     // Position
            vec3_dup((float*) (verts+off+4), norms[i]); // Normal
            vec2_dup((float*) (verts+off+7), uvs[i]);   // UV Map
        }
    }
    buildVAOVBO(&vao, &vbo, verts, sizeof(verts));
    
    PartInstance* p = NewPartInstance();
    p->ClassName = "PrimitivePart";
    p->Vao = vao;
    p->Vbo = vbo;
    p->Vertices = n;
    p->DrawMode = DrawType;
    p->VboReused = GL_FALSE;

    return p;
}

// canvas
    // @brief Contructs a part instance for rendering a plane/ canvas. 
    // Named canvas for its intended-usage as displaying images
    // @param size x/z sizing of canvas
    // @return Canvas Instance
PartInstance* canvas(float size) {
    static GLuint vao = 0, vbo = 0;
    static int init = 0;

    if (!init) {
        init=1;
        float s=.5;
        float verts[] = {
             s, s, 0, 1,   0, 0, 1,   1, 1,
             s,-s, 0, 1,   0, 0, 1,   0, 1,
            -s, s, 0, 1,   0, 0, 1,   1, 0,
            -s,-s, 0, 1,   0, 0, 1,   0, 0
        };
        buildVAOVBO(&vao, &vbo, verts, sizeof(verts));
    }
    
    PartInstance* p = NewPartInstance();
    p->ClassName = "CanvasPart";
    PartSetSize(p, (vec3){size, size, 0});
    p->Vao = vao;
    p->Vbo = vbo;
    p->Vertices = 4;
    p->DrawMode = GL_TRIANGLE_STRIP;
    p->VboReused = GL_TRUE;

    return p;
}

// Circle
    // @brief Contructs a part instance for rendering a circle
    // @param sides Number of sides to render on circle; inputs are standardized to reduce unique vaos/vbos
    // @param radius Radius of circle
    // @return Part Instance for drawing circle
PartInstance* circle(int sides, float radius) {
    static GLuint vao[6] = {0}, vbo[6] = {0};
    GLuint cvao = 0, cvbo = 0, *evao, *evbo;
    if (sides<=2) return NULL;
    
    // Set effective vao/ vbo int address to use
    int mem;
    switch(sides) {
        case XLO_DETAIL:    mem= 0; break;  // Standard Params
        case LO_DETAIL:     mem= 1; break;
        case MED_DETAIL:    mem= 2; break;
        case HI_DETAIL:     mem= 3; break;
        case XHI_DETAIL:    mem= 4; break;
        case XXHI_DETAIL:   mem= 5; break;
        default:            mem=-1; break;  // Custom Params
    }
    if (mem==-1) {
        evao = &cvao; evbo = &cvbo;
    }
    else {
        evao = &vao[mem]; evbo = &vbo[mem];
    }
    
    // If not initialized or custom params, build vertice data from new
    if ((*evao==0) || (*evbo==0)) {
        float s = .5;
        float verts[VERT_ATTRIB_ROW * (sides+2)];
        {
            // Triangle Fan
            int off=0;
            vec4_dup((float*) (verts+0), UNIT4_W);          // Position
            vec3_dup((float*) (verts+4), UNIT3_Z);          // Normal
            vec2_dup((float*) (verts+7), (vec2){.5, .5});   // UV Map     

            for (int i=1; i<sides+2; i++) {
                double a = (float)i * 360.f / (float)sides; // Angle
                vec4 pos = {s*sinf(a*TO_RAD), s*cosf(a*TO_RAD), 0.f, 1.f};
                
                off = i * VERT_ATTRIB_ROW;
                vec4_dup((float*) (verts+off+0), pos);          // Position
                vec3_dup((float*) (verts+off+4), UNIT3_Z);          // Normal
                vec2_dup((float*) (verts+off+7), (vec2){.5+pos[0], .5+pos[1]});   // UV Map 
            }
        }
        buildVAOVBO(evao, evbo, verts, sizeof(verts));
    }

    // Create Instance
    PartInstance* p = NewPartInstance();
    p->ClassName = "CirclePart";
    PartSetSize(p, (vec3){radius*2, radius*2, 0});
    p->Vao = *evao;
    p->Vbo = *evbo;
    p->Vertices = sides+2;
    p->DrawMode = GL_TRIANGLE_FAN;
    p->VboReused = (mem==-1)? GL_FALSE : GL_TRUE;
    return p;
}

// Cube
    // @brief Contructs a part instance for rendering a cube
    // @param size x/y/z sizing of cube
    // @return Part Instance for drawing cube
PartInstance* cube(float size) {
    static unsigned int vao = 0, vbo = 0;
    static int init = 0;

    // Generate vertices
    if (!init) {
        init = 1;
        // Generate cube
        float verts[VERT_ATTRIB_ROW * 36];
        {
            float s = .5;
            mat4x4 front_verts[4];
            mat4x4_from_pos(front_verts[0], (vec3){ s, s, s});
            mat4x4_from_pos(front_verts[1], (vec3){ s,-s, s});
            mat4x4_from_pos(front_verts[2], (vec3){-s, s, s});
            mat4x4_from_pos(front_verts[3], (vec3){-s,-s, s}); 
            vec2 UVe[4] = {{0,0},{0,1},{1,0},{1,1}};
            vec2 UVo[4] = {{1,1},{1,0},{0,1},{0,0}};

            // Rotate around sides
            for (int f=0; f<6; f++) {
                mat4x4 rot, tri;
                mat4x4_identity(rot);
                if (f<4) // Front, Back, Left, Right Rotations
                    mat4x4_rotate_X(rot, rot, 90*f*TO_RAD);
                else // Top & Bottom
                    mat4x4_rotate_Y(rot, rot, 90*TO_RAD * (f==4?1:-1));

                int vidx[6] = {0, 1, 2, 2, 1, 3};

                for (int v=0; v<6; v++) {
                    int pt = vidx[v];
                    int off = ((f*6)+v) * VERT_ATTRIB_ROW;

                    mat4x4_mul(tri, rot, front_verts[pt]); // Rotate front face

                    vec4_dup(verts+off, tri[3]);  // Set Position
                    vec3_dup(verts+off+4, tri[2]);  // Set Normal
                    vec2_dup(verts+off+7, (f%2==0)? UVe[pt]:UVo[pt]); // Set UV Map
                }
            }
        }
        buildVAOVBO(&vao, &vbo, verts, sizeof(verts));
    }

    // Create Instance
    PartInstance* p = NewPartInstance();
    p->ClassName = "CubePart";
    PartSetSize(p, (vec3){size,size,size});
    p->Vao = vao;
    p->Vbo = vbo;
    p->Vertices = 36;
    p->DrawMode = GL_TRIANGLES;
    p->VboReused = GL_TRUE;
    return p;
}

// SphereVertFromAngles
    // @brief Sets a vec4 to a position on a sphere of radius 1 given 2 angles
    // @param th Angle 1 (Horizontal XZ)
    // @param ph Angle 2 (Vertical Y)
static void SphereVertFromAngles(vec4 v, double th, double ph) {
    v[0] = cos(ph * TO_RAD) * sin(th * TO_RAD);
    v[1] = sin(ph * TO_RAD);
    v[2] = cos(ph * TO_RAD) * cos(th * TO_RAD);
    v[3] = 1.f;
}
// uvSphere
    // @brief Contructs a part instance for rendering a sphere built with quads (UV)
    // @param segments The amount of detail around the sphere (XZ). These inputs are standardized to reduce unqiue vbos/vaos
    // @param rings The amount of detail along the sphere (Y). These inputs are standardized to reduce unique vaos/vbos
    // @return Part Instance for drawing sphere. If not given enough segment/ring input, NULL is returned
    // since the requested shape is not qualified for drawing a sphere geometrically in 3D space.
PartInstance* uvSphere(int segments, int rings, float radius) {
    static GLuint vao[6][6] = {{0,0,0,0,0,0}}, vbo[6][6] = {{0,0,0,0,0,0}};
    GLuint cvao = 0, cvbo = 0, *evao, *evbo;
    if ((segments<=2) || (rings<=1)) return NULL;
    
    // Set effective vao/ vbo integer to use
    int mem[2];
    for (int i=0; i<2; i++) {
        switch((i==0)?segments:rings) {
            case XLO_DETAIL:    mem[i]= 0; break;  // Standard Params
            case LO_DETAIL:     mem[i]= 1; break;
            case MED_DETAIL:    mem[i]= 2; break;
            case HI_DETAIL:     mem[i]= 3; break;
            case XHI_DETAIL:    mem[i]= 4; break;
            case XXHI_DETAIL:   mem[i]= 5; break;
            default:            mem[i]=-1; break;  // Custom Params
        }
    }   
    if ((mem[0]==-1) || (mem[1]==-1)) {
        evao=&cvao; evbo=&cvbo;
    }
    else {
        evao=&(vao[mem[0]][mem[1]]); evbo=&(vbo[mem[0]][mem[1]]);
    }


    if ((*evao==0) || (*evbo==0)) {
        float verts[VERT_ATTRIB_ROW * (rings+1) * (segments+1) * 2];
        {
            double ph_inc = 180.0/(rings+1), th_inc = 360.0/segments;
            for (int r=0; r<rings+1; r++) {
                for (int s=0; s<segments+1; s++) {
                    double ph = fmod(ph_inc*r, 180.0) - 90.0;
                    double th = fmod(th_inc*s, 360.0);
                    int off = (r*(segments+1) + s) * 2 * VERT_ATTRIB_ROW;

                    // Get Vert Positions
                    vec4 p1, p2;
                    SphereVertFromAngles(p1, th, ph); vec3_scale(p1, p1, .5);
                    SphereVertFromAngles(p2, th, ph+ph_inc); vec3_scale(p2, p2, .5);
                    
                    // Set Positions
                    vec4_dup(verts+off, p1);    // Vert1 Position
                    vec3_dup(verts+off+4, p1);  // Vert1 Normal
                    vec4_dup(verts+off + VERT_ATTRIB_ROW, p2);  // Vert2 Position
                    vec3_dup(verts+off+4+VERT_ATTRIB_ROW, p2);  // Vert2 Normal
                    

                    float a = (th_inc*s)/90.f;
                    float b = (ph+90.)/90.f;
                    float c = (ph+ph_inc+90.)/90.f;
                    //b = (b<1)? b: 2-b;
                    //c = (c<1)? c: 2-c;

                    //float u = sin((th_inc*s) * TO_RAD);
                    //float v = cos((th_inc*s) * TO_RAD);
                    vec2 uv1={a, b}, uv2 = {a, c};
                    
                    vec2_dup(verts+off+7, uv1);
                    vec2_dup(verts+off+7+VERT_ATTRIB_ROW, uv2);
                }
            }
        }
        buildVAOVBO(evao, evbo, verts, sizeof(verts));
    }

    // Create Instance
    PartInstance* p = NewPartInstance();
    p->Name = "NewUVSphere";
    p->ClassName = "UVSpherePart";
    PartSetSize(p, (vec3){radius*2, radius*2, radius*2});
    p->Vao = *evao;
    p->Vbo = *evbo;
    p->Vertices = (rings+1) * (segments+1) * 2;
    p->DrawMode = GL_TRIANGLE_STRIP;
    p->VboReused = ((mem[0]==-1) || (mem[1]==-1))? GL_FALSE : GL_TRUE;
    return p;
}

// icoSphere
// Credit: William A. Vlakkies CSCIx239 library
PartInstance* icoSphere(float radius) {
    static int init = 0;
    static GLuint vao, vbo;

    if (!init ){
        init=1;

        float verts[VERT_ATTRIB_ROW*60] =
        {
            // X      Y      Z     W      Nx     Ny     Nz    S   T
             0.276, 0.851, 0.447, 1.0,  0.471, 0.342, 0.761, 0.0,0.0,
             0.894, 0.000, 0.447, 1.0,  0.471, 0.342, 0.761, 1.0,0.0,
             0.000, 0.000, 1.000, 1.0,  0.471, 0.342, 0.761, 0.5,1.0,
            -0.724, 0.526, 0.447, 1.0, -0.180, 0.553, 0.761, 0.0,0.0,
             0.276, 0.851, 0.447, 1.0, -0.180, 0.553, 0.761, 1.0,0.0,
             0.000, 0.000, 1.000, 1.0, -0.180, 0.553, 0.761, 0.5,1.0,
            -0.724,-0.526, 0.447, 1.0, -0.582, 0.000, 0.762, 0.0,0.0,
            -0.724, 0.526, 0.447, 1.0, -0.582, 0.000, 0.762, 1.0,0.0,
             0.000, 0.000, 1.000, 1.0, -0.582, 0.000, 0.762, 0.5,1.0,
             0.276,-0.851, 0.447, 1.0, -0.180,-0.553, 0.761, 0.0,0.0,
            -0.724,-0.526, 0.447, 1.0, -0.180,-0.553, 0.761, 1.0,0.0,
             0.000, 0.000, 1.000, 1.0, -0.180,-0.553, 0.761, 0.5,1.0,
             0.894, 0.000, 0.447, 1.0,  0.471,-0.342, 0.761, 0.0,0.0,
             0.276,-0.851, 0.447, 1.0,  0.471,-0.342, 0.761, 1.0,0.0,
             0.000, 0.000, 1.000, 1.0,  0.471,-0.342, 0.761, 0.5,1.0,
             0.000, 0.000,-1.000, 1.0,  0.180, 0.553,-0.761, 0.0,0.0,
             0.724, 0.526,-0.447, 1.0,  0.180, 0.553,-0.761, 1.0,0.0,
            -0.276, 0.851,-0.447, 1.0,  0.180, 0.553,-0.761, 0.5,1.0,
             0.000, 0.000,-1.000, 1.0, -0.471, 0.342,-0.761, 0.0,0.0,
            -0.276, 0.851,-0.447, 1.0, -0.471, 0.342,-0.761, 1.0,0.0,
            -0.894, 0.000,-0.447, 1.0, -0.471, 0.342,-0.761, 0.5,1.0,
             0.000, 0.000,-1.000, 1.0, -0.471,-0.342,-0.761, 0.0,0.0,
            -0.894, 0.000,-0.447, 1.0, -0.471,-0.342,-0.761, 1.0,0.0,
            -0.276,-0.851,-0.447, 1.0, -0.471,-0.342,-0.761, 0.5,1.0,
             0.000, 0.000,-1.000, 1.0,  0.180,-0.553,-0.761, 0.0,0.0,
            -0.276,-0.851,-0.447, 1.0,  0.180,-0.553,-0.761, 1.0,0.0,
             0.724,-0.526,-0.447, 1.0,  0.180,-0.553,-0.761, 0.5,1.0,
             0.000, 0.000,-1.000, 1.0,  0.582, 0.000,-0.762, 0.0,0.0,
             0.724,-0.526,-0.447, 1.0,  0.582, 0.000,-0.762, 1.0,0.0,
             0.724, 0.526,-0.447, 1.0,  0.582, 0.000,-0.762, 0.5,1.0,
             0.894, 0.000, 0.447, 1.0,  0.761, 0.552, 0.180, 0.0,0.0,
             0.276, 0.851, 0.447, 1.0,  0.761, 0.552, 0.180, 1.0,0.0,
             0.724, 0.526,-0.447, 1.0,  0.761, 0.552, 0.180, 0.5,1.0,
             0.276, 0.851, 0.447, 1.0, -0.291, 0.894, 0.179, 0.0,0.0,
            -0.724, 0.526, 0.447, 1.0, -0.291, 0.894, 0.179, 1.0,0.0,
            -0.276, 0.851,-0.447, 1.0, -0.291, 0.894, 0.179, 0.5,1.0,
            -0.724, 0.526, 0.447, 1.0, -0.940, 0.000, 0.179, 0.0,0.0,
            -0.724,-0.526, 0.447, 1.0, -0.940, 0.000, 0.179, 1.0,0.0,
            -0.894, 0.000,-0.447, 1.0, -0.940, 0.000, 0.179, 0.5,1.0,
            -0.724,-0.526, 0.447, 1.0, -0.291,-0.894, 0.179, 0.0,0.0,
             0.276,-0.851, 0.447, 1.0, -0.291,-0.894, 0.179, 1.0,0.0,
            -0.276,-0.851,-0.447, 1.0, -0.291,-0.894, 0.179, 0.5,1.0,
             0.276,-0.851, 0.447, 1.0,  0.761,-0.552, 0.180, 0.0,0.0,
             0.894, 0.000, 0.447, 1.0,  0.761,-0.552, 0.180, 1.0,0.0,
             0.724,-0.526,-0.447, 1.0,  0.761,-0.552, 0.180, 0.5,1.0,
             0.276, 0.851, 0.447, 1.0,  0.291, 0.894,-0.179, 0.0,0.0,
            -0.276, 0.851,-0.447, 1.0,  0.291, 0.894,-0.179, 1.0,0.0,
             0.724, 0.526,-0.447, 1.0,  0.291, 0.894,-0.179, 0.5,1.0,
            -0.724, 0.526, 0.447, 1.0, -0.761, 0.552,-0.180, 0.0,0.0,
            -0.894, 0.000,-0.447, 1.0, -0.761, 0.552,-0.180, 1.0,0.0,
            -0.276, 0.851,-0.447, 1.0, -0.761, 0.552,-0.180, 0.5,1.0,
            -0.724,-0.526, 0.447, 1.0, -0.761,-0.552,-0.180, 0.0,0.0,
            -0.276,-0.851,-0.447, 1.0, -0.761,-0.552,-0.180, 1.0,0.0,
            -0.894, 0.000,-0.447, 1.0, -0.761,-0.552,-0.180, 0.5,1.0,
             0.276,-0.851, 0.447, 1.0,  0.291,-0.894,-0.179, 0.0,0.0,
             0.724,-0.526,-0.447, 1.0,  0.291,-0.894,-0.179, 1.0,0.0,
            -0.276,-0.851,-0.447, 1.0,  0.291,-0.894,-0.179, 0.5,1.0,
             0.894, 0.000, 0.447, 1.0,  0.940, 0.000,-0.179, 0.0,0.0,
             0.724, 0.526,-0.447, 1.0,  0.940, 0.000,-0.179, 1.0,0.0,
             0.724,-0.526,-0.447, 1.0,  0.940, 0.000,-0.179, 0.5,1.0,
        };
        // Divide vertex pos by 2
        for (int i=0; i<60; i++) for (int j=0; j<3; j++) {
            verts[VERT_ATTRIB_ROW*i +j]=verts[VERT_ATTRIB_ROW*i +j]/2.;
        }
        buildVAOVBO(&vao, &vbo, verts, sizeof(verts));
    }

    // Create Instance
    PartInstance* p = NewPartInstance();
    p->Name = "NewIcoSphere";
    p->ClassName = "IcoSpherePart";
    PartSetSize(p, (vec3){radius*2, radius*2, radius*2});
    p->Vao = vao;
    p->Vbo = vbo;
    p->Vertices = 60;
    p->DrawMode = GL_TRIANGLES;
    p->VboReused = GL_TRUE;
    return p;
}


// Cylinder
    // @brief Contructs a part instance for rendering a cylinder
    // @param sides Number of sides to render on cylinder; inputs are standardized to reduce unique vaos/vbos
    // @return Part Instance for drawing cylinder
PartInstance* cylinder(int sides, float radius, float depth) {
    static unsigned int vao[6] = {0}, vbo[6] = {0};
    unsigned int cvao = 0, cvbo = 0, *evao, *evbo;
    if (sides<=2)
        return NULL;
    
    // Set effective vao/ vbo integer to use
    int mem = -1;
    switch(sides) {
        case XLO_DETAIL:    mem= 0; break;  // Standard Params
        case LO_DETAIL:     mem= 1; break;
        case MED_DETAIL:    mem= 2; break;
        case HI_DETAIL:     mem= 3; break;
        case XHI_DETAIL:    mem= 4; break;
        case XXHI_DETAIL:   mem= 5; break;
        default:            mem=-1; break;  // Custom Params
    }
    if (mem==-1) {
        evao = &cvao; evbo = &cvbo;
    }
    else {
        evao = &vao[mem]; evbo = &vbo[mem];
    }
    

    if ((*evao==0) || (*evbo==0)) {
        float verts[(6*sides+4) * VERT_ATTRIB_ROW];
        {
            // Top [sides*2 +1]
            float s=.5;
            {
                vec3 normal = {0,s,0};
                for (int i=0; i<sides; i++) {
                    int off0 = (i*2)*VERT_ATTRIB_ROW;
                    int off1 = (i*2+1)*VERT_ATTRIB_ROW;
                    float a = (float)i * (360.0/(float)sides) *TO_RAD;
                    vec4 pos = {s*sinf(a), s, s*cosf(a), 1};
                    vec2 uv = {pos[0]+s, pos[2]+s};

                    // Outer
                    vec4_dup((float*) verts+off0+0, pos);
                    vec3_dup((float*) verts+off0+4, normal);
                    vec2_dup((float*) verts+off0+7, uv);
                    // Inner
                    vec4_dup((float*) verts+off1+0, (vec4){0,s,0,1});
                    vec3_dup((float*) verts+off1+4, normal);
                    vec2_dup((float*) verts+off1+7, (vec2){.5,.5});
                } {
                    int off0 = (sides*2)*VERT_ATTRIB_ROW;
                    vec4 pos = {s*sinf(0), s, s*cosf(0), 1};
                    vec2 uv = {pos[0]+s, pos[2]+s};

                    vec4_dup((float*) verts+off0+0, pos);
                    vec3_dup((float*) verts+off0+4, normal);
                    vec2_dup((float*) verts+off0+7, uv);
                }
            }
            // Side [sides*2 + 2]
            {
                int topoff = (sides*2+1) * VERT_ATTRIB_ROW;
                for (int i=0; i<sides+1; i++) {
                    int off0 = (i*2)*VERT_ATTRIB_ROW + topoff;
                    int off1 = (i*2+1)*VERT_ATTRIB_ROW + topoff;
                    float a = (float)i * (360.0/(float)sides) *TO_RAD;
                    vec4 pos0 = {s*sinf(a), s, s*cosf(a), 1};
                    vec4 pos1 = {s*sinf(a), -s, s*cosf(a), 1};
                    vec3 normal = {sinf(a), 0, cosf(a)};

                    float u = (a*TO_DEG)/180.f;
                    //u = u<1? u: 2-u;
                    

                    // Outer
                    vec4_dup((float*) verts+off0+0, pos0);
                    vec3_dup((float*) verts+off0+4, normal);
                    vec2_dup((float*) verts+off0+7, (vec2){u, 1});
                    // Inner
                    vec4_dup((float*) verts+off1+0, pos1);
                    vec3_dup((float*) verts+off1+4, normal);
                    vec2_dup((float*) verts+off1+7, (vec2){u, 0});
                }
            }
            // Bottom [sides*2 + 1]
            {
                int sideoff = (sides*4+3) * VERT_ATTRIB_ROW;
                vec3 normal = {0,-s,0};
                for (int i=0; i<sides; i++) {
                    int off0 = (i*2)*VERT_ATTRIB_ROW + sideoff;
                    int off1 = (i*2+1)*VERT_ATTRIB_ROW + sideoff;
                    float a = (float)i * (360.0/(float)sides) *TO_RAD;
                    vec4 pos = {s*sinf(a), -s, s*cosf(a), 1};
                    vec2 uv = {pos[0]+s, pos[2]+s};

                    // Outer
                    vec4_dup((float*) verts+off0+0, pos);
                    vec3_dup((float*) verts+off0+4, normal);
                    vec2_dup((float*) verts+off0+7, uv);
                    // Inner
                    vec4_dup((float*) verts+off1+0, (vec4){0,-s,0,1});
                    vec3_dup((float*) verts+off1+4, normal);
                    vec2_dup((float*) verts+off1+7, (vec2){.5,.5});
                } {
                    int off0 = (sides*2)*VERT_ATTRIB_ROW  + sideoff;
                    vec4 pos = {s*sinf(0), -s, s*cosf(0), 1};
                    vec2 uv = {pos[0]+s, pos[2]+s};

                    vec4_dup((float*) verts+off0+0, pos);
                    vec3_dup((float*) verts+off0+4, normal);
                    vec2_dup((float*) verts+off0+7, uv);
                }
            }
        }
        buildVAOVBO(evao, evbo, verts, sizeof(verts));
    }

    // Create Instance
    PartInstance* p = NewPartInstance();
    p->Name = "NewCylinder";
    p->ClassName = "CylinderPart";
    PartSetSize(p, (vec3){radius*2, depth, radius*2});
    p->Vao = *evao;
    p->Vbo = *evbo;
    p->Vertices = (6*sides)+4;
    p->DrawMode = GL_TRIANGLE_STRIP;
    p->VboReused = (mem==-1)? GL_FALSE : GL_TRUE;
    return p;
}

// clonePart
    // @brief Duplicates a part instance resuing all set properties including vao/vbo properties
    // @param src Part Instance to clone
    // @return Cloned Part Instance
PartInstance* clonePart(PartInstance* src) {
    PartInstance* p = NewPartInstance();
    memcpy(p, src, sizeof(PartInstance));
    p->Parent = NULL;
    return p;
}




// Methods
void PartSetParent(PartInstance* p, OriginInstance* parent) {
    if (p->Parent!=NULL)
        _OriginRemovePart(p->Parent, p);
    
    if (parent!=NULL) {
        int res = _OriginAddPart(parent, p);
        if (res<0)
            Error("Could not add new part (%s) to origin (%s)", p->Name, parent->Name);
    }
    p->Parent = parent;
}

void PartSetSize(PartInstance* p, vec3 size) {
    vec3_dup(p->Size, size);
}

void PartSetPivot(PartInstance* p, vec3 pivot) {
    vec3_dup(p->Pivot, pivot);
}

void PartSetPosition(PartInstance* p, vec3 pos) {
    vec3_dup(p->CFrame[3], pos);
}

void PartSetRotation(PartInstance* p, vec3 rot, int euler_order) {
    mat4x4 temp;
    mat4x4_from_euler(temp, rot, euler_order);
    for (int i=0; i<3; i++)
        vec4_dup(p->CFrame[i], temp[i]);
}

void PartSetCFrame(PartInstance* p, mat4x4 cf) {
    mat4x4_dup(p->CFrame, cf);
}

void PartSetNodeAttribByName(PartInstance* p, GLenum nodetype, char* name, vec4 val) {
    if (!(nodetype<3)) return;
    
    GLuint node = p->VertNodes[nodetype];
    int loc = glGetAttribLocation(node, name);
    if ((loc >= ATTRIB_POS_NODE) && (loc<ATTRIB_POS_NODE+4)) {
        int col = loc - ATTRIB_POS_NODE;
        vec4_dup(p->NodeAttrs[nodetype][col], val);
    }
}

void PartSetNodeColorByName(PartInstance* p, GLenum nodetype, char* name, color4 val) {
    if (!(nodetype<3)) return;
    
    GLuint node = p->VertNodes[nodetype];
    int loc = glGetAttribLocation(node, name);
    if ((loc >= ATTRIB_POS_NODE) && (loc<ATTRIB_POS_NODE+4)) {
        int col = loc - ATTRIB_POS_NODE;
        vec4_from_col(p->NodeAttrs[nodetype][col], val);
    }
}

void PartSetColor(PartInstance* p, color4 col) {
    vec4_from_col(p->Color, col);
}

void PartAddLightGroup(PartInstance* p, GLuint lg) {
    for (int i=0; i<MAX_LIGHTGROUPS; i++) {
        if (p->LightGroups[i] == 0) {
            p->LightGroups[i] = lg;
            break;
        }
    }
}

void PartRemoveLightGroup(PartInstance* p, GLuint lg) {
    for (int i=0; i<MAX_LIGHTGROUPS; i++) {
        if (p->LightGroups[i]==lg) {
            for (int j=i; j<MAX_LIGHTGROUPS-1; j++)
                p->LightGroups[j]=p->LightGroups[j+1];
            p->LightGroups[MAX_LIGHTGROUPS-1] = 0;
            break;
        }
    }
}








// LIGHT INSTANCE
void _PassHander(void* func, va_list args) {
    ((void(*)())(func))();
}
// Generic Constructor/Deconstructor
LightInstance* NewLightInstance() {
    LightInstance* l = malloc(sizeof(LightInstance));
    // Book Keeping
    l->Name = "NewLight";
    l->ClassName = "BaseLight";
    l->Parent = NULL;
    l->CanRender = GL_TRUE;

    // Default Light Uniforms
    vec4_set(l->Color, 1.f);
    mat4x4_identity(l->CFrame);
    l->LightEnum = ENUM_LIGHT_NONE;

    // Shader settings
    l->OutFBO = 0;
    l->OverShadeNode = 0;
    l->Noding= ENUM_NODE0;
    l->BlendSrc = GL_ONE; 
    l->BlendDst = GL_ZERO;

    l->ZIndex = 0;
    l->LightGroup = GLOBAL_LIGHTGROUP;
    l->UseTexture = 0;

    // Custom Uniforms
    for (int i=0; i<MAX_TEX_UNIFORMS; i++) l->TexUniforms[i].Name = NULL;
    for (int i=0; i<MAX_VEC_UNIFORMS; i++) l->VecUniforms[i].Name = NULL;
    for (int i=0; i<MAX_MAT_UNIFORMS; i++) l->MatUniforms[i].Name = NULL;
    
    // Internal
    l->_Malloced = GL_TRUE;

    // Events
    SignalInit(&l->PrePass, _PassHander);
    SignalInit(&l->PostPass, _PassHander);
    return l;
}

void* DestroyLightInstance(LightInstance* l) {
    // Remove References
    LightSetParent(l, NULL);
    if (l->_Malloced) {
        l->_Malloced = GL_FALSE;
        free(l);
    }
    return NULL;
}

// Primitive Light Constructors
LightInstance* globalOcclusion() {
    LightInstance* l = NewLightInstance();

    l->LightEnum = ENUM_LIGHT_OCCLUSION;
    l->OutFBO = 0;
    l->Noding = ENUM_NODE0;
    l->BlendSrc = GL_ONE;
    l->BlendDst = GL_ONE_MINUS_SRC_ALPHA;
    l->LightGroup = GLOBAL_LIGHTGROUP;
    l->UseTexture = GL_FALSE;

    return l;
}

LightInstance* ambientLight() {
    LightInstance* l = NewLightInstance();

    l->ClassName = "AmbientLight";
    l->LightEnum = ENUM_LIGHT_AMBIENT;

    return l;
}

LightInstance* pointLight(float radius) {
    LightInstance* l = NewLightInstance();

    l->ClassName = "PointLight";
    l->LightEnum = ENUM_LIGHT_POINT;
    LightSetVecUniform(l, UNI_NAME_LIGHT_RADIUS, (float[]){radius}, 1);

    return l;
}

LightInstance* spotLight(float radius, float inner_angle, float outer_angle) {
    LightInstance* l = NewLightInstance();

    l->ClassName = "SpotLight";
    l->LightEnum = ENUM_LIGHT_SPOT;
    LightSetVecUniform(l, UNI_NAME_LIGHT_RADIUS, (float[]){radius}, 1);
    LightSetVecUniform(l, UNI_NAME_LIGHT_THETA0, (float[]){inner_angle}, 1);
    LightSetVecUniform(l, UNI_NAME_LIGHT_THETA1, (float[]){outer_angle}, 1);

    return l;
}

LightInstance* cloneLight(LightInstance* l) {
    LightInstance* new = NewLightInstance();
    memcpy(new, l, sizeof(LightInstance));
    new->Parent = NULL;
    return new;
}




// Methods
void LightSetParent(LightInstance* l, OriginInstance* parent) {
    if (l->Parent!=NULL)
        _OriginRemoveLight(l->Parent, l);
    
    if (parent!=NULL) {
        int res = _OriginAddLight(parent, l);
        if (res<0)
            Error("Could not add new light (%s) to origin (%s)", l->Name, parent->Name);
    }
    l->Parent = parent;
}

void LightSetColor(LightInstance* l, color4 col){
    vec4_from_col(l->Color, col);
}

void LightSetPosition(LightInstance* l, vec3 pos) {
    vec3_dup(l->CFrame[3], pos);
}

void LightSetRotation(LightInstance* l, vec3 rot, int euler_order) {
    mat4x4 temp;
    mat4x4_from_euler(temp, rot, euler_order);
    for (int i=0; i<3; i++)
        vec4_dup(l->CFrame[i], temp[i]);
}

void LightSetCFrame(LightInstance* l, mat4x4 cf) {
    mat4x4_dup(l->CFrame, cf);
}

void LightSetTexUniform(LightInstance* l, char* name, GLuint val, GLenum target) {
    // Set Existing
    for (int i=0; i<MAX_TEX_UNIFORMS; i++) {
        if (l->TexUniforms[i].Name==NULL) continue;
        if (strcmp(l->TexUniforms[i].Name, name)==0) {
            l->TexUniforms[i].Value = val;
            l->TexUniforms[i].BindTarget = target;
            return;
        }
    }
    // New
    for (int i=0; i<MAX_TEX_UNIFORMS; i++) {
        if ((l->TexUniforms[i].Name==NULL) || (strcmp(l->TexUniforms[i].Name, "")==0)) {
            l->TexUniforms[i].Name = name;
            l->TexUniforms[i].Value = val;
            l->TexUniforms[i].BindTarget = target;
            return;
        }
    }
}

void LightSetVecUniform(LightInstance* l, char* name, float val[], int n) {
    // Set Existing
    for (int i=0; i<MAX_VEC_UNIFORMS; i++) {
        if (l->VecUniforms[i].Name==NULL) continue;
        if (strcmp(l->VecUniforms[i].Name, name)==0) {
            l->VecUniforms[i].N = n;
            for (int j=0; j<n; j++) l->VecUniforms[i].Value[j] = val[j];
            return;
        }
    }
    // New
    for (int i=0; i<MAX_VEC_UNIFORMS; i++) {
        if ((l->VecUniforms[i].Name==NULL) || (strcmp(l->VecUniforms[i].Name, "")==0)) {
            l->VecUniforms[i].Name = name;
            l->VecUniforms[i].N = n;
            for (int j=0; j<n; j++) l->VecUniforms[i].Value[j] = val[j];
            return;
        }
    }
}

void LightSetMatUniform(LightInstance* l, char* name, mat4x4 val, int n, int m) {
    // Set Existing
    for (int i=0; i<MAX_MAT_UNIFORMS; i++) {
        if (l->MatUniforms[i].Name==NULL) continue;
        if (strcmp(l->MatUniforms[i].Name, name)==0) {
            l->MatUniforms[i].N = n;
            l->MatUniforms[i].M = m;
            mat4x4_dup(l->MatUniforms->Value, val);
            return;
        }
    }
    // New
    for (int i=0; i<MAX_MAT_UNIFORMS; i++) {
        if ((l->MatUniforms[i].Name==NULL) || (strcmp(l->MatUniforms[i].Name, "")==0)) {
            l->MatUniforms[i].Name = name;
            l->MatUniforms[i].N = n;
            l->MatUniforms[i].M = m;
            mat4x4_dup(l->MatUniforms->Value, val);
            return;
        }
    }
}

void LightRemoveIntUniform(LightInstance* l, char* name) {
    for (int i=0; i<MAX_TEX_UNIFORMS; i++) {
        if (l->TexUniforms[i].Name==NULL) continue;
        if (strcmp(l->TexUniforms[i].Name, name)==0) 
            l->TexUniforms[i].Name = NULL;
    }
}

void LightRemoveVecUniform(LightInstance* l, char* name) {
    for (int i=0; i<MAX_VEC_UNIFORMS; i++) {
        if (l->VecUniforms[i].Name==NULL) continue;
        if (strcmp(l->VecUniforms[i].Name, name)==0) 
            l->VecUniforms[i].Name = NULL;
    }
}

void LightRemoveMatUniform(LightInstance* l, char* name) {
    for (int i=0; i<MAX_MAT_UNIFORMS; i++) {
        if (l->MatUniforms[i].Name==NULL) continue;
        if (strcmp(l->MatUniforms[i].Name, name)==0) 
            l->MatUniforms[i].Name = NULL;
    }
}








// ORIGIN INSTANCE
// Generic Constructor/Deconstructor
static void _OriginDefault(OriginInstance* o) {
    o->Name = "NewOrigin";
    o->ClassName = "BaseOrigin";
    o->Parent = NULL;
    o->CanRender = GL_TRUE;
    
    // Origin
    mat4x4_identity(o->CFrame);
    o->NumParts = o->NumLights = o->NumOrigins = 0;
    for (int i=0; i<MAX_PART_INSTANCES; i++) o->Parts[i] = NULL;
    for (int i=0; i<MAX_LIGHT_INSTANCES; i++) o->Lights[i] = NULL;
    for (int i=0; i<MAX_ORIGIN_INSTANCES; i++) o->Origins[i] = NULL;  

    // Bounding Box
    vec3_zero(o->BoundingBoxSize);
    mat4x4_identity(o->BoundingBoxCFrame);

    // Internal
    o->_Malloced = GL_FALSE;
}

OriginInstance* NewOriginInstance() {
    OriginInstance* o = malloc(sizeof(OriginInstance));
    _OriginDefault(o);
    o->_Malloced = GL_TRUE;
    return o;
}

void* DestroyOriginInstance(OriginInstance* o) {
    // Remove Instances
    OriginSetParent(o, NULL);
    for (int i=0; i<MAX_PART_INSTANCES; i++) {
        PartInstance* child = o->Parts[i];
        o->Parts[i] = NULL;

        if (child==NULL) break;
        child->Parent = NULL;
        DestroyPartInstance(child);
    }
    for (int i=0; i<MAX_LIGHT_INSTANCES; i++) {
        LightInstance* child = o->Lights[i];
        o->Lights[i] = NULL;
        
        if (child==NULL) break;
        child->Parent = NULL;
        DestroyLightInstance(child);
    }
    for (int i=0; i<MAX_ORIGIN_INSTANCES; i++) {
        OriginInstance* child = o->Origins[i];
        o->Origins[i] = NULL;
        
        if (child==NULL) break;
        child->Parent = NULL;
        DestroyOriginInstance(child);
    }
    
    if (o->_Malloced) {
        o->_Malloced = GL_FALSE;
        free(o);
    }
    return NULL;
}


// Custom constructors
OriginInstance* arcPart(PartInstance* arcit,
                        vec3 pos, vec3 rot, vec3 scale, 
                        double a, double width, int n
) {
    // TODO
    return NULL;
}


OriginInstance* repeatPart(PartInstance* ref, int num, vec3 range) {
    OriginInstance* parent = NewOriginInstance();

    for (int i=0; i<num; i++) {
        PartInstance* ch = clonePart(ref); {
            vec3 pos;
            vec3_scale(pos, range, (float)i/(float)(num-1));
            PartSetPosition(ch, pos);
            PartSetParent(ch, parent);
        }
    }

    return parent;
}


OriginInstance* roof(PartInstance* ref, float width0, float width1, float depth, float height, float in0, float in1) {
    OriginInstance* parent = NewOriginInstance();

    PartInstance* nwp;

    // Middle
    nwp = clonePart(ref); {
        PartSetSize(nwp, (vec3){depth, in0, width0});
        PartSetPivot(nwp, PIV_TOP);
        PartSetPosition(nwp, (vec3){0.,height,0.});
        PartSetParent(nwp, parent);
    }

    float w2 = (width1-width0)/2;
    float a = atan(w2/height);
    float h = sqrt((w2*w2) + (height*height));

    nwp = clonePart(ref); {
        PartSetSize(nwp, (vec3){depth, h, in1});
        PartSetPivot(nwp, (vec3){0, PIV_TOP[1], PIV_BCK[2]});
        PartSetRotation(nwp, (vec3){a, 0., 0.}, EULER_XYZ);
        PartSetPosition(nwp, (vec3){0., height, -width0/2.});
        PartSetParent(nwp, parent);
    }
    nwp = clonePart(ref); {
        PartSetSize(nwp, (vec3){depth, h, in1});
        PartSetPivot(nwp, (vec3){0, PIV_TOP[1], PIV_FWD[2]});
        PartSetRotation(nwp, (vec3){-a, 0., 0.}, EULER_XYZ);
        PartSetPosition(nwp, (vec3){0., height, width0/2.});
        PartSetParent(nwp, parent);
    }

    return parent;
}


OriginInstance* circlePart(PartInstance* ref, int num,
                           float degrees, float dist, float incline_angle) {
    

    OriginInstance* o = NewOriginInstance();
    for (int i=0; i<num; i++) {
        float a = TO_RAD * (float)i * degrees/(float)num;
        PartInstance* p = clonePart(ref); {
            PartSetParent(p, o);
            PartSetRotation(p, (vec3){a, incline_angle,0}, EULER_YZX);
            PartSetPosition(p, (vec3){dist*sin(a), 0, dist*cos(a)});
        }
    }

    return o;
}

OriginInstance* cloneOrigin(OriginInstance* o) {
    OriginInstance* new = NewOriginInstance();
    
    new->Name = o->Name;
    new->ClassName = o->ClassName;
    new->CanRender = o->CanRender;

    mat4x4_dup(new->CFrame, o->CFrame);
    for (int i=0; i<MAX_PART_INSTANCES; i++) {
        PartInstance* ch = o->Parts[i];
        if (ch==NULL) break;

        ch = clonePart(ch);
        PartSetParent(ch, new);
    }
    for (int i=0; i<MAX_LIGHT_INSTANCES; i++) {
        LightInstance* ch = o->Lights[i];
        if (ch==NULL) break;

        ch = cloneLight(ch);
        LightSetParent(ch, new);
    }
    for (int i=0; i<MAX_ORIGIN_INSTANCES; i++) {
        OriginInstance* ch = o->Origins[i];
        if (ch==NULL) break;

        ch = cloneOrigin(ch);
        OriginSetParent(ch, new);
    }
    vec3_dup(new->BoundingBoxSize, o->BoundingBoxSize);
    mat4x4_dup(new->BoundingBoxCFrame, o->BoundingBoxCFrame);

    return new;
}


// Methods
void OriginSetParent(OriginInstance* o, OriginInstance* parent) {
    if (o==parent) Error("Cannot parent origin to itself");
    if (o->Parent!=NULL)
        _OriginRemoveOrigin(o->Parent, o);
    
    if (parent!=NULL) {
        int res = _OriginAddOrigin(parent, o);
        if (res<0)
            Error("Could not add new origin (%s) to origin (%s)", o->Name, parent->Name);
    }
    o->Parent = parent;
}

void OriginSetPosition(OriginInstance* o, vec3 pos) {
    vec3_dup(o->CFrame[3], pos);
}

void OriginSetRotation(OriginInstance* o, vec3 rot, int euler_order) {
    mat4x4 temp;
    mat4x4_from_euler(temp, rot, euler_order);
    for (int i=0; i<3; i++)
        vec4_dup(o->CFrame[i], temp[i]);
}

void OriginSetCFrame(OriginInstance* o, mat4x4 cf) {
    mat4x4_dup(o->CFrame, cf);
}

PartInstance* OriginFindFirstPart(OriginInstance* o, char* Name, unsigned char recursive_depth) {
    for (int i=0; i<MAX_PART_INSTANCES; i++) {
        if ((o->Parts[i]!=NULL) && (strcmp(o->Parts[i]->Name, Name)==0))
            return o->Parts[i];
    }
    if (recursive_depth) for (int i=0; i<o->NumOrigins; i++) {
        if (o->Origins[i]==NULL) 
            break;
        
        PartInstance* ret = OriginFindFirstPart(o->Origins[i], Name, recursive_depth-1);
        if (ret!=NULL) 
            return ret;
    }
    return NULL;
}

LightInstance* OriginFindFirstLight(OriginInstance* o, char* Name, unsigned char recursive_depth) {
    for (int i=0; i < MAX_LIGHT_INSTANCES; i++) {
        if ((o->Lights[i]!=NULL) && (strcmp(o->Lights[i]->Name, Name)==0))
            return o->Lights[i];
    }
    if (recursive_depth) for (int i=0; i < o->NumOrigins; i++) {
        if (o->Origins[i]==NULL) 
            break;
        
        LightInstance* ret = OriginFindFirstLight(o->Origins[i], Name, recursive_depth-1);
        if (ret!=NULL) 
            return ret;
    }
    return NULL;
}

OriginInstance* OriginFindFirstOrigin(OriginInstance* o, char* Name, unsigned char recursive_depth) {
    for (int i=0; i<MAX_ORIGIN_INSTANCES; i++) {
        if ((o->Origins[i]!=NULL) && (strcmp(o->Origins[i]->Name, Name)==0))
            return o->Origins[i];
    }
    if (recursive_depth) for (int i=0; i < o->NumOrigins; i++) {
        if (o->Origins[i]==NULL) 
            break;
        
        OriginInstance* ret = OriginFindFirstOrigin(o->Origins[i], Name, recursive_depth-1);
        if (ret!=NULL) 
            return ret;
    }
    return NULL;
}

void OriginUpdateBoundingBox(OriginInstance* o) {
    if (!(o->Parts[0])) {
        mat4x4_identity(o->BoundingBoxCFrame);
        vec3_zero(o->BoundingBoxSize);
        return;
    }
    
    mat4x4 rot;
    vec3 minv = {0,0,0}, maxv = {0,0,0};
    mat4x4_invert(rot, o->Parts[0]->CFrame);    // Get inverted rotation matrix of first part
    vec4_dup(rot[3], UNIT4_W);                  // Remove position
    
    // Find mim and max bounds of each part on each axis
    for (int i=0; i<MAX_PART_INSTANCES; i++) {
        PartInstance* ch = o->Parts[i];
        if (ch==NULL) break;

        mat4x4 use;
        mat4x4_mul(use, ch->CFrame, rot);
        for (int x=0; x<2; x++)
        for (int y=0; y<2; y++)
        for (int z=0; z<2; z++) {
            vec3 corner;
            mat4x4 M;
            vec3_comp_mul(corner, ch->Size, (vec3){x?1:-1, y?1:-1, z?1:-1});
            vec3_scale(corner, corner, .5);
            mat4x4_from_pos(M, corner);
            mat4x4_mul(M, use, M);
            for (int j=0; j<3; j++) {
                minv[j] = min(minv[j], M[3][j]);
                maxv[j] = max(maxv[j], M[3][j]);
            }
        }
    }

    // Calculate bounding box
    vec3 cen;
    vec3_sub(cen, maxv, minv);          // Find range
    vec3_dup(o->BoundingBoxSize, cen);
    
    vec3_scale(cen, cen, .5);   // Divide in half
    vec3_add(cen, minv, cen);    // Add half range on minimum
    
    mat4x4_invert(rot, rot);    // Grab rotation 
    vec3_dup(rot[3], cen);      // Grab Translation
    mat4x4_dup(o->BoundingBoxCFrame, rot);
}


static int _OriginAddPart(OriginInstance* o, PartInstance* p) {
    for (int i=0; i<MAX_PART_INSTANCES; i++) {
        if (o->Parts[i]==NULL) {
            o->Parts[i]=p;
            OriginUpdateBoundingBox(o);
        }
        if (o->Parts[i]==p) {
            return i;
        }
    }
    return -1;
}

static int _OriginRemovePart(OriginInstance* o, PartInstance* p) {
    for (int i=0; i<MAX_PART_INSTANCES; i++) {
        if (o->Parts[i]==p) {
            for (int j=i; j<MAX_PART_INSTANCES-1; j++) 
                o->Parts[j]=o->Parts[j+1];
            o->Parts[MAX_PART_INSTANCES-1]=NULL;
            OriginUpdateBoundingBox(o);
            return 0;
        }
    }
    return -1;
}

static int _OriginAddLight(OriginInstance* o, LightInstance* l) {
    for (int i=0; i<MAX_LIGHT_INSTANCES; i++) {
        if (o->Lights[i]==NULL) {
            o->Lights[i]=l;
        }
        if (o->Lights[i]==l) {
            return i;
        }
    }
    return -1;
}

static int _OriginRemoveLight(OriginInstance* o, LightInstance* l) {
    for (int i=0; i<MAX_LIGHT_INSTANCES; i++) {
        if (o->Lights[i]==l) {
            for (int j=i; j<MAX_LIGHT_INSTANCES-1; j++) 
                o->Lights[j]=o->Lights[j+1];
            o->Lights[MAX_LIGHT_INSTANCES-1]=NULL;
            return 0;
        }
    }
    return -1;
}

static int _OriginAddOrigin(OriginInstance* o, OriginInstance* ch) {
    for (int i=0; i<MAX_ORIGIN_INSTANCES; i++) {
        if (o->Origins[i]==NULL) {
            o->Origins[i]=ch;
        }
        if (o->Origins[i]==ch) {
            return i;
        }
    }
    return -1;
}

static int _OriginRemoveOrigin(OriginInstance* o, OriginInstance* ch) {
    for (int i=0; i<MAX_ORIGIN_INSTANCES; i++) {
        if (o->Origins[i]==ch) {
            for (int j=i; j<MAX_ORIGIN_INSTANCES-1; j++) 
                o->Origins[j]=o->Origins[j+1];
            o->Origins[MAX_ORIGIN_INSTANCES-1]=NULL;
            return 0;
        }
    }
    return -1;
}








// JUMP INSTANCE
// Generic Constructor/Deconstructor
// NewJumpInstance
    // @brief Creates a new Jump Instance, used for rendering an external verse object,
    // the camera is not currently hosted in; Uses malloc to allocate object memory
    // @return Allocated JumpInstance address.
JumpInstance* NewJumpInstance() {
    JumpInstance* j = (JumpInstance*) malloc(sizeof(JumpInstance));
    // Book keeping
    j->Name = "NewJump";
    j->ClassName = "Jump";
    j->Verse0 = NULL;
    j->Verse1 = NULL;
    j->RenderIn0 = 1;
    j->RenderIn1 = 1;

    // Jumping
    j->Depth = 0.f;
    mat4x4_identity(j->CFrame0);
    mat4x4_identity(j->CFrame1);
    mat4x4_identity(j->_ToV0);
    mat4x4_identity(j->_ToV1);

    // Portal Geometry
    _OriginDefault(&j->Root);
    j->Root.Parent = NULL;
    j->Root._Malloced = GL_FALSE;

    // Internal
    j->_Malloced = GL_TRUE;

    // Add in occlusion light 
    {
        LightInstance* occ = globalOcclusion();
        occ->Name = "Occlusion";
        LightSetParent(occ, &j->Root);
    }

    // Events
    return j;
}

// DestroyJumpInstance
    // @brief Deallocates memory for a JumpInstance object
    // @param j The jump instance to free up
void *DestroyJumpInstance(JumpInstance* j) {
    JumpConnectVerses(j, NULL, NULL);
    DestroyOriginInstance(&j->Root);
    if (j->_Malloced) {
        j->_Malloced = GL_FALSE;
        free(j);
    }
    return NULL;
}

// Primitive Jump Constructors
JumpInstance* portal(float radius) {
    JumpInstance* j = NewJumpInstance();
    PartInstance* p = circle(HI_DETAIL, radius);
    PartSetParent(p, &j->Root);
    
    return j;
}


// Methods
// JumpConnectVerses
    // @brief Will bind to verses together with a JumpInstance; 
    // ordering of verses should follow same ordering of JumpSetCFrames
    // @param j Jump Instance to bind verses on
    // @pram v0 First verse to connect
    // @param v1 Second verse to connect
void JumpConnectVerses(JumpInstance* j, VerseInstance* v0, VerseInstance* v1) {
    if (j->Verse0) _VerseRemoveJump(j->Verse0, j);
    if (j->Verse1) _VerseRemoveJump(j->Verse1, j);
    if (v0!= NULL) _VerseAddJump(v0, j);
    if (v1!= NULL) _VerseAddJump(v1, j);
    j->Verse0=v0;
    j->Verse1=v1;
}

// JumpSetCFrames
    // @brief Will bind to verses together with a JumpInstance; 
    // ordering of verses should follow same ordering of JumpConnectVerses
    // @param j Jump Instance to bind verses on
    // @pram CF0 CFrame location in j.Verse0 to render v.Verse1 at; Portal drawn around Z axis
    // @param CF1 CFrame location in j.Verse1 to render v.Verse0 at; Portal drawn around Z axis
void JumpSetCFrames(JumpInstance* j, mat4x4 CF0, mat4x4 CF1) {
    mat4x4_dup(j->CFrame0, CF0);
    mat4x4_dup(j->CFrame1, CF1);

    // Calculate a matrix to translate CFrames between verses
    // Repeat for both sides
    for (int k=0; k<2; k++) {
        mat4x4 ToB, M;
        
        // First apply source CFrame matrix
        // This exists in the current
        mat4x4_dup(ToB, (!k)? CF0 : CF1); // Source
        
        // Rotate by 180 degrees (in place)
        // This is because portals always are drawn in the same direction, but are opposite ends in translated space
        // Commented out on update allowing portals to have arbitrary geometry
        /*
        mat4x4_identity(M);
        mat4x4_rotate_Y(M,M,180*TO_RAD);
        mat4x4_mul(ToB, ToB, M);
        */

        // Apply inverse of destination CFrame
        // Similar to Camera, we want the offset of the jump to the desination verses's origin
        mat4x4_invert(M, (!k)? CF1 : CF0);
        mat4x4_mul(ToB, ToB, M);

        // Set appropriate property
        mat4x4_dup((!k)? j->_ToV1 : j->_ToV0, ToB); // Output Translation
    }
}

void JumpSetOffsets(JumpInstance* j, mat4x4 Off0, mat4x4 Off1) {
    mat4x4_dup(j->CFrame0, Off0);
    mat4x4_dup(j->CFrame1, Off1);
}








// VERSE INSTANCE
// Methods
// VerseFindFirstJump
    // @brief Will return the first JumpInstance parented under a VerseInstance with a given Name
    // @param v Jump Instance to search
    // @param Name Name to search for
    // @return If a Jump Instance is found with the corresponding name, it is returned;
    // Otherwise NULL is returned
JumpInstance* VerseFindFirstJump(VerseInstance* v, char* Name) {
    for (int i=0; i<MAX_JUMP_INSTANCES; i++) {
        if ( (v->Jumps[i]!=NULL) && (strcmp(v->Jumps[i]->Name, Name)==0) )
            return v->Jumps[i];
    }
    return NULL;
}


// VerseAddJump
    // @brief Adds a jump under a VerseInstance
    // @param v Verse Instance to add part to
    // @param j Jump Instance to be added
    // @return 0 on success; -1 if part could not be added
static int _VerseAddJump(VerseInstance* v, JumpInstance* j) {
    for (int i=0; i<MAX_JUMP_INSTANCES; i++) {
        if (v->Jumps[i]==NULL) {
            v->Jumps[i]=j;
        }
        if (v->Jumps[i]==j) {
            return i;
        }
    }
    return -1;
}

// VerseRemoveJump
    // @brief Removes a jump under a VerseInstance; Does not free jump from memory
    // @param v Verse Instance to remove jump from
    // @param p Jump Instance to be removed
    // @return 0 on success; -1 if jump could not be removed/found
static int _VerseRemoveJump(VerseInstance* v, JumpInstance* j) {
    for (int i=0; i<MAX_JUMP_INSTANCES; i++) {
        if (v->Jumps[i]==j) {
            for (int j=i; j<MAX_JUMP_INSTANCES-1; j++) 
                v->Jumps[j]=v->Jumps[j+1];
            v->Jumps[MAX_JUMP_INSTANCES-1]=NULL;
            return 0;
        }
    }
    return -1;
}








// Shaders
// Credit to Willem A. (Vlakkies) Schreuder
    // @brief Will print log of information about built shader; used for debugging
static void _PrintShaderLog(GLuint shader, char* filename) {
    int len=0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len>1) {
        int n=0;
        char* buffer = (char*)malloc(len);
        if (!buffer) Error("Cannot allocate %d bytes fir shader log\n", len);
        glGetShaderInfoLog(shader, len, &n, buffer);
        fprintf(stderr, "%s:\n%s\n", filename, buffer);
    }   
    glGetShaderiv(shader, GL_COMPILE_STATUS, &len);
    if (!len) Error("Error compiling %s\n",filename);
}

// Credit to Willem A. (Vlakkies) Schreuder
    // @brief Will print log of information about built shader; used for debugging
static void _PrintProgramLog(GLuint prog) {
    int len=0;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len>1) {
        int n=0;
        char* buffer = (char*) malloc(len);
        if (!buffer) Error("Cannot allocate %d bytes of text for program log\n", len);
        glGetProgramInfoLog(prog, len, &n, buffer);
        printf("%s\n", buffer);
    }
    glGetProgramiv(prog, GL_LINK_STATUS, &len);
    if (!len) Error("Error Linking Program");
}

// NewShaderObjFromFile
    // @brief Will compiler a shader from a given file's source text;
    // prints any errors in compiling individual shaders
    // @param type The type of shader to create
    // @param filename The name of the file to read
    // @throws Will throw a fatal error if shader file cannot be read correctly
    // @return Returns 
GLuint NewShaderObjFromFile(GLenum type, char* filename) {
    // Open source file and get size
    FILE* fp = fopen(filename, "r");
    if (!fp) Error("Cannot open file: %s\n", filename);
    fseek(fp,0,SEEK_END);
    int n = ftell(fp);
    rewind(fp);

    // Allocate and read source
    char* source = malloc(n+1);
    if (!source) Error("Could not allocate space for shader (%s) source\n", filename);
    if (fread(source,sizeof(char),n,fp)!=n) Error("Could not read shader (%s) source\n", filename);
    source[n]='\x00';
    fclose(fp); 

    // Build Shader Object
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char**) &source, NULL);
    glCompileShader(shader);
    free(source);

    _PrintShaderLog(shader, filename);
    return shader;
}

GLuint NewShaderFromObjs(int n, GLuint objs[]) {
    GLuint SHADER = glCreateProgram();

    for (int i=0; i<n; i++) {
        glAttachShader(SHADER, objs[i]);
    }
    glProgramParameteri(SHADER, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glLinkProgram(SHADER);
    _PrintProgramLog(SHADER);

    for (int i=0; i<n; i++) {
        glDetachShader(SHADER, objs[i]);
    }
    return SHADER;
}

GLuint NewShaderFromFile(GLenum type, char* filename) {
    GLuint node, shader;
    node = NewShaderObjFromFile(type, filename);
    shader = NewShaderFromObjs(1, (GLuint[]){node});
    glDeleteShader(node);
    return shader;
}

// Custom shader constructors
GLuint simpleVertNode() {
    static GLuint prog;
    static int init = 0;
    if (!init) {
        init = 1;
        prog = NewShaderFromFile(GL_VERTEX_SHADER, "shdr/simple.vert");
    }
    return prog;
}

GLuint simpleFragNode() {
    static GLuint prog;
    static int init = 0;
    if (!init) {
        init = 1;
        prog = NewShaderFromFile(GL_FRAGMENT_SHADER, "shdr/simple.frag");
    }
    return prog;
}


// Methods
void UpdateShaderFromObjs(GLuint SHADER, int n, GLuint objs[]) {
    for (int i=0; i<n; i++) {
        glAttachShader(SHADER, objs[i]);
    }
    glProgramParameteri(SHADER, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glLinkProgram(SHADER);
    _PrintProgramLog(SHADER);

    for (int i=0; i<n; i++) {
        glDetachShader(SHADER, objs[i]);
    }
}

void UpdateShaderFromFile(GLuint SHADER, GLenum type, char* filename) {
    GLuint node;
    node = NewShaderObjFromFile(type, filename);
    UpdateShaderFromObjs(SHADER, 1, (GLuint[]){node});
    glDeleteShader(node);
}
