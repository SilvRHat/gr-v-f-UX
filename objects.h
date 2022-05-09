// Multi_verse: RE(Rendering Engine)
// Gavin Zimmerman

// Mv:RE Objects
// Provides instances for rendering Multi_verse scenes
#ifndef OBJECTS_H
#define OBJECTS_H

// OBJECTS provides structures for building Multi_verse scenes:
// Parts    Physical objects which can be drawn; this structure
//          includes various properties which influence what/ how
//          the object is drawn.
// Lights   Passes on rendering pipeline; this structure defines
//          parameters for an additional pass on the scene. Inspired
//          from forward-rendering, but acknowledges full ability of
//          additional passes unlike most game engines. 
// Origins  Holder of the above and local coordinate space; this
//          structure allows easy transformations of parts
// Jumps    Portals, space manipulation; this structure is unique
//          to the rendering pipeline and allows easy rendering of
//          multiple scenes.
// Verses   Scene/ Level; the objects which are connectable by jumps

// All these structures are named as instances, since they hold numerous
// member variables for describing the state of an instance. A number of
// non-instance functions are also included which allow easy creation of
// OpenGL objects such as shader programs and buffers. These follow a
// slightly different convention


// DEPENDENCIES
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>

#include "graphics.h"
#include "signal.h"
#include "linmath.h"
#include "utils.h"

// Shader files
// <shdr/simple.vert>
// <shdr/simple.frag>




// Methods
void buildVAOVBO(GLuint* vao_ptr, GLuint *vbo_ptr, float vertex_data[], int n);
GLuint getUniqueLightGroup();
GLuint getInstancedAttribVBO();


// CONST
// ////////////////////////////////////////////

// Standardized between verses
    // Projection Matrices should alwways use these values, or objects will not match in clipspace (between verses)
#define NEAR_PROJECTION 1.f
#define FAR_PROJECTION  2048


// Structure
#define VERT_ATTRIB_ROW     9
#define INST_ATTRIB_ROW     48
#define INST_ATTRIB_COL     2024


// Max Child Instances (per appropriate parent instance)
#define MAX_PART_INSTANCES      1024    // OriginInstance
#define MAX_LIGHT_INSTANCES     256     // OriginInstance
#define MAX_ORIGIN_INSTANCES    1024    // OriginInstance
#define MAX_JUMP_INSTANCES      64      // VerseInstance
#define MAX_LIGHTGROUPS         32      // PartInstance
#define MAX_VEC_UNIFORMS        16      // LightInstance
#define MAX_TEX_UNIFORMS        16      // LightInstance
#define MAX_MAT_UNIFORMS        4       // LightInstance


// Standardized detail-level (reflects how many vertices are used in objects)
    // Use same VBO for similarly-detailed objects
#define XXHI_DETAIL  128
#define XHI_DETAIL   64
#define HI_DETAIL    32
#define MED_DETAIL   16
#define LO_DETAIL    8
#define XLO_DETAIL   4


#define PIV_CEN (vec3){  0,  0,  0}
#define PIV_LFT (vec3){-.5,  0,  0}
#define PIV_RGT (vec3){ .5,  0,  0}
#define PIV_BOT (vec3){  0,-.5,  0}
#define PIV_TOP (vec3){  0, .5,  0}
#define PIV_BCK (vec3){  0,  0,-.5}
#define PIV_FWD (vec3){  0,  0, .5}


#define ENUM_LIGHT_NONE         0
#define ENUM_LIGHT_POINT        1
#define ENUM_LIGHT_SPOT         2
#define ENUM_LIGHT_AMBIENT      3
#define ENUM_LIGHT_OCCLUSION    4


#define MAX_NODES               3

#define ENUM_NODE0              0
#define ENUM_NODE1              1
#define ENUM_NODE2              2
#define ENUM_NODE0_OVERSHADE    MAX_NODES+0
#define ENUM_NODE1_OVERSHADE    MAX_NODES+1
#define ENUM_NODE2_OVERSHADE    MAX_NODES+2


