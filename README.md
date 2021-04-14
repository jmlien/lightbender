# lightbender
This program generates a light-bending metal structure in OBJ format. 

The light-bending metal structure is simply a _N_ by _M_ grid of panels, where _N_ and _M_ are used defined paramters. 
Each panel is a rotated rectangle whose oritentation is determined by the target image, the light sources and the camera location. 

The input parameters of the lightbender program include 
1. an image 
2. a .r file that specifies the light(s), the camera, the resolution of the structure 
3. the filename of the output OBJ file


# Compile lightbender on OSX

You will need to install glm, glew, glfw3

> brew install glm glew glfw3

Then use cmake and make 
> mkdir build; cd build; cmake ..; make

After the compilation is done, you should see a compile binary called "lightbender" in the build folder


# Run lightbender
There is an example in "examples" folder. Try

> cd examples; ../build/lightbender turing.r turing.obj
