#include "simulation.h"

#include "scene.h"

#include "imgui.h"

#include "PerlinNoise.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

#include "preamble.glsl"

#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>
#include <iostream>

void Simulation::Init(Scene* scene)
{
    mScene = scene;

    PerlinNoise pn = PerlinNoise(0.1, 0.5, 10.0, 1, 1 ); // double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed


    int width = 25;
    int height = 25;

    float* newVertices = new float[2000];
    int index = 0;
    for (int y=0; y<=height-1; y++)
    {
        for (int x=0 ; x<=width-1; x++)
        {
            newVertices[index]   = (float)x;
            //std::cout << newVertices[index] << " ";
            newVertices[index+1] = pn.GetHeight(x,y);
            //std::cout << newVertices[index+1] << " ";
            newVertices[index+2] = (float)y;
            //std::cout << newVertices[index+2] << std::endl;
            index += 3;
        }
    }

    int numV = index;
    std::cout << "Vertices: " << numV << std::endl;

    index = 0;
    int* newIndices = new int[20000];
    int row = width;

    for(int w=0; w<width-1; w++){
        for(int h=0; h<height-1; h++){
            newIndices[index] = h + (w*row);
            //std::cout << newIndices[index] << " ";
            newIndices[index+1] = h + 1 + (w*row);
            //std::cout << newIndices[index+1] << " ";
            newIndices[index+2] = h + row + (w*row);
            //std::cout << newIndices[index+2] << std::endl;

            newIndices[index+3] = h + 1 + (w*row);
            //std::cout << newIndices[index+3] << " ";
            newIndices[index+4] = h + row + (w*row);
            //std::cout << newIndices[index+4] << " ";
            newIndices[index+5] = h + 1 + (w*row) + row;
            //std::cout << newIndices[index+5] << std::endl;
            index += 6;
        }
    }
    int numI = index;
    std::cout << "Indicies: " << numI << std::endl;

    GLuint newPositionBO;
    glGenBuffers(1, &newPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glBufferData(GL_ARRAY_BUFFER, numV * sizeof(float), newVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint newIndexBO;
    glGenBuffers(1, &newIndexBO);
    // Why not bind to GL_ELEMENT_ARRAY_BUFFER?
    // Because binding to GL_ELEMENT_ARRAY_BUFFER attaches the EBO to the currently bound VAO, which might stomp somebody else's state.
    glBindBuffer(GL_ARRAY_BUFFER, newIndexBO);
    glBufferData(GL_ARRAY_BUFFER, numI * sizeof(int), newIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Hook up VAO
    GLuint newMeshVAO;
    glGenVertexArrays(1, &newMeshVAO);

    glBindVertexArray(newMeshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newIndexBO);

    mScene->newMeshVAO = newMeshVAO;

    Camera mainCamera;
    mainCamera.Eye = glm::vec3(5.0f);
    glm::vec3 target = glm::vec3(0.0f);
    mainCamera.Look = normalize(target - mainCamera.Eye);
    mainCamera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainCamera.FovY = glm::radians(70.0f);
    mScene->MainCamera = mainCamera;

    mScene->MainCamera.isManual = true;
}

void Simulation::HandleEvent(const SDL_Event& ev)
{
    if (ev.type == SDL_MOUSEMOTION)
    {
        mDeltaMouseX += ev.motion.xrel;
        mDeltaMouseY += ev.motion.yrel;
    }
}

void Simulation::Update(float deltaTime)
{
    const Uint8* keyboard = SDL_GetKeyboardState(NULL);
    
    int mx, my;
    Uint32 mouse = SDL_GetMouseState(&mx, &my);

    bool* isManual = &mScene->MainCamera.isManual;

    if (mScene->MainCamera.isManual == true)
    {
        if ((mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0)
        {
            flythrough_camera_update_manual(
                value_ptr(mScene->MainCamera.Eye),
                value_ptr(mScene->MainCamera.Look),
                value_ptr(mScene->MainCamera.Up),
                NULL,
                deltaTime,
                5.0f, // eye_speed
                0.1f, // degrees_per_cursor_move
                80.0f, // max_pitch_rotation_degrees
                mDeltaMouseX, mDeltaMouseY,
                keyboard[SDL_SCANCODE_W], 
                keyboard[SDL_SCANCODE_A], 
                keyboard[SDL_SCANCODE_S], 
                keyboard[SDL_SCANCODE_D],
                keyboard[SDL_SCANCODE_SPACE], 
                keyboard[SDL_SCANCODE_LCTRL],
                0
            );
        }
    } else {
        flythrough_camera_update_automatic(
                value_ptr(mScene->MainCamera.Eye),
                value_ptr(mScene->MainCamera.Look),
                value_ptr(mScene->MainCamera.Up),
                NULL,
                deltaTime,
                5.0f, // eye_speed
                0
        );
    }

    mDeltaMouseX = 0;
    mDeltaMouseY = 0;

    if (ImGui::Begin("GUI Window"))
    {
        ImGui::Checkbox("Manual FlyThrough", isManual);
        ImGui::Text("Mouse Pos: (%d, %d)", mx, my);
    }
    ImGui::End();
}

void* Simulation::operator new(size_t sz)
{
    // zero out the memory initially, for convenience.
    void* mem = ::operator new(sz);
    memset(mem, 0, sz);
    return mem;
}
    