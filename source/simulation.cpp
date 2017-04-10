#include "simulation.h"

#include "scene.h"

#include "imgui.h"

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "flythrough_camera.h"

#include "../shaders/preamble.glsl"

#include "stb_image.h"

#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>
#include <iostream>

void Simulation::Init(Scene* scene)
{
    mScene = scene;

    Camera mainCamera;
    mainCamera.Eye = glm::vec3(0.0f, 13.0f, 0.0f);
    glm::vec3 target = glm::vec3(1.0f, 13.0f, 1.0f);
    mainCamera.Look = normalize(target - mainCamera.Eye);
    mainCamera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    mainCamera.FovY = glm::radians(45.0f);

    mScene->MainCamera = mainCamera;


    //Create Bezier Curve
    glm::vec3* CameraPoints = new glm::vec3[4];
    CameraPoints[0] = glm::vec3(0.0f, 13.0f, 0.0f);
    CameraPoints[1] = glm::vec3(WIDTH / 2.0f, 15.0f, HEIGHT / 2.0f);
    CameraPoints[2] = glm::vec3(WIDTH / 2.0f, 20.0f, -HEIGHT / 2.0f);
    CameraPoints[3] = glm::vec3(0.0f, 13.0f, 0.0f);

    mScene->MainCamera.cameraCurve = BezierCurve(4, CameraPoints);

    //Create Bezier Curve
    glm::vec3* LookAtPoints = new glm::vec3[4];
    LookAtPoints[0] = glm::vec3(30.0f, 0.0f, 60.0f);
    LookAtPoints[1] = glm::vec3(-57.0f, 0.0f, 90.0f);
    LookAtPoints[2] = glm::vec3(-65.0f, 0.0f, 54.0f);
    LookAtPoints[3] = glm::vec3(100.0f, 0.0f, 80.0f);

    mScene->MainCamera.lookAtCurve = BezierCurve(4, LookAtPoints);
    std::cout << "Init Complete" << std::endl;
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
    bool* isLocked = &mScene->MainCamera.isLocked;
    bool* autoIncrement = &mScene->MainCamera.autoIncrement;

    float movementSpeed = mScene->MainCamera.movementSpeed;


    if(mScene->MainCamera.isLocked == true){
        mScene->MainCamera.lookAtCurve.T = mScene->MainCamera.cameraCurve.T;
    }

    if(mScene->MainCamera.autoIncrement == true){
        mScene->MainCamera.lookAtCurve.T += movementSpeed/1000;
        if(mScene->MainCamera.lookAtCurve.T >= 1.0f){
            mScene->MainCamera.lookAtCurve.T = 0.0f;
        }
        mScene->MainCamera.cameraCurve.T += movementSpeed/1000;
        if(mScene->MainCamera.cameraCurve.T >= 1.0f){
            mScene->MainCamera.cameraCurve.T = 0.0f;
        }
    }

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

    if (ImGui::Begin("Camera Control"))
    {
        ImGui::Checkbox("Manual FlyThrough", isManual);
        ImGui::Checkbox("Lock Curves", isLocked);
        ImGui::Checkbox("Auto Increment", autoIncrement);
        ImGui::SliderFloat("Movement Speed", &mScene->MainCamera.movementSpeed, 0.0f, 1.0f);
        ImGui::SliderFloat("Camera Position", &mScene->MainCamera.cameraCurve.T, 0.0f, 1.0f);
        ImGui::SliderFloat("LookAt Position", &mScene->MainCamera.lookAtCurve.T, 0.0f, 1.0f);
    }
    ImGui::End();

    if (ImGui::Begin("Curve Control"))
    {
        ImGui::SliderFloat3("Camera Curve[0]", value_ptr(mScene->MainCamera.cameraCurve.Points[0]), 0.0f, 300.0f);
        ImGui::SliderFloat3("Camera Curve[1]", value_ptr(mScene->MainCamera.cameraCurve.Points[1]), 0.0f, 300.0f);
        ImGui::SliderFloat3("Camera Curve[2]", value_ptr(mScene->MainCamera.cameraCurve.Points[2]), 0.0f, 300.0f);
        ImGui::SliderFloat3("Camera Curve[3]", value_ptr(mScene->MainCamera.cameraCurve.Points[3]), 0.0f, 300.0f);
        ImGui::SliderFloat3("LookAt Curve[0]", value_ptr(mScene->MainCamera.lookAtCurve.Points[0]), 0.0f, 300.0f);
        ImGui::SliderFloat3("LookAt Curve[1]", value_ptr(mScene->MainCamera.lookAtCurve.Points[1]), 0.0f, 300.0f);
        ImGui::SliderFloat3("LookAt Curve[2]", value_ptr(mScene->MainCamera.lookAtCurve.Points[2]), 0.0f, 300.0f);
        ImGui::SliderFloat3("LookAt Curve[3]", value_ptr(mScene->MainCamera.lookAtCurve.Points[3]), 0.0f, 300.0f);
    }
    ImGui::End();

    if (ImGui::Begin("Terrain Parameters")) {
        ImGui::SliderFloat("Persistence", &mScene->persistence, 0.01f, 10.0f);
        ImGui::SliderFloat("Frequency",  &mScene->frequency, 0.01f, 10.0f);
        ImGui::SliderFloat("Amplitude",  &mScene->amplitude, 0.1f, 70.0f);
        ImGui::SliderInt("Octaves",  &mScene->octaves, 1, 12);
        ImGui::SliderInt("Randomseed",  &mScene->randomseed, 0, 100);
    }
    ImGui::End();

    float epsilon = 0.00001f;
    if (fabs(mScene->oldPersistence - mScene->persistence) > epsilon ||
        fabs(mScene->oldFrequency - mScene->frequency) > epsilon ||
        fabs(mScene->oldAmplitude - mScene->amplitude) > epsilon ||
        mScene->oldOctaves != mScene->octaves ||
        mScene->oldRandomseed != mScene->randomseed
        ) {
            mScene->oldPersistence = mScene->persistence; 
            mScene->oldFrequency = mScene->frequency;
            mScene->oldAmplitude = mScene->amplitude;
            mScene->oldOctaves = mScene->octaves;
            mScene->oldRandomseed = mScene->randomseed;

            mScene->InitVertices();
    }
}

void* Simulation::operator new(size_t sz)
{
    // zero out the memory initially, for convenience.
    void* mem = ::operator new(sz);
    memset(mem, 0, sz);
    return mem;
}
    