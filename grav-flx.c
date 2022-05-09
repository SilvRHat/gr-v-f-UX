// Gr@v:f/UX
// Gavin:Zimmerman

// Gravity Flux field
#include "grav-flx.h"


// VERSE
static void buildFlux(GLFWwindow* window);
static void clearFlux(GLFWwindow* window);
VerseInstance GRAV_VERSE = {
    .Name = "Gr@v:f/UX",
    .ClassName = "Verse//11",
    
    .Build = buildFlux,
    .Clean = clearFlux,

    .Jumps = {NULL},
    .Root = {
        .Name = "Root",
        .ClassName = "RootOrigin",

        .Parent = NULL,
        .CanRender = GL_TRUE,
        ._Malloced = GL_FALSE,

        .Parts = {NULL},
        .Lights = {NULL},
        .Origins = {NULL}
    }
};

static VerseInstance* self;
static OriginInstance* root;

GLuint lzr_vert_shader, az_vert_shader, post_shader, ui_shader, az_frag, st_frag;


#define GRAV_FRAMEBUFFERS   1
#define GRAV_DEPTHBUFFERS   1
#define GRAV_COLORBUFFERS   1
GLuint frameBuffers[GRAV_FRAMEBUFFERS];
GLuint colorBuffers[GRAV_COLORBUFFERS];
GLuint depthBuffers[GRAV_DEPTHBUFFERS];

static int mode=0;

// HANDLERS
static float frand(float min,float max)
{
    float rng = max-min;
    float off = min;
    return rand()*rng/RAND_MAX+off;
}

static void loadShaders() {
    // Load Shader Objs
    GLuint lZr_objs[4];
    GLuint AZ_objs[5];

    lZr_objs[0] = NewShaderObjFromFile(GL_VERTEX_SHADER, "shdr/lZr/lZr.vert");
    lZr_objs[1] = NewShaderObjFromFile(GL_TESS_CONTROL_SHADER, "shdr/lZr/lZr.tesc");
    lZr_objs[2] = NewShaderObjFromFile(GL_TESS_EVALUATION_SHADER, "shdr/lZr/lZr.tese");
    lZr_objs[3] = NewShaderObjFromFile(GL_GEOMETRY_SHADER, "shdr/lZr/lZr.geom");

    AZ_objs[0] = NewShaderObjFromFile(GL_VERTEX_SHADER, "shdr/AZ/AZ.vert");
    AZ_objs[1] = NewShaderObjFromFile(GL_TESS_CONTROL_SHADER, "shdr/AZ/AZ.tesc");
    AZ_objs[2] = NewShaderObjFromFile(GL_TESS_EVALUATION_SHADER, "shdr/AZ/AZ.tese");
    AZ_objs[3] = NewShaderObjFromFile(GL_GEOMETRY_SHADER, "shdr/AZ/AZ.geom");
    AZ_objs[4] = NewShaderObjFromFile(GL_TESS_EVALUATION_SHADER, "shdr/AZ/musgrave.glsl");
    


    // Build
    static int init = 0;
    if (!init) {
        init=1;
        lzr_vert_shader = NewShaderFromObjs(4, lZr_objs);
        az_vert_shader = NewShaderFromObjs(5, AZ_objs);
        ui_shader = NewShaderFromFile(GL_VERTEX_SHADER, "shdr/ui.vert");
        post_shader= NewShaderFromFile(GL_FRAGMENT_SHADER, "shdr/post0.frag");
        az_frag = NewShaderFromFile(GL_FRAGMENT_SHADER, "shdr/cel_blue.frag");
        st_frag = NewShaderFromFile(GL_FRAGMENT_SHADER, "shdr/stars.frag");
    }
    else {
        UpdateShaderFromObjs(lzr_vert_shader, 4, lZr_objs);
        UpdateShaderFromObjs(az_vert_shader, 5, AZ_objs);
        UpdateShaderFromFile(ui_shader, GL_VERTEX_SHADER, "shdr/ui.vert");
        UpdateShaderFromFile(post_shader, GL_FRAGMENT_SHADER, "shdr/post0.frag");
        UpdateShaderFromFile(az_frag, GL_FRAGMENT_SHADER, "shdr/cel_blue.frag");
        UpdateShaderFromFile(st_frag, GL_FRAGMENT_SHADER, "shdr/stars.frag");
    }
    UpdateShaderFromFile(simpleFragNode(), GL_FRAGMENT_SHADER, "shdr/simple.frag");
}


static void hitKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((key==GLFW_KEY_R) && (action==GLFW_PRESS)) {
        loadShaders();
    }
    if ((key==GLFW_KEY_P) && (action==GLFW_PRESS)) {
        mode = (mode+1)%2;
        
        LightInstance* nwl;
        nwl = OriginFindFirstLight(root, "pass0", 255); if (nwl) {
            nwl->OutFBO = mode?0: frameBuffers[0];
        }
        nwl = OriginFindFirstLight(root, "pass1", 255); if (nwl) {
            nwl->CanRender = mode?0:1;
        }
        
    }
}


// SRC
static GLuint snoise3DTexture() {
    static int init=0;
    static GLuint nt;
    
    int size = 256;
    float width = 23.289;
    
    if (!init) {
        // Noise data
        float* data = malloc(sizeof(float)*size*size*size);
        for (int x=0; x<size; x++)
        for (int y=0; y<size; y++)
        for (int z=0; z<size; z++) {
            int idx = (z*size*size) + (y*size) + x;
            vec3 pos = {width * (float)x/(float)size, width * (float)y/(float)size, width * (float)z/(float)size};
            data[idx] = snoise3(pos);
        }

        // Texture
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1,&nt);
        glBindTexture(GL_TEXTURE_3D,nt);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, size, size, size, 0, GL_RED, GL_FLOAT, data);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_3D, 0);

        free(data);
    }
    return nt;
}

static void setTexture(GLuint texture, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void setRenderBuffer(GLuint rbo, int width, int height) {
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

static void setFBOs(int width, int height) {
    static int init = 0;
    if (!init) {
        init = 1;
    }
    else {
        //glDeleteBuffers(GRAV_COLORBUFFERS, colorBuffers);
        //glDeleteRenderbuffers(GRAV_DEPTHBUFFERS, depthBuffers);
        //glDeleteFramebuffers(GRAV_FRAMEBUFFERS, frameBuffers);
    }

    // Create Buffers
    glGenTextures(GRAV_COLORBUFFERS, colorBuffers);
    glGenRenderbuffers(GRAV_DEPTHBUFFERS, depthBuffers);
    glGenFramebuffers(GRAV_FRAMEBUFFERS, frameBuffers);

    // Init
    for (int i=0; i<GRAV_COLORBUFFERS; i++)
        setTexture(colorBuffers[i], width, height);
    
    for (int i=0; i<GRAV_DEPTHBUFFERS; i++)
        setRenderBuffer(depthBuffers[i], width, height);


    // Bindings
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[0], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffers[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Reset Usage
    LightInstance* nwl;
    nwl = OriginFindFirstLight(root, "pass0", 255); if (nwl) {
        nwl->OutFBO = mode?0: frameBuffers[0];
    }
    nwl = OriginFindFirstLight(root, "pass1", 255); if (nwl) {
        LightSetTexUniform(nwl, "t_Pass0", colorBuffers[0], GL_TEXTURE_2D);
    }
}

static void resize(GLFWwindow* window, int width, int height) {
    float asp = (float)width/(float)height;
    mat4x4_perspective(
        self->ProjectionMatrix,
        50*TO_RAD,
        asp,
        NEAR_PROJECTION, FAR_PROJECTION
    );
    setFBOs(width, height);
}

static void step() {
    glClearColor(3./255.,4./255.,12./255.,0.1);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void buildFlux(GLFWwindow* window) {
    // Init
    self = &GRAV_VERSE;
    root = &self->Root;
    mat4x4_identity(root->CFrame);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    resize(window, width, height);
    loadShaders();

    // Connection
    SignalConnect(&KeyInput, hitKey);
    SignalConnect(&PreRender, step);
    SignalConnect(&FrameBufferSize, resize);

    // Scene
    PartInstance* nwp;
    LightInstance* nwl;
    //OriginInstance* nwo;
    OriginInstance* form = root;


    
    // Light
    GLuint postLightGroup = getUniqueLightGroup();
    nwl = pointLight(0.); {
        nwl->Name="pass0";
        nwl->BlendSrc = GL_ONE;
        nwl->BlendDst = GL_ZERO;
        LightSetTexUniform(nwl, "t_SNoise", snoise3DTexture(), GL_TEXTURE_3D);
        LightSetPosition(nwl, (vec3){1000,800,500});
        LightSetParent(nwl, form);
        nwl->OutFBO = mode?0: frameBuffers[0];
        nwl->ZIndex = 0;
    }
    nwl = pointLight(0); {
        nwl->Name="pass1";
        LightSetColor(nwl, (color4){255, 253, 250, 255});
        LightSetPosition(nwl, (vec3){5000,5000,3500});
        LightSetParent(nwl, form);
        LightSetTexUniform(nwl, "t_Pass0", colorBuffers[0], GL_TEXTURE_2D);
        nwl->Noding = ENUM_NODE0_OVERSHADE;
        nwl->OverShadeNode = post_shader;
        nwl->LightGroup = postLightGroup;
        nwl->BlendDst = GL_ZERO;
        nwl->BlendSrc = GL_ONE;
        nwl->ZIndex = 10;
        nwl->OutFBO = 0;
    }

    // Post-process frame
    nwp = canvas(1.); {
        PartSetPosition(nwp, (vec3){0.,0.,-1.});
        nwp->Pivot[0] = PIV_LFT[0];
        nwp->Pivot[1] = PIV_BOT[1];
        nwp->VertNodes[ENUM_NODE0] = ui_shader;
        PartRemoveLightGroup(nwp, GLOBAL_LIGHTGROUP);
        PartAddLightGroup(nwp, postLightGroup);
        PartSetParent(nwp, form);
    }
    
    // Azteroids
    for (int i =0; i<400; i++) {
        float s1 = frand(2,6); float s2=s1+4;
        float s3 = frand(.2,5); float s4=s3+.5;
        vec3 pos = { frand(-250,250), frand(-250,250), frand(-250,250) };
        vec3 size0 = {frand(s1,s2), frand(s1,s2), frand(s1,s2)}; /*2-10*/
        vec3 size1 = {frand(s3,s4), frand(s3,s4), frand(s3,s4)}; /*.5-3*/;
        nwp = icoSphere(1.); {
            nwp->VertNodes[ENUM_NODE0] = az_vert_shader;
            nwp->FragNodes[ENUM_NODE0] = az_frag;
            nwp->DrawMode = GL_PATCHES;
            PartSetPosition(nwp, pos);
            PartSetSize(nwp, size0);
            PartSetNodeAttribByName(nwp, ENUM_NODE0, "i_Size1", size1);
            PartSetNodeAttribByName(nwp, ENUM_NODE0, "i_Pos", pos);
            PartSetParent(nwp, form);
        }
    }
    
    
    nwp = uvSphere(MED_DETAIL, MED_DETAIL, 5.); {
        PartSetSize(nwp, (vec3){4.5,9.,6.});
        nwp->DrawMode = GL_LINE_STRIP;
        nwp->CanRender=0;
        PartSetParent(nwp, form);
    }

    // Stars
    nwp = icoSphere(1.); {
        PartSetSize(nwp, (vec3){-800,800,800});
        nwp->FragNodes[ENUM_NODE0] = st_frag;
        PartSetParent(nwp, form);
    }
}

static void clearFlux(GLFWwindow* window) {
    DestroyOriginInstance(root);
}