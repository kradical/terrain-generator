#include "simulation.h"

#include "scene.h"

#include "imgui.h"

#include "PerlinNoise.h"

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

    // double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed
    PerlinNoise pn = PerlinNoise(0.1, 0.1, 15.0, 1, 0); 

    float vertices[HEIGHT][WIDTH][3];
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0 ; x < WIDTH; x++) {
            vertices[y][x][0] = (float)x;
            vertices[y][x][1] = pn.GetHeight(x,y);
            vertices[y][x][2] = (float)y;
        }
    }

    int indices[WIDTH - 1][HEIGHT - 1][6];
    for(int w = 0; w < WIDTH - 1; w++) {
        for(int h = 0; h < HEIGHT - 1; h++) {
            indices[w][h][0] = h + (w * WIDTH);
            indices[w][h][1] = h + 1 + (w * WIDTH);
            indices[w][h][2] = h + WIDTH + (w * WIDTH);
            indices[w][h][3] = h + 1 + (w * WIDTH);
            indices[w][h][4] = h + WIDTH + (w * WIDTH);
            indices[w][h][5] = h + 1 + (w * WIDTH) + WIDTH;
        }
    }

    GLuint newPositionBO;
    glGenBuffers(1, &newPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glBufferData(GL_ARRAY_BUFFER, NUMVERTICES * sizeof(float), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint newIndexBO;
    glGenBuffers(1, &newIndexBO);
    // Why not bind to GL_ELEMENT_ARRAY_BUFFER?
    // Because binding to GL_ELEMENT_ARRAY_BUFFER attaches the EBO to the currently bound VAO, which might stomp somebody else's state.
    glBindBuffer(GL_ARRAY_BUFFER, newIndexBO);
    glBufferData(GL_ARRAY_BUFFER, NUMINDICES * sizeof(int), indices, GL_STATIC_DRAW);
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

    // Set up water texture object
    std::string diffuse_texname_full = "assets/water.tga";
    int x, y, comp;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* pixels = stbi_load(diffuse_texname_full.c_str(), &x, &y, &comp, 4);
    stbi_set_flip_vertically_on_load(0);

    if (!pixels) {
        fprintf(stderr, "stbi_load(%s): %s\n", diffuse_texname_full.c_str(), stbi_failure_reason());
    } else {
        float maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

        glGenTextures(1, &mScene->waterMapTO);
        glBindTexture(GL_TEXTURE_2D, mScene->waterMapTO);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(pixels);
    }

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
    