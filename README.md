# terrain-generator

## CSC305 Project 3
### Dustin Bolink & Konrad Schultz

This repository contains all necessary assets to run. it has a script called run.sh to compile and run.

Navigate to the root folder and try `$ ./run.sh`

Your system must be using opengl 4 and have the sdl2 dev dependency in your system or user includes.

See links for the [ubuntu ppa and instructions](https://www.phoronix.com/scan.php?page=news_item&px=Ubuntu-16.04-OI-Intel-GL-4.2), and [installing sdl2 dev package](https://wiki.libsdl.org/Installation).

### GUI and Navigation
The gui contains a window for adjusting flythrough camera/look at path values, speed, and manual mode. It also has a window for adjusting bezier curve points that the camera paths through. It also has a window for adjusting terrain generation parameters.

### Screenshots
![Screenshot1](https://raw.githubusercontent.com/kradical/terrain-generator/master/screenshots/Screenshot%20from%202017-04-09%2017-26-04.png)
![Screenshot2](https://raw.githubusercontent.com/kradical/terrain-generator/master/screenshots/Screenshot%20from%202017-04-09%2017-26-13.png)
![Screenshot3](https://raw.githubusercontent.com/kradical/terrain-generator/master/screenshots/Screenshot%20from%202017-04-09%2017-26-28.png)
![Screenshot4](https://raw.githubusercontent.com/kradical/terrain-generator/master/screenshots/Screenshot%20from%202017-04-09%2017-26-45.png)

### Features
This is a non-exhaustive list of features implemented:
* An array of vertices to form a plane
* Perlin noise to create a height map
* Texture sampling and blending to color terrain
* Calculated and used normals to blend based on terrain steepness
* Bezier curve for camera position
* Bezier curve for camera look at location
* Parameterized perlin noise inputs
* Parameterized Bezier curve points
* Parameterized automatic flythrough vs. manual mode
* Skybox

Anything parameterized can be played with at runtime via the imgui windows.
