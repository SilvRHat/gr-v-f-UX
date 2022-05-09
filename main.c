// Gr@v:f/UX
// Gavin:Zimmerman

// Main / Panel Source
// DEPENDENCIES
#include "main.h"

GLboolean Update = 0;
SignalInstance  KeyInput = NewSignal,
                MouseButtonInput = NewSignal,
                ScrollInput = NewSignal,
                CursorInput = NewSignal,
                FrameBufferSize = NewSignal;


// SOURCE
// Handlers
static void _KeyInputHandler(void* func, va_list args) {
    GLFWwindow* window = va_arg(args, GLFWwindow*);
    int key = va_arg(args, int);
    int scancode = va_arg(args, int);
    int action = va_arg(args, int);
    int mods = va_arg(args, int);
    ((void(*)(GLFWwindow*, int, int, int, int))(func))(window, key, scancode, action, mods);
}
static void _MouseButtonInputHandler(void* func, va_list args) {
    GLFWwindow* window = va_arg(args, GLFWwindow*);
    int button = va_arg(args, int);
    int action = va_arg(args, int);
    int mods = va_arg(args, int);
    ((void(*)(GLFWwindow*, int, int, int))(func))(window, button, action, mods);
}
static void _ScrollInputHandler(void* func, va_list args) {
    GLFWwindow* window = va_arg(args, GLFWwindow*);
    double xoffset = va_arg(args, double);
    double yoffset = va_arg(args, double);
    ((void(*)(GLFWwindow*, double, double))(func))(window, xoffset, yoffset);
}
static void _CursorInputHandler(void* func, va_list args) {
    GLFWwindow* window = va_arg(args, GLFWwindow*);
    double xpos = va_arg(args, double);
    double ypos = va_arg(args, double);
    ((void(*)(GLFWwindow*, double, double))(func))(window, xpos, ypos);
}
static void _FrameBufferSizeHandler(void* func, va_list args) {
    GLFWwindow* window = va_arg(args, GLFWwindow*);
    int _w = va_arg(args, int);
    int _h = va_arg(args, int);
    ((void(*)(GLFWwindow*, int, int))(func))(window, _w, _h);
}


// Input Callbacks
static void KeyCallbackHandler(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Call Static Handlers
    {
        if ((key==GLFW_KEY_ESCAPE) && (action==GLFW_PRESS))
            glfwSetWindowShouldClose(window, 1);
        if ((key==GLFW_KEY_B) && (action==GLFW_PRESS)) {
            printf("Rebuilding\n");
            glfwSetWindowShouldClose(window, 1);
            Update = 1;
            if (system("make")<0) Error("  Make recieved Err code");
        }
    }
    // Signal Dynamic Handlers
    SignalFire(&KeyInput, window, key, scancode, action, mods);
}


static void MouseCallbackHandler(GLFWwindow* window, int button, int action, int mods) {
    SignalFire(&MouseButtonInput, window, button, action, mods);
}


static void ScrollCallbackHandler(GLFWwindow* window, double xoffset, double yoffset) {
    // Call Static Handlers
    cameraScrollInput(window, xoffset, yoffset);
    // Signal Dynamic Handlers
    SignalFire(&ScrollInput, window, xoffset, yoffset);
}


static void CursorCallbackHandler(GLFWwindow* window, double xpos, double ypos) {
    // Call Static Handlers
    cameraCursorInput(window, xpos, ypos);
    // Signal Dynamic Handlers
    SignalFire(&CursorInput, window, xpos, ypos);
}

static void SizeCallback(GLFWwindow* window, int _w, int _h) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // Call Static Handlers
    // Signal Dynamic Handlers
    SignalFire(&FrameBufferSize, window, width, height);
}

void gameLoop(GLFWwindow* window) {
    ErrCheck("ERRCHK IN: gameLoop()");
    static double t0 = 0;
    double t = glfwGetTime();
    double dt = t - t0;
    t0 = t;

    // Camera
    cameraStep(window, t, dt);

    // Render Scene
    if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderStep(window, t, dt);
    }
    
    
    // Swap Buffers
    glfwSwapBuffers(window);

    // Input
    glfwPollEvents();
    ErrCheck("ERRCHK RET: gameLoop()");
}



void glfwErrorHandler(int code, const char* desc) {
    printf("ERROR %d: %s\n", code, (desc!=NULL)? desc: "GLFW Encountered an issue");
}



// Main
int main(int argc, char* argv[]) {
    GLFWwindow* window;
    int windowX = 1080, windowY = 720;


    // Init
#ifdef USEGLEW
    if (glewInit()!=GLEW_OK) exit(-1);
#endif
    if (!glfwInit()) {
        printf("    Error 0: Could not initialize glfw3\n");
        exit(EXIT_FAILURE);
    }
    glfwSetErrorCallback(glfwErrorHandler);

    // Window creation
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    window = glfwCreateWindow(windowX, windowY, "Gr@v:f/UX // Gavin Zimmerman", NULL, NULL);
    if (!window) { 
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    // Initialize Signals
    SignalInit(&KeyInput, _KeyInputHandler);
    SignalInit(&MouseButtonInput, _MouseButtonInputHandler);
    SignalInit(&ScrollInput, _ScrollInputHandler);
    SignalInit(&CursorInput, _CursorInputHandler);
    SignalInit(&FrameBufferSize, _FrameBufferSizeHandler);


    // Input Callbacks
    glfwSetKeyCallback(window, KeyCallbackHandler);
    glfwSetMouseButtonCallback(window, MouseCallbackHandler);
    glfwSetScrollCallback(window, ScrollCallbackHandler);
    glfwSetCursorPosCallback(window, CursorCallbackHandler);
    glfwSetFramebufferSizeCallback(window, SizeCallback);
    glfwSetWindowSizeCallback(window, SizeCallback);

    // Controller Systems
    cameraInit(window); 
    renderInit(window);
    SignalFire(&FrameBufferSize, window, windowX, windowY);

    // OpenGL
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Game Loop 
    GRAV_VERSE.Build(window);
    while(!glfwWindowShouldClose(window)) {
        gameLoop(window);
    }
    GRAV_VERSE.Clean(window);

    // Termination
    SignalDestroy(&KeyInput);
    SignalDestroy(&MouseButtonInput);
    SignalDestroy(&ScrollInput);
    SignalDestroy(&CursorInput);
    SignalDestroy(&FrameBufferSize);
    
    renderExit(window);
    glfwDestroyWindow(window);
    glfwTerminate();
    if (Update) {
        execv(argv[0], argv);
    }

    return EXIT_SUCCESS;
}