Ray Tracing – C++

This code gets the objects and the properties of scene from the file. It is a little bit different from the example file. 
Camera contains eye and look at point. (rest of them are not using in the code, it will calculate by those attributes.).
The sphere properties contain color, ks, ka, center position and a radius. 
The plane properties contain color, ks, ka, a point on the plane and a normal vector.
Also this code supports the triangle for making polygons. This also has properties such as color, ks, ka, and 3 corner point of the triangle.
The ambient of light and the light are same as the example code. 

For having a better reflection, I use a black and white floor which is appear if the ks is 2. ( e.g #10)

The scene can have several light source and several spheres and triangles.

How to run the code?
1)	g++ main.cpp
2)	./a.out test_file_name

You can see several examples in the Results folder with their .scn files. 

