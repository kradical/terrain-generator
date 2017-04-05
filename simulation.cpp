#include "simulation.h"

#include "scene.h"

#include "imgui.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

#include "preamble.glsl"

#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>

void Simulation::Init(Scene* scene)
{
    mScene = scene;

    float newVertices[4][3]  = {
        {-10.0, 0.0, -10.0}, 
        {10.0, 0.0, -10.0}, 
        {10.0, 0.0, 10.0}, 
        {-10.0, 0.0, 10.0}
    };

    int newIndices[6] = {
        0, 1, 2, 
        0, 2, 3
    };

    GLuint newPositionBO;
    glGenBuffers(1, &newPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), newVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint newIndexBO;
    glGenBuffers(1, &newIndexBO);
    // Why not bind to GL_ELEMENT_ARRAY_BUFFER?
    // Because binding to GL_ELEMENT_ARRAY_BUFFER attaches the EBO to the currently bound VAO, which might stomp somebody else's state.
    glBindBuffer(GL_ARRAY_BUFFER, newIndexBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(int), newIndices, GL_STATIC_DRAW);
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

    if ((mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0)
    {
        flythrough_camera_update(
            value_ptr(mScene->MainCamera.Eye),
            value_ptr(mScene->MainCamera.Look),
            value_ptr(mScene->MainCamera.Up),
            NULL,
            deltaTime,
            5.0f, // eye_speed
            0.1f, // degrees_per_cursor_move
            80.0f, // max_pitch_rotation_degrees
            mDeltaMouseX, mDeltaMouseY,
            keyboard[SDL_SCANCODE_W], keyboard[SDL_SCANCODE_A], keyboard[SDL_SCANCODE_S], keyboard[SDL_SCANCODE_D],
            keyboard[SDL_SCANCODE_SPACE], keyboard[SDL_SCANCODE_LCTRL],
            0);
    }

    mDeltaMouseX = 0;
    mDeltaMouseY = 0;

    if (ImGui::Begin("Example GUI Window"))
    {
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
    