// Multi_verse: RE(Rendering Engine)
// Gavin Zimmerman


// RENDERING PIPELINE
#ifndef RENDER_H
#define RENDER_H

// DEPENDENCIES
#include "graphics.h"
#include "objects.h"
#include "signal.h"
#include "linmath.h"


// CONST
#define DRAW_INFO_ROW       6

#define MAX_RENDER_VERSES           64
#define MAX_RENDER_JUMP_DEPTH       2
#define MAX_BATCH_PART_INSTANCES    8096
#define MAX_BATCH_LIGHT_INSTANCES   512




// STRUCTS
struct RenderInstanceData_s {
    void* Instance;
    mat4x4 ViewMat;
};
typedef struct RenderInstanceData_s RenderInstanceData;


struct RenderData_s {
    // Instances to render
    int NumParts, NumLights;
    RenderInstanceData Parts[MAX_BATCH_PART_INSTANCES];
    RenderInstanceData Lights[MAX_BATCH_LIGHT_INSTANCES];
    
    // Data to segment bindings and pass onto shader attribute inputs
    int NumDrawing;
    GLuint DrawData[MAX_BATCH_PART_INSTANCES][DRAW_INFO_ROW];
    float  AttribData[MAX_BATCH_PART_INSTANCES][INST_ATTRIB_ROW];
};
typedef struct RenderData_s RenderData;


struct RenderJumpData_s {
    // Related objects
    JumpInstance* j;
    VerseInstance* JumpTo;

    // Positionals
    int Direction;
    vec3 BoundsMin, BoundsMax;
    mat4x4 OffsetTo, OffsetFrom;

    // Travel log (for nested jumping)
    int TravelDepth;
    int Travel[MAX_RENDER_JUMP_DEPTH];
};
typedef struct RenderJumpData_s RenderJumpData;


struct CameraData_s {
    VerseInstance* CameraVerse;
    mat4x4 CameraCFrame;
};
typedef struct CameraData_s CameraData;


struct UniformData_s {
    // World uniforms
    float Time;
    vec4 ClipPlane[6];
    vec2 Resolution, Mouse;
    mat4x4 ViewMat, ProjectionMat;
    GLuint Texture;

    // Light Uniforms
    mat4x4 LightViewMat;
    LightInstance* Light;
};
typedef struct UniformData_s UniformData;




// EVENTS
extern SignalInstance PreRender, PostRender;




// METHODS
void renderInit(GLFWwindow* window);
void renderExit(GLFWwindow* window);
void renderStep(GLFWwindow* window, float time, float step);

void setRenderCameraCFV(mat4x4 CFrame, VerseInstance* verse);
void getRenderCameraCFV(mat4x4 CFrame, VerseInstance** verse);
void getRenderUIVerse(VerseInstance** verse);


#endif