// Shader attribute layout positions
#define ATTRIB_POS_VERTEX       0
#define ATTRIB_POS_NORMAL       1
#define ATTRIB_POS_UV_MAP       2
#define ATTRIB_POS_MODELVIEW    3   // 3, 4, 5, 6
#define ATTRIB_POS_NORMALMAT    7   // 7, 8, 9
#define ATTRIB_POS_SIZE         10
#define ATTRIB_POS_COLOR        11
#define ATTRIB_POS_NODE         12  // 12, 13, 14, 15


// Shader default uniform names
#define UNI_NAME_VIEWMAT        "u_ViewMat"
#define UNI_NAME_PROJECTIONMAT  "u_ProjectionMat"
#define UNI_NAME_CLIPPLANE      "u_ClipPlane"
#define UNI_NAME_RESOLUTION     "u_Resolution"
#define UNI_NAME_MOUSEPOS       "u_Mouse"
#define UNI_NAME_TIME           "u_Time"
#define UNI_NAME_TEXTURE        "u_Texture"
#define UNI_NAME_LIGHTVIEWMAT   "l_LightViewMat"
#define UNI_NAME_LIGHTENUM      "l_Enum"
#define UNI_NAME_LIGHTCOLOR     "l_Color"

// Standard non-default uniform names
#define UNI_NAME_LIGHT_RADIUS   "l_Radius"
#define UNI_NAME_LIGHT_THETA0   "l_Theta0"
#define UNI_NAME_LIGHT_THETA1   "l_Theta1"


#define GLOBAL_LIGHTGROUP 1


// SIMPLE DATATYPES
// ////////////////////////////////////////////
struct TexUniformData_s {
    char* Name;
    GLuint Value;
    GLenum BindTarget;
};
typedef struct TexUniformData_s TexUniformData;

struct VecUniformData_s {
    char* Name;
    vec4 Value;
    GLuint N;
};
typedef struct VecUniformData_s VecUniformData;

struct MatUniformData_s {
    char* Name;
    mat4x4 Value;
    GLuint N, M;
};
typedef struct MatUniformData_s MatUniformData;

struct BufferData_s {
    GLuint UName;

    GLenum Type;
    GLenum Target;
    GLenum Format;
    GLint InternalFormat;
    GLsizei Dimensions[4];
};
typedef struct BufferData_s BufferData;





// GAME INSTANCES
// ////////////////////////////////////////////
struct PartInstance_s;
struct LightInstance_s;
struct OriginInstance_s;
struct JumpInstance_s;
struct VerseInstance_s;
typedef struct PartInstance_s PartInstance;
typedef struct LightInstance_s LightInstance;
typedef struct OriginInstance_s OriginInstance;
typedef struct JumpInstance_s JumpInstance;
typedef struct VerseInstance_s VerseInstance;

struct PartInstance_s {
    // Book Keeping
    char* Name;
    char* ClassName;
    OriginInstance* Parent;
    GLboolean CanRender;

    // Positional & Scaling
    vec3 Size;
    vec3 Pivot;
    mat4x4 CFrame;

    // Shader Panel
    GLuint PipeNodes[MAX_NODES];
    GLuint VertNodes[MAX_NODES];
    GLuint FragNodes[MAX_NODES];
    mat4x4 NodeAttrs[MAX_NODES];

    // Texturing
    vec4 Color;
    GLuint Texture;
    GLuint LightGroups[MAX_LIGHTGROUPS];

    // Vertex Data
    GLenum DrawMode;
    GLuint Vao, Vbo;
    GLboolean VboReused;
    GLboolean SeperatedNodes;
    unsigned int Vertices;

    // Internal
    GLboolean _Malloced;
};

struct LightInstance_s {
    // Book Keeping
    char* Name;
    char* ClassName;
    OriginInstance* Parent;
    GLboolean CanRender;

    // Default Light Uniforms
    vec4 Color;
    mat4x4 CFrame;
    GLenum LightEnum;

