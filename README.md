# Final Progress Review - Gr@v:f/UX
## Gavin Zimmerman



### Controls
 ESC                            Escape
 WASD                           Move camera
 Mouse Right Button             Rotate camera
 Mouse Right Button + shift     Dolly camera
 Mouse Scroll                   Zoom in / out
 B                              Build scene [shortcut dev tool]
 R                              Reload Shaders [shortcut dev tool]
 P                              Toggle post-processing

### Compiling and Running
 Compiling: make
 Running:  ./final


### About
 Asteroids

### Files
 camera.c/.h      Camera Logic
 grav-flx.c/.h    Main scene
 main.c/.h        Top level program, responsible for setting up input and window
 noise.c/.h       Source code to develop 3D noise texture

 Shaders
   AZ                Azteroid vertex shader pipeline
   cel_blue.frag     Azteroid cel shading
   post0.frag        Post processing
   stars.frag        Star background     

 Multiverse Rendering Engine
   MVRE.h           Rendering Frameowrk logic
   graphics.h       Graphics Dependencies
   linmath.h        Linear Algebra utilities
   objects.c/.h     Object Logic; Instancing
   render.c/.h      Render logic
   signal.c/.h      Programming pattern for event-driven programming
   utils.c/utils.h  Utility function


Key Accomplishments
 The key showcase in this project is deeloping a space scene completely procedurally, without meshes or textures onhand. The geometry makes use of several concepts we learned
 this year including: dynamic tesselation based on distance, noise functions, stored textures (3D texture stores simplex), and procedural textures. The majority of interesting code for this
 process is under 'shdr/AZ'. These are all then created differently by supplying different inputs to each instanced object, such that they can be form a random shape. Getting this to run fairly
 efficiently was one of my main obstacles and I utilize various optimizations we learned in class among dynamic tesselation, backface culling, camera culling, texture-lookups for noise, and smaller loops.
 
 Another item I've worked on is post-processing, which aims to layer noise and a red hue ontop the first pass- this emulates noise found in traditional cel animation
 by lighting. 
