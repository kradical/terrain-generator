#include "simulation.h"

#include "scene.h"

#include "imgui.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

#include "preamble.glsl"

#include "stb_image.h"

#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>
#include <iostream>

void Simulation::Init(Scene* scene)
{
    mScene = scene;

    mScene->InitVertices();
    mScene->InitTexture(&(mScene->waterMapTO), "assets/water.tga");
    mScene->InitTexture(&(mScene->sandMapTO), "assets/sand.tga");
    mScene->InitTexture(&(mScene->grassMapTO), "assets/grass.tga");
    mScene->InitTexture(&(mScene->rockMapTO), "assets/rock.tga");
    mScene->InitTexture(&(mScene->snowMapTO), "assets/snow.tga");

    Camera mainCamera;
    mainCamera.Eye = glm::vec3(5.0f);
    glm::vec3 target = glm::vec3(0.0f);
    mainCamera.Look = normalize(target - mainCamera.Eye);
    mainCamera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainCamera.FovY = glm::radians(70.0f);
    mScene->MainCamera = mainCamera;
    mScene->MainCamera.isManual = true;

    //Create Bezier Curve
    glm::vec3* tmpPoints = new glm::vec3(0.0f,0.0f,0.0f);
    mScene->MainCamera.bezierCurve = BezierCurve(1, tmpPoints);
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
                mScene,
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
    