    // Shader settings
    GLuint OutFBO;
    GLenum Noding;
    GLuint OverShadeNode;
    GLenum BlendSrc, BlendDst;
    mat4x4 ProjectionMatrix;

    int ZIndex;
    GLuint LightGroup;
    GLboolean UseTexture;

    // Configurable inputs
    TexUniformData TexUniforms[MAX_TEX_UNIFORMS];
    VecUniformData VecUniforms[MAX_VEC_UNIFORMS];
    MatUniformData MatUniforms[MAX_MAT_UNIFORMS];

    // Internal
    GLboolean _Malloced;

    // Events
    SignalInstance PrePass;
    SignalInstance PostPass;
};

struct OriginInstance_s {
    // Book Keeping
    char *Name;
    char *ClassName;
    OriginInstance* Parent;
    GLboolean CanRender;

    // Origin
    mat4x4 CFrame;
    GLuint NumParts, NumLights, NumOrigins;
    PartInstance* Parts[MAX_PART_INSTANCES];
    LightInstance* Lights[MAX_LIGHT_INSTANCES];
    OriginInstance* Origins[MAX_ORIGIN_INSTANCES];

    // Bounding Box
    vec3 BoundingBoxSize;
    mat4x4 BoundingBoxCFrame;
    
    // Internal
    GLboolean _Malloced;
};

struct JumpInstance_s {
    // Book Keeping
    char *Name;
    char *ClassName;
    VerseInstance *Verse0, *Verse1;
    GLboolean RenderIn0, RenderIn1;

    // Jumping
    float Depth;
    mat4x4 CFrame0, CFrame1;
    mat4x4 _ToV0, _ToV1; 

    // Portal Geometry
    OriginInstance Root;

    // Internal
    GLboolean _Malloced;
};

struct VerseInstance_s {
    // Book Keeping
    char* Name;
    char *ClassName;

    // Components
    GLuint NumJumps;
    OriginInstance Root;
    JumpInstance*  Jumps[MAX_JUMP_INSTANCES];
    mat4x4 ProjectionMatrix;    // Only used in portal clipping

    // Per-Instance Methods
        // @brief Function to construct/ init the verse instance
        // @param window GLFW window context
    void (*Build) (GLFWwindow*);
        // @brief Function to deconstruct/ cleanup the verse instance
        // @param window GLFW window context
    void (*Clean) (GLFWwindow*);

    // Events
    SignalInstance PreRender;
    SignalInstance PostRender;
};






// PART INSTANCE
// Generic Constructor/Deconstructor
PartInstance* NewPartInstance();
void* DeletePartInstanceBuffers(PartInstance* p);
void* DestroyPartInstance(PartInstance* p);

// Primitive Object constructors
PartInstance* point(vec4 pt);
PartInstance* primitive(int n, vec4 pts[], vec3 norms[], vec2 uvs[], GLenum DrawType);
PartInstance* canvas(float size);
PartInstance* circle(int sides, float radius);
PartInstance* cube(float size);
PartInstance* uvSphere(int segments, int rings, float radius);
PartInstance* icoSphere(float radius);
PartInstance* cylinder(int sides, float radius, float depth);
PartInstance* clonePart(PartInstance* p);

// Custom Object constructors


// Methods
void PartSetParent(PartInstance* p, OriginInstance* parent);
void PartSetSize(PartInstance* p, vec3 size);
void PartSetPivot(PartInstance* p, vec3 pivot);
void PartSetPosition(PartInstance* p, vec3 pos);
void PartSetRotation(PartInstance* p, vec3 rot, int euler_order);
void PartSetCFrame(PartInstance* p, mat4x4 cf);
void PartSetNodeAttribByName(PartInstance* p, GLenum nodetype, char* name, vec4 val);
void PartSetNodeColorByName(PartInstance* p, GLenum nodetype, char* name, color4 val);
void PartSetColor(PartInstance* p, color4 col);
void PartAddLightGroup(PartInstance* p, GLuint lg);
void PartRemoveLightGroup(PartInstance* p, GLuint lg);





// LIGHT INSTANCE
// Generic Constructor/Deconstructor
LightInstance* NewLightInstance();
void* DestroyLightInstance(LightInstance* l);

// Primitive Light Constructos
LightInstance* globalOcclusion();
LightInstance* ambientLight();
LightInstance* pointLight(float radius);
LightInstance* spotLight(float radius, float inner_angle, float outer_angle);
LightInstance* cloneLight(LightInstance* l);


// Methods
void LightSetParent(LightInstance* l, OriginInstance* parent);
void LightSetColor(LightInstance* l, color4 col);
void LightSetPosition(LightInstance* l, vec3 pos);
void LightSetRotation(LightInstance* l, vec3 rot, int euler_order);
void LightSetCFrame(LightInstance* l, mat4x4 cf);
void LightSetTexUniform(LightInstance* l, char* name, GLuint val, GLenum target);
void LightSetVecUniform(LightInstance* l, char* name, float val[], int n);
void LightSetMatUniform(LightInstance* l, char* name, mat4x4 val, int n, int m);
void LightRemoveTexUniform(LightInstance* l, char* name);
void LightRemoveVecUniform(LightInstance* l, char* name);
void LightRemoveMatUniform(LightInstance* l, char* name);




// ORIGIN INSTANCE
// Generic Constructor/Deconstructor
OriginInstance* NewOriginInstance();
void* DestroyOriginInstance(OriginInstance* o);

// Custom model constructors
OriginInstance* roof(PartInstance* ref, float width0, float width1, float depth, float height, float in0, float in1);
OriginInstance* repeatPart(PartInstance* ref, int num, vec3 range);
OriginInstance* arcPart(PartInstance* arcit,
                        vec3 pos, vec3 rot, vec3 scale, 
                        double a, double width, int n);

OriginInstance* circlePart(PartInstance* ref, int num,
                           float degrees, float dist, float incline_angle);

OriginInstance* cloneOrigin(OriginInstance* o);


// Methods
void OriginSetParent(OriginInstance* o, OriginInstance* parent);
void OriginSetPosition(OriginInstance* p, vec3 pos);
void OriginSetRotation(OriginInstance* p, vec3 rot, int euler_order);
void OriginSetCFrame(OriginInstance* p, mat4x4 cf);
void OriginUpdateBoundingBox(OriginInstance* o);
PartInstance* OriginFindFirstPart(OriginInstance* o, char* name, unsigned char recursive_depth);
LightInstance* OriginFindFirstLight(OriginInstance* o, char* name, unsigned char recursive_depth);
OriginInstance* OriginFindFirstOrigin(OriginInstance* o, char* name, unsigned char recursive_depth);




// JUMP INSTANCE
// Generic Constructor/Deconstructor
JumpInstance* NewJumpInstance();
void *DestroyJumpInstance(JumpInstance* j);

// Primitive Jump Constructors
JumpInstance* portal(float radius);


// Methods
void JumpConnectVerses(JumpInstance* j, VerseInstance* v0, VerseInstance* v1);
void JumpSetCFrames(JumpInstance* j, mat4x4 CF0, mat4x4 CF1);




// VERSE INSTANCE
// Methods
JumpInstance* VerseFindFirstJump(VerseInstance* v, char* Name);




// SHADER/ NODE INSTANCES
// Simple Constructors
GLuint NewShaderObjFromFile(GLenum type, char* filename);
GLuint NewShaderFromObjs(int n, GLuint objs[]);
GLuint NewShaderFromFile(GLenum type, char* filename);

// Custom shader constructors
GLuint simpleVertNode();
GLuint simpleFragNode();


// Methods
void UpdateShaderFromObjs(GLuint shader, int n, GLuint objs[]);
void UpdateShaderFromFile(GLuint shader, GLenum type, char* filename);


#endif