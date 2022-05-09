// Multi_verse: RE(Rendering Engine)
// Gavin Zimmerman

// RENDERING PIPELINE
#include "render.h"


// EVENTS
SignalInstance PreRender, PostRender;


// EARLY STATIC DECLARATIONS
static void buildUIVerse(GLFWwindow* window);
static void cleanUIVerse(GLFWwindow* window);



// STATE GLOBALS
// Uniforms
    // @brief Holds information on uniforms, typically values are changed throughout the pipeline and context
static UniformData* Uniforms; 

// RenderStack
    // @brief Holds information for rendering logic. It includes an array for parts and lights requested
    // to be drawn. It also contains information on parts including OpenGL state pre-requeistes (VAO, 
    // Program nodes, etc.) and instanceable attribute data
static RenderData* RenderStack;

// UI_VERSE
    // @brief A special verse instance for displaying 2D elements in front of user camera
static VerseInstance UI_VERSE = {
    .Name = "UserInterfaceVerse",
    .ClassName = "Verse0xFFFF",

    .Build = buildUIVerse,
    .Clean = cleanUIVerse,

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


// RenderCamera
    // @brief Retains camera state
static CameraData RenderCamera = {
    .CameraVerse = NULL,
};


// Misc State
static int drawcalls = 0;




// SOURCE
static void buildUIVerse(GLFWwindow* window) {
    mat4x4_identity(UI_VERSE.Root.CFrame);
    // Projection Matrix
    mat4x4_ortho(UI_VERSE.ProjectionMatrix, 0, 1, 0, 1, NEAR_PROJECTION, FAR_PROJECTION);
    LightInstance* occlusion = globalOcclusion();
    LightSetParent(occlusion, &UI_VERSE.Root);
}


static void cleanUIVerse(GLFWwindow* window) {
    // Remove components
    DestroyOriginInstance(&UI_VERSE.Root);
}








static void getOrderedLightsFromRender(RenderInstanceData OrderedLights[]) {
    int n = RenderStack->NumLights;
    GLuint sortval[n][3];
    int sortidx[n];

    // Initialize sorting data from lights into uint vector array
    for (int i=0; i<n; i++) {
        LightInstance* l = RenderStack->Lights[i].Instance;
        sortval[i][0] = l->ZIndex;
        sortval[i][1] = l->LightGroup;
        sortval[i][2] = l->Noding % MAX_NODES;
        sortidx[i] = i;
    }

    // Sort that
    uiVecQuickSort(n, 3, sortval, sortidx);

    // Pull sorted data
    for (int dst = 0; dst<n; dst++) {
        int src = sortidx[dst];
        memcpy(&OrderedLights[dst], &RenderStack->Lights[src], sizeof(RenderInstanceData));
    }
}


static void loadSortedAttribDataFromRender(GLuint Noding, GLuint LightGroup) {
    int n = RenderStack->NumParts;
    GLuint sortval[n][DRAW_INFO_ROW];
    int sortidx[n];
    
    
    // Pull index of which node type to use. Modulo converts OPT_OVERSHADE to OPT; see objects.h
    int nodeI = Noding % MAX_NODES;
    int parts = 0;

    // Init sorting data for parts into vector array
    for (int i=0; i<n; i++) {
        PartInstance* p = RenderStack->Parts[i].Instance;

        // Validate
        int IsInLightGroup = 0;
        for (int j=0; (!IsInLightGroup) && (j<MAX_LIGHTGROUPS); j++)
            if (p->LightGroups[j]==LightGroup) IsInLightGroup = 1;
        
        if (!IsInLightGroup) continue;

        // Pull sortable data (data that will cause different gldraw calls/ state changes)
        sortval[parts][0] = p->VertNodes[nodeI];    // Relevant Shaders based on light settings
        sortval[parts][1] = p->FragNodes[nodeI];
        sortval[parts][2] = p->Vao;
        sortval[parts][3] = p->Texture;
        sortval[parts][4] = p->DrawMode;
        sortval[parts][5] = p->Vertices;
        sortidx[parts] = i;
        parts++;
    }
    // Sort that
    uiVecQuickSort(parts, DRAW_INFO_ROW, sortval, sortidx);


    // Pull from sorting hat into render-ready data
    for (int dst=0; dst<parts; dst++) {
        int src = sortidx[dst];
        mat4x4 ModelViewMat, NormalMat;
        mat4x4_dup(ModelViewMat, RenderStack->Parts[src].ViewMat);
        mat4x4_invert(NormalMat, ModelViewMat);
        mat4x4_transpose(NormalMat, NormalMat);

        // Drawing Info
        PartInstance* p = RenderStack->Parts[src].Instance;
        RenderStack->DrawData[dst][0] = p->VertNodes[nodeI];    // Relevant Shaders based on light settings
        RenderStack->DrawData[dst][1] = p->FragNodes[nodeI];
        RenderStack->DrawData[dst][2] = p->Vao;
        RenderStack->DrawData[dst][3] = p->Texture;
        RenderStack->DrawData[dst][4] = p->DrawMode;
        RenderStack->DrawData[dst][5] = p->Vertices;
        

        // Populate attribute data
        float attribs[INST_ATTRIB_ROW] = {0};
        for (int i=0; i<4; i++)
            mat4x4_col(attribs+(i*4), ModelViewMat, i);
        for (int i=0; i<3; i++)
            vec4_dup(attribs+(i*3 +16), NormalMat[i]);
        for (int i=0; i<4; i++)
            mat4x4_col(attribs+(i*4 +32), p->NodeAttrs[nodeI], i);
        vec4_dup(attribs+25, p->Color);
        vec3_dup(attribs+29, p->Size);

        memcpy(RenderStack->AttribData[dst], attribs, sizeof(attribs));
    }
    RenderStack->NumDrawing = parts;
}


static void setNodeUniforms(GLuint node) {
    int loc;
    // ViewMat
    loc = glGetUniformLocation(node, UNI_NAME_VIEWMAT);
    glProgramUniformMatrix4fv(node, loc, 1, GL_FALSE, (const GLfloat*) Uniforms->ViewMat);
    // ProjectionMat
    loc = glGetUniformLocation(node, UNI_NAME_PROJECTIONMAT);
    glProgramUniformMatrix4fv(node, loc, 1, GL_FALSE, (const GLfloat*) Uniforms->ProjectionMat);
    // ClipPlane
    loc = glGetUniformLocation(node, UNI_NAME_CLIPPLANE);
    glProgramUniform4fv(node, loc, 1, (const GLfloat *) Uniforms->ClipPlane);
    // Resolution
    loc = glGetUniformLocation(node, UNI_NAME_RESOLUTION);
    glProgramUniform2fv(node, loc, 1, Uniforms->Resolution);
    // Mouse
    loc = glGetUniformLocation(node, UNI_NAME_MOUSEPOS);
    glProgramUniform2fv(node, loc, 1, Uniforms->Mouse);
    // Time
    loc = glGetUniformLocation(node, UNI_NAME_TIME);
    glProgramUniform1f(node, loc, Uniforms->Time);
    // Texture
    loc = glGetUniformLocation(node, UNI_NAME_TEXTURE);
    glProgramUniform1i(node, loc, 0);
    // LightViewMat
    LightInstance* l = Uniforms->Light;
    loc = glGetUniformLocation(node, UNI_NAME_LIGHTVIEWMAT);
    glProgramUniformMatrix4fv(node, loc, 1, GL_FALSE, (const GLfloat*) Uniforms->LightViewMat);
    // LightType
    loc = glGetUniformLocation(node, UNI_NAME_LIGHTENUM);
    glProgramUniform1i(node, loc, l->LightEnum);
    // Light Color
    loc = glGetUniformLocation(node, UNI_NAME_LIGHTCOLOR);
    glProgramUniform4fv(node, loc, 1, l->Color);

    

    // Custom Uniforms 
    // Int types
    for (int j=0; j<MAX_TEX_UNIFORMS; j++) {
        if (l->TexUniforms[j].Name==NULL) continue;
        if (strcmp(l->TexUniforms[j].Name,"")==0) continue;

        loc = glGetUniformLocation(node, l->TexUniforms[j].Name);
        if (loc<0) continue;

        glActiveTexture(GL_TEXTURE1 + j);
        glBindTexture(l->TexUniforms[j].BindTarget, l->TexUniforms[j].Value);
        glProgramUniform1i(node, loc, j+1);
        glActiveTexture(GL_TEXTURE0);
    }
    // Vec types
    for (int j=0; j<MAX_VEC_UNIFORMS; j++) {
        if (l->VecUniforms[j].Name==NULL) continue;
        if (strcmp(l->VecUniforms[j].Name,"")==0) continue;

        loc = glGetUniformLocation(node, l->VecUniforms[j].Name);
        if (loc<0) continue;

        int n = clamp(l->VecUniforms[j].N, 1, 4);
        switch(n) {
            case 1: glProgramUniform1fv(node, loc, 1, l->VecUniforms[j].Value); break;
            case 2: glProgramUniform2fv(node, loc, 1, l->VecUniforms[j].Value); break;
            case 3: glProgramUniform3fv(node, loc, 1, l->VecUniforms[j].Value); break;
            case 4: glProgramUniform4fv(node, loc, 1, l->VecUniforms[j].Value); break;
        }
    }
    // Mat types
    for (int j=0; j<MAX_MAT_UNIFORMS; j++) {
        if (l->MatUniforms[j].Name==NULL) continue;
        if (strcmp(l->MatUniforms[j].Name,"")==0) continue;

        loc =  glGetUniformLocation(node, l->MatUniforms[j].Name);
        if (loc<0) continue;
        
        int n = clamp(l->MatUniforms[j].N, 2, 4);
        int m = clamp(l->MatUniforms[j].M, 2, 4);
        float val[n][m];
        for (int c=0; c<n; c++) for (int r=0; r<m; r++)
            val[c][r] = l->MatUniforms[j].Value[c][r];
        
             if ((n==4) && (m==4)) glProgramUniformMatrix4fv  (node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==4) && (m==3)) glProgramUniformMatrix4x3fv(node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==4) && (m==2)) glProgramUniformMatrix4x2fv(node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==3) && (m==4)) glProgramUniformMatrix3x4fv(node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==3) && (m==3)) glProgramUniformMatrix3fv  (node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==3) && (m==2)) glProgramUniformMatrix3x2fv(node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==2) && (m==4)) glProgramUniformMatrix2x4fv(node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==2) && (m==3)) glProgramUniformMatrix2x3fv(node, loc, 1, GL_FALSE, (const GLfloat*) val);
        else if ((n==2) && (m==2)) glProgramUniformMatrix2fv  (node, loc, 1, GL_FALSE, (const GLfloat*) val);
    }
    ErrCheck("ERRCHK RET: setNodeUniforms()");
}


static void lightRenderPass(LightInstance* l) {
    static int init = 0;
    static GLuint ShaderPipe;
    if (!init) {
        init=1;
        glGenProgramPipelines(1, &ShaderPipe);
    }
    
    int count = 0;
    GLuint vertnode = 0,
           fragnode = 0,
           vao = 0,
           texture = 0,
           drawmode = 0, 
           verts = 0;
    
    glUseProgram(0);
    glBindProgramPipeline(ShaderPipe);
    for (int i=0; i<RenderStack->NumDrawing; i++) {
        GLuint active_text = (l->UseTexture)? RenderStack->DrawData[i][3] : 0;
        GLuint active_frag = (l->Noding >= ENUM_NODE0_OVERSHADE)? l->OverShadeNode : RenderStack->DrawData[i][1];
        // Evaluate Changes
        GLboolean IsDiffFragNode = active_frag != fragnode;
        GLboolean IsDiffTexture  = active_text != texture;
        GLboolean IsDiffVertNode = RenderStack->DrawData[i][0] != vertnode;
        GLboolean IsDiffVao      = RenderStack->DrawData[i][2] != vao; 
        GLboolean IsDiffDrawmode = RenderStack->DrawData[i][4] != drawmode;
        GLboolean IsDiffVertices = RenderStack->DrawData[i][5] != verts;

        // Any change in state requires new gldraw call / attribute writing
        if (IsDiffVertNode || IsDiffFragNode || IsDiffVao || IsDiffTexture || 
            IsDiffDrawmode || IsDiffVertices || (count >= INST_ATTRIB_COL)
        ) {
            // Draw last set of data
            if ((count>0) && (vertnode>0) && (fragnode>0)) {
                glPatchParameteri(GL_PATCH_VERTICES, 3);
                glDrawArraysInstanced(drawmode, 0, verts, count);
                drawcalls++;
            }
            count=0;
            
            // Update attribs
            int size = sizeof(RenderStack->AttribData[0]) * min((MAX_BATCH_PART_INSTANCES-i), INST_ATTRIB_COL);
            glBindBuffer(GL_ARRAY_BUFFER, getInstancedAttribVBO());
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, RenderStack->AttribData[i]);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        if (IsDiffVao) {
            vao = RenderStack->DrawData[i][2];
            glBindVertexArray(vao);
        }
        if (IsDiffTexture) {
            texture = active_text;
            Uniforms->Texture = texture;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
        }
        if (IsDiffDrawmode || IsDiffVertices) {
            drawmode = RenderStack->DrawData[i][4];
            verts = RenderStack->DrawData[i][5];
        }
        if (IsDiffVertNode || IsDiffFragNode) {
            vertnode = RenderStack->DrawData[i][0];
            fragnode = active_frag;
            
            
            glUseProgramStages(
                ShaderPipe, 
                GL_VERTEX_SHADER_BIT | GL_GEOMETRY_SHADER_BIT | 
                GL_TESS_CONTROL_SHADER_BIT | GL_TESS_EVALUATION_SHADER_BIT, 
                vertnode
            );
            glUseProgramStages(ShaderPipe, GL_FRAGMENT_SHADER_BIT, fragnode);
            
            // Set uniforms
            GLuint nodes[2] = {vertnode, fragnode};
            for (int i=0; i<2; i++) {
                GLuint node = nodes[i];
                if (node>0) setNodeUniforms(node);
            }
        }
        count++;
    }
    if ((count>0) && (vertnode>0) && (fragnode>0)) {
        glDrawArraysInstanced(drawmode, 0, verts, count);
        drawcalls++;
    }

    ErrCheck("ERRCHK RET: lightRenderPass()");
}


static void flushRender() {
    RenderInstanceData OrderedLights[RenderStack->NumLights];

    // Perform forward-rendering with 1 pass per light
    glEnable(GL_BLEND);
    {
        GLuint lastLG = 0;
        GLuint lastND = 0;
        
        // Order lights by (noding, light group) which affect how parts are sorted prior to drawing
        getOrderedLightsFromRender(OrderedLights);

        // Iterate over each light and rendering scene changing shader inputs
        for (int i=0; i<RenderStack->NumLights; i++) {
            LightInstance* l = OrderedLights[i].Instance;

            if ((lastLG != l->LightGroup) || (lastND != l->Noding)) {
                lastND = l->Noding;
                lastLG = l->LightGroup;
                loadSortedAttribDataFromRender(lastND, lastLG);
            }

            // Set basic blending
            {
                glEnable(GL_BLEND);
                glBlendFunc(l->BlendSrc, l->BlendDst);
            }

            // Set new uniforms
            Uniforms->Light = l;
            mat4x4_dup(Uniforms->LightViewMat, OrderedLights[i].ViewMat);

            // Bind Frame Buffer
            glBindFramebuffer(GL_FRAMEBUFFER, l->OutFBO);

            // Run pass
            SignalFire(&l->PrePass);
            lightRenderPass(l);
            SignalFire(&l->PostPass);
        }
    }
    RenderStack->NumLights = 0;
    RenderStack->NumParts = 0;

    ErrCheck("ERRCHK RET: flushRender()");
}


static void pushOriginToRender(OriginInstance* o, mat4x4 OriginViewMatrix) {
    RenderInstanceData PushInst;
    // Parts
    for (int i=0; i<MAX_PART_INSTANCES; i++) {
        PartInstance* ch = o->Parts[i];
        if (ch==NULL) break;
        if (!ch->CanRender) continue;
        if (RenderStack->NumParts==MAX_BATCH_PART_INSTANCES) Error("Hit maximum part rendering stack limit");
        
        // Pivot Offset
        mat4x4 pom;
        vec4 pov = {0,0,0,1}; 
        vec3_comp_mul(pov, ch->Pivot, ch->Size);
        vec3_scale(pov, pov, -1);
        mat4x4_from_pos(pom, pov);

        PushInst.Instance = ch;
        mat4x4_mul(PushInst.ViewMat, OriginViewMatrix, ch->CFrame);
        mat4x4_mul(PushInst.ViewMat, PushInst.ViewMat, pom);
        
        memcpy(&RenderStack->Parts[RenderStack->NumParts], &PushInst, sizeof(RenderInstanceData));
        RenderStack->NumParts++;
    }
    // Lights
    for (int i=0; i<MAX_LIGHT_INSTANCES; i++) {
        LightInstance* ch = o->Lights[i];
        if (ch==NULL) break;
        if (!ch->CanRender) continue;
        if (RenderStack->NumLights==MAX_BATCH_LIGHT_INSTANCES) Error("Hit maximum light rendering stack limit");

        PushInst.Instance = ch;
        mat4x4_mul(PushInst.ViewMat, OriginViewMatrix, ch->CFrame);

        memcpy(&RenderStack->Lights[RenderStack->NumLights], &PushInst, sizeof(RenderInstanceData));
        RenderStack->NumLights++;
    }
    // Origins
    for (int i=0; i<MAX_ORIGIN_INSTANCES; i++) {
        OriginInstance* ch = o->Origins[i];
        if (ch==NULL) break;
        if (!ch->CanRender) continue;
        
        mat4x4 ChildViewMat;
        mat4x4_mul(ChildViewMat, OriginViewMatrix, ch->CFrame);
        pushOriginToRender(ch, ChildViewMat);
    }
}


// renderStep
    // @brief Steps through a single frame and renders scene 
    // @param window The glfw context window
    // @param time Running time
    // @param step Time passed since previous step
void renderStep(GLFWwindow* window, float time, float step) {
    // Get extra context info
    int width, height;
    double cursorX, cursorY;
    glfwGetFramebufferSize(window, &width, &height);
    glfwGetCursorPos(window, &cursorX, &cursorY);
    drawcalls=0;

    SignalFire(&PreRender);

    // Setup a stack on verses to render
    RenderJumpData VerseStack[MAX_RENDER_VERSES];
    for (int i=0; i<MAX_RENDER_VERSES; i++) {
        VerseStack[i].j = NULL;
        VerseStack[i].JumpTo = NULL;
        vec3_dup(VerseStack[i].BoundsMin, (vec3){-1,-1,-1});
        vec3_dup(VerseStack[i].BoundsMax, (vec3){ 1, 1, 1});
        
        VerseStack[i].TravelDepth = 1;
        VerseStack[i].Travel[0] = i;
    };
    
    // First initialize starting verses to branch out from
    {    
        // User interface
        mat4x4 UI;
        mat4x4_from_pos(UI, (vec3) {0,0,-1});
        vec4_dup(UI[2], (vec4){0,0,-1,0});
        mat4x4_invert(VerseStack[0].OffsetTo, UI);
        VerseStack[0].JumpTo = &UI_VERSE;

        // Current scene
        mat4x4_invert(VerseStack[1].OffsetTo, RenderCamera.CameraCFrame);
        VerseStack[1].JumpTo = RenderCamera.CameraVerse; 
    }


    // Find neighboring verses
    // Look into each connected verses for more connected verses that are visible to camera
    int stackpos = 2;
    for (int i=0; (i<MAX_RENDER_VERSES) && (stackpos<MAX_RENDER_VERSES); i++) {
        // From [Labeled A]; To [Labeled B]
        RenderJumpData* JDataA = &VerseStack[i];
        RenderJumpData* JDataB = NULL;
        VerseInstance* vIn = JDataA->JumpTo;
        if (!vIn) break;

        // Check each jump in current scene
        for (int j=0; j<MAX_JUMP_INSTANCES; j++) {
            JumpInstance* jmp = vIn->Jumps[j];
            if (jmp==NULL) break;

            // Try each direction of jump
            for (int k=0; k<2; k++) {
                // From [Labeled A]; To [Labeled B]
                VerseInstance* vA = (!k)? jmp->Verse0 : jmp->Verse1;
                VerseInstance* vB = (!k)? jmp->Verse1 : jmp->Verse0;

                // Check if valid
                if (vIn!=vA) continue; // Invalid direction
                if (!(JDataA->TravelDepth < MAX_RENDER_JUMP_DEPTH)) continue;   // Exceeds limit
                if ((JDataA->j == jmp) && (JDataA->Direction!=k)) continue;     // Prevent Jumpback Phenomenon
                if (!((!k)?jmp->RenderIn0:jmp->RenderIn1)) continue;            // Jump doesn't allow this direction
                if (stackpos>=MAX_RENDER_VERSES) break; // Cannot exceed total verse render stack

                // Check if visible
                mat4x4 JVM, Clip; // JumpViewMat (To portal center), ClipMat (To Clip space)
                mat4x4_mul(JVM, JDataA->OffsetTo, (!k)? jmp->CFrame0:jmp->CFrame1);

                // Check Bounding box
                // Since arbitrary geometry is valid for a portal, we test a bounding box of the portal geometry
                // This saves massive performance by culling entire scenes if not in visible view (via recursive NDC bound setting)
                vec3 bmin={ 1, 1, 1}, // Bounding box minimum corner (NDC)
                     bmax={-1,-1,+1}; // Bounding box maximum corner (NDC)
                
                mat4x4_mul(Clip, vA->ProjectionMatrix, JVM);
                mat4x4_mul(Clip, Clip, jmp->Root.BoundingBoxCFrame);
                for (int x=0; x<2; x++) 
                for (int y=0; y<2; y++)
                for (int z=0; z<2; z++) {
                    // Get 1 of 8 corners on bounding box
                    vec4 off = {.5, .5, .5, 1};
                    vec3_comp_mul(off, off, (vec3){x?-1:1, y?-1:1, z?-1:1});
                    vec3_comp_mul(off, off, jmp->Root.BoundingBoxSize);

                    // Find NDC coordinates of corner
                    mat4x4 M;
                    mat4x4_from_pos(M, off);            // Get offset position in matrix
                    mat4x4_mul(M, Clip, M);             // Translate to clip space     
                    vec4_scale(off, M[3], 1/M[3][3]);   // Translate to Position Vector in NDC space

                    // Bounds update
                    for (int s=0; s<3; s++) {
                        bmin[s] = min(bmin[s], off[s]);
                        bmax[s] = max(bmax[s], off[s]);   
                    }
                }

                
                // Bounds check
                int outside=0;
                
                for (int s=0; s<3; s++) {
                    if (
                        (bmax[s]<=JDataA->BoundsMin[s]) || 
                        (bmin[s]>=JDataA->BoundsMax[s])
                    ) outside=1;
                }
                if (outside) continue;
                


                // Passed validation
                JDataB = &(VerseStack[stackpos]);
                
                // Set new jump
                JDataB->j = jmp;
                JDataB->JumpTo = vB;
                for (int s=0; s<3; s++) {
                    JDataB->BoundsMin[s] = max(JDataA->BoundsMin[s], bmin[s]);
                    JDataB->BoundsMax[s] = min(JDataA->BoundsMax[s], bmax[s]);
                }
                
                // Remember where Jump is (from camera (ModelViewSpace))
                mat4x4_dup(JDataB->OffsetFrom, JVM);
                // Update ViewMatrix to apply to parts in verse B
                mat4x4_mul(JDataB->OffsetTo, JDataA->OffsetTo, (!k)? jmp->_ToV1 : jmp->_ToV0);
                // Remember direction used to prevent jumping backwards (ex: A->B->A)
                JDataB->Direction = k;
                // Add destination to travel log
                JDataB->TravelDepth = JDataA->TravelDepth+1;
                for (int s=0; s<JDataA->TravelDepth; s++)
                    JDataB->Travel[s] = JDataA->Travel[s];
                JDataB->Travel[JDataA->TravelDepth] = stackpos;

                stackpos++;
            }
        }
    }


    // Update uniforms
    Uniforms->Time = time;
    vec2_dup(Uniforms->Resolution, (vec2) {width, height});
    vec2_dup(Uniforms->Mouse, (vec2) {cursorX, cursorY});

    // Clear Buffers
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    
    // Go through verses backwards rendering them
    glEnable(GL_STENCIL_TEST);
    for (int i=stackpos-1; i > -1; i--) {
        RenderJumpData *rj = &VerseStack[i];
        VerseInstance* v = rj->JumpTo;
        JumpInstance* j = rj->j;
        
        // Clear Stencil Buffer
        glClearStencil(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_STENCIL_BUFFER_BIT);

        // Reset ClipPlanes
        for (int i=0; i<6; i++) 
            vec4_dup(Uniforms->ClipPlane[i], VEC4_ZERO);

        // Set stencil buffer
        if (j) {
            // The idea here is to render each verse travelled to get to the current scene
            // Each time the stencil value is incremented if it was visible from all previous
            // jumps
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
            
            // Stencil each jump traveled
            glDepthMask(0);
            glColorMask(0,0,0,0);
            for (int s=1; s<rj->TravelDepth; s++) {
                glStencilFunc(GL_EQUAL, (GLint)s-1, 0xFFFFFFFF);

                RenderJumpData* from = &VerseStack[rj->Travel[s]];
                if (from->j) { 
                    // Set uniforms
                    mat4x4_dup(Uniforms->ViewMat, from->OffsetFrom);
                    mat4x4_dup(Uniforms->ProjectionMat, from->JumpTo->ProjectionMatrix);
                    // Draw jump geometry
                    pushOriginToRender(&from->j->Root, from->OffsetFrom);
                    flushRender();
                }
            }
            
            // Set Clipping Planes
        }
        // Set stencil test
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDepthMask(1);
        glColorMask(1,1,1,1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_EQUAL, rj->TravelDepth-1, 0xFFFFFFFF);


        // Render Scenes
        {
            // Set uniforms
            mat4x4_dup(Uniforms->ViewMat, rj->OffsetTo);
            mat4x4_dup(Uniforms->ProjectionMat, v->ProjectionMatrix);

            // Draw scene
            mat4x4 RootViewMat;
            mat4x4_mul(RootViewMat, rj->OffsetTo, v->Root.CFrame);
            pushOriginToRender(&v->Root, RootViewMat);
            flushRender();
        }
        
        // Close Jump; layer depth buffer to prevent closer scenes to leaking into further scenes
        if (j) {
            glColorMask(0,0,0,0);
            // Flush to default
            glDepthMask(1);
            pushOriginToRender(&j->Root, rj->OffsetFrom);
            flushRender();
        }
        
    }
    glDisable(GL_STENCIL_TEST);
}



// renderInit
    // @brief Initializes rendering pipeline resources and opengl state
    // @param window The glfw context window
void renderInit(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    SignalInit(&PreRender, NULL);
    SignalInit(&PostRender, NULL);


    // Enable Technologies // face-culling and user-clipping
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_CLIP_DISTANCE0);
    glDepthFunc(GL_LEQUAL);
    
    // Allocate and init objects
    UI_VERSE.Build(window);
    Uniforms = malloc(sizeof(UniformData));
    RenderStack = malloc(sizeof(RenderData));
    RenderStack->NumLights = RenderStack->NumParts = RenderStack->NumDrawing = 0;
}


// renderExit
    // @brief Gracefully deallocates resources needed for rendering pipeline
    // @param window The glfw context window
void renderExit(GLFWwindow* window) {
    UI_VERSE.Clean(window);
    free(RenderStack);
    free(Uniforms);
}








// setRenderCameraCFV
    // @brief This functions allows the setting of camera data used by the render pipeline.
    // A camera only needs two items of data to render a scene. First the camera position, 
    // and the scene to render camera from.
    // @param CFrame The 4x4 matrix to copy camera positio/rotational data from
    // @param verse The VerseInstance pointer to set
void setRenderCameraCFV(mat4x4 CFrame, VerseInstance* verse) {
    mat4x4_dup(RenderCamera.CameraCFrame, CFrame);
    RenderCamera.CameraVerse = verse;
}

// getRenderCameraCFV
    // @brief This functions allows the retrieval of camera data used by the render pipeline.
    // A camera only needs two items of data to render a scene. First the camera position, 
    // and the scene to render camera from.
    // @param CFrame The 4x4 matrix to copy camera positio/rotational datainto
    // @param verse A pointer to the VerseInstance pointer to set
void getRenderCameraCFV(mat4x4 CFrame, VerseInstance** verse) {
    mat4x4_dup(CFrame, RenderCamera.CameraCFrame);
    *verse = RenderCamera.CameraVerse;
}

// getRenderUIVerse
    // @brief Returns a VerseInstance that is projected in front of the camera.
    // The UI verse uses an orthogonal projection, sized and placed such that objects in this space
    // use NDC coordinates on XY Position values, where Z is used for depth of elements.
    // Users are allowed to connect jumps to this verse which render correctly, which allows
    // an arbitrary number of cameras to exist and be rendered in any crazy way.
    // @return Returns the VerseInstance associated with UI
void getRenderUIVerse(VerseInstance** verse) {
    *verse = &UI_VERSE;
}