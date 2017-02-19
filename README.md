# Title

## Survival game
- Multiplayer?
	- Fix shooting in multiplayer
- Loot: weapons, vehicles
	- Add support for picking up weapons
- Terrain:
	- Loot/zombie zones from .bmp
	- Heightbased texture mix
	- Terrain decoration:
		- Trees, bushes
		- 2D grass things

##IMPORTANT

- YOU MUST UNZIP THE LIB FILE IN "dependencies\libs\x64\wxWidgets\wxmsw31ud_core.zip" to be able to compile the GameLauncher in debug mode.

## TODO:
- Rename "Object" struct to "Mesh"

- Remove depricated "DebugRayTracer"

- Fix the window resize-functionality currently wonky

- Consider removing last run of the treeTraverser kernel with a modified version of the colorToPixel kernel.

- Consider "chaining" OpenCL kernels to free up some CPU-time and to help making other calculations on CPU without it having to launch kernel steps during rendering of image.

- Look into what needs to be changed to support not only Windows but also Linux and Mac.
  - OpenGL - CL interop
  - get working directory for program.build()
  - system("pause")
  - etc.
  
- Look into improving how the pixel color is computed. Good information at this site: http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_2_Phong_Mirrors_and_Shadows.shtml

- Optimize and clean up kernels
  - Add read_only,write_only,const and similar properties
  - Minimize reads and writes to global memory, for example by making a privat copy when accessing multiple times

- Optimize and clean up host code
  - When possible/beneficial change flags for cl::Buffers to as conservative ones as possible such as CL_MEM_READ_ONLY or CL_MEM_WRITE_ONLY.

## WIP

- Survive:
	- Add support for dying, taking damage etc.
- Bad guys: Zombies?
	- Make basic AI that can hurt the player(s)
- Game Launcher
	- Make Better UI.
- Add content
	- Add content; vehicles and weapons
