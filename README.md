# RayTracer
Reverse Ray Tracer for rendering graphics using CPU or GPU via OpenCL.


##IMPORTANT

- YOU MUST UNZIP THE LIB FILE IN "dependencies\libs\x64\wxWidgets\wxmsw31ud_core.zip" to be able to compile the GameLauncher.

## TODO:
- Rename "Object" struct to "Mesh"

- Make sure there is CPU-compute support and remove depricated "DebugRayTracer"

- Fix the window resize-functionality, currently crashing when using advancedRender

- Consider removing last run of the treeTraverser kernel with a modified version of the colorToPixel kernel.

- Consider "chaining" OpenCL kernels to free up some CPU-time and to help making other calculations on CPU without it having to launch kernel steps during rendering of image.

- Look into what needs to be changed to support not only Windows but also Linux and Mac.
  - OpenGL - CL interop
  - get working directory for program.build()
  - system("pause")
  - etc.
  
- Look into improving how the pixel color are computed. Good information at this site: http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_2_Phong_Mirrors_and_Shadows.shtml

- Optimize and clean up kernels
  - Add read_only,write_only,const and similar properties
  - Minimize reads and writes to global memory, for example by making a privat copy when accessing multiple times
  - Look into the possibility of optimizing traceBruteForceColor()
    - Look into if possible and beneficial to: Save the objects whose AABB were hit and only run intersectsTriangle() on the closest ones in some way.

- Optimize and clean up host code
  - When possible/beneficial change flags for cl::Buffers to as conservative ones as possible such as CL_MEM_READ_ONLY or CL_MEM_WRITE_ONLY.
  - Remove unnecessary reads/writes to cl:Buffers

  

## WIP

-GameLauncher: Creating functional gamelauncher.

-GameLauncher: Successfully save all the required settings and load them correctly.