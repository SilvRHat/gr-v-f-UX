// Gr@v:f/UX
// Gavin:Zimmerman

// Camera Controller Source
#include "camera.h"
#include "grav-flx.h"



// STATE GLOBALS
static CameraInstance Camera;


// SOURCE
void cameraInit(GLFWwindow* window) {
    Camera.Focus = 18;
    Camera.az = 30*TO_RAD;
    Camera.ph = -30*TO_RAD;
    Camera.Name = "CurrentCamera";
    Camera.ViewVerse = &GRAV_VERSE;

    mat4x4_from_pos(Camera.CFrame, (vec3) {0, 0, 0});
    cameraStep(window, 0, 0);
}



void cameraStep(GLFWwindow* window, double t, double step) {
    mat4x4 M, ZoomFrame;

    // Character Controls
    vec2 charmove = {
        -glfwGetKey(window, GLFW_KEY_A) + glfwGetKey(window, GLFW_KEY_D),   // Horizontal inputs; left/right
        -glfwGetKey(window, GLFW_KEY_S) + glfwGetKey(window, GLFW_KEY_W)    // Vertical inputs; forward/backward
    };
    if (vec2_length(charmove)>0) {
        // Normalize so that speed is constance
        vec2_norm(charmove, charmove);
        // Apply speed
        vec2_scale(charmove, charmove, CHARACTER_SPEED);
        mat4x4_from_pos(M, (vec3) {charmove[0]*step, 0, -charmove[1]*step});
        mat4x4_mul(Camera.CFrame, Camera.CFrame, M);
    }
    

    // Build Camera CFrame
    // Build Rotation
    mat4x4_identity(M);
    mat4x4_rotate_Y(M, M, Camera.az);
    mat4x4_rotate_X(M, M, Camera.ph);

    // Translate
    vec4_dup(M[3], Camera.CFrame[3]);
    mat4x4_dup(Camera.CFrame, M);

    // Zoom Out
    mat4x4_dup(ZoomFrame, M);
    mat4x4_translate_in_place(ZoomFrame, (vec3){0, 0, Camera.Focus});
    mat4x4_dup(Camera.ViewCFrame, ZoomFrame);

    // Set Camera owned by Render Pipeline
    setRenderCameraCFV(Camera.ViewCFrame, Camera.ViewVerse);
}


void cameraCursorInput(GLFWwindow* window, double xpos, double ypos) {
    static double xpos0, ypos0;
    double dx, dy;
    static int init=0;
    if (!init) { init=1;
        glfwGetCursorPos(window, &xpos0, &ypos0);
    }

    dx = xpos - xpos0;
    dy = ypos - ypos0;
    // Developer controls 
    if (
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) || 
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE)
    ){
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            // Translate
            mat4x4 m;
            mat4x4_from_pos(m, (vec3) {-dx * Camera.Focus / 500.0, dy * Camera.Focus / 500.0, 0});
            mat4x4_mul(Camera.CFrame, Camera.CFrame, m);
        
        } else {
            // Rotate
            Camera.az -= dx * TO_RAD / 2.0;
            Camera.ph -= dy * TO_RAD / 2.0;
            Camera.ph = clamp(Camera.ph, -MAX_INCLINATION_ANG*TO_RAD, MAX_INCLINATION_ANG*TO_RAD);
            Camera.az = fmod(Camera.az, 360.0*TO_RAD); Camera.ph = fmod(Camera.ph, 360.0*TO_RAD);
        }
    }

    xpos0 = xpos;
    ypos0 = ypos;
}


void cameraScrollInput(GLFWwindow* window, double xoffset, double yoffset) {
    Camera.Focus = clamp(Camera.Focus - (yoffset/2.0), MIN_FOCUS, MAX_FOCUS);
}