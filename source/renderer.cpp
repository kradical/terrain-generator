#include "renderer.h"

#include "scene.h"

#include "imgui.h"

#include "../shaders/preamble.glsl"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>

#include <iostream>

void Renderer::Init(Scene* scene)
{
    mScene = scene;

    // feel free to increase the GLSL version if your computer supports it
    mShaders.SetVersion("410");
    mShaders.SetPreambleFile("shaders/preamble.glsl");

    mSceneSP = mShaders.AddProgramFromExts({ "shaders/scene.vert", "shaders/scene.frag" });
    mSkyboxSP = mShaders.AddProgramFromExts({ "shaders/skybox.vert", "shaders/skybox.frag"});
}

void Renderer::Resize(int width, int height)
{
    mBackbufferWidth = width;
    mBackbufferHeight = height;

    // Init Backbuffer FBO
    {
        glDeleteTextures(1, &mBackbufferColorTO);
        glGenTextures(1, &mBackbufferColorTO);
        glBindTexture(GL_TEXTURE_2D, mBackbufferColorTO);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, mBackbufferWidth, mBackbufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteTextures(1, &mBackbufferDepthTO);
        glGenTextures(1, &mBackbufferDepthTO);
        glBindTexture(GL_TEXTURE_2D, mBackbufferDepthTO);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mBackbufferWidth, mBackbufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteFramebuffers(1, &mBackbufferFBO);
        glGenFramebuffers(1, &mBackbufferFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBackbufferColorTO, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mBackbufferDepthTO, 0);
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "glCheckFramebufferStatus: %x\n", fboStatus);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Renderer::Render()
{
    mShaders.UpdatePrograms();

    // Clear last frame
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);

        glClearColor(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // render skybox
    if (*mSkyboxSP) {
        glDepthMask(GL_FALSE);
        glUseProgram(*mSkyboxSP);

        GLint SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mSkyboxSP, "ModelViewProjection");
        GLint SCENE_SKYBOX_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSkyboxSP, "Skybox");

        const Camera& mainCamera = mScene->MainCamera;

        glm::vec3 eye = mainCamera.Eye;
        glm::vec3 up = mainCamera.Up;

        glm::mat4 worldView = glm::mat4(glm::mat3(glm::lookAt(eye, eye + mainCamera.Look, up)));
        glm::mat4 viewProjection = glm::perspective(mainCamera.FovY, (float)mBackbufferWidth / mBackbufferHeight, 0.01f, 100.0f);
        glm::mat4 worldProjection = viewProjection * worldView;
        glm::mat4 modelViewProjection = worldProjection;
        glProgramUniformMatrix4fv(*mSkyboxSP, SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelViewProjection));

        // bind texture
        glActiveTexture(GL_TEXTURE0 + SCENE_SKYBOX_MAP_TEXTURE_BINDING);
        glProgramUniform1i(*mSkyboxSP, SCENE_SKYBOX_MAP_UNIFORM_LOCATION, SCENE_SKYBOX_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mScene->skyboxMapTO);

        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glViewport(0, 0, mBackbufferWidth, mBackbufferHeight);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(mScene->skyboxVAO);
        glDrawElementsBaseVertex(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, 0);
        
        glBindVertexArray(0);
 
        glDepthMask(GL_TRUE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glUseProgram(0);
    }

    // render scene
    if (*mSceneSP)
    {
        glUseProgram(*mSceneSP);

        GLint SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "ModelViewProjection");
        GLint SCENE_WATER_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "WaterMap");
        GLint SCENE_SAND_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "SandMap");
        GLint SCENE_GRASS_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "GrassMap");
        GLint SCENE_ROCK_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "RockMap");
        GLint SCENE_SNOW_MAP_UNIFORM_LOCATION = glGetUniformLocation(*mSceneSP, "SnowMap");

        const Camera& mainCamera = mScene->MainCamera;

        glm::vec3 eye = mainCamera.Eye;
        glm::vec3 up = mainCamera.Up;

        glm::mat4 worldView = glm::lookAt(eye, eye + mainCamera.Look, up);
        glm::mat4 viewProjection = glm::perspective(mainCamera.FovY, (float)mBackbufferWidth / mBackbufferHeight, 0.01f, 200.0f);
        glm::mat4 worldProjection = viewProjection * worldView;
        glm::mat4 modelViewProjection = worldProjection;
        glProgramUniformMatrix4fv(*mSceneSP, SCENE_MODELVIEWPROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(modelViewProjection));

        // bind textures
        glActiveTexture(GL_TEXTURE0 + SCENE_WATER_MAP_TEXTURE_BINDING);
        glProgramUniform1i(*mSceneSP, SCENE_WATER_MAP_UNIFORM_LOCATION, SCENE_WATER_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mScene->waterMapTO);
        
        glActiveTexture(GL_TEXTURE0 + SCENE_SAND_MAP_TEXTURE_BINDING);
        glProgramUniform1i(*mSceneSP, SCENE_SAND_MAP_UNIFORM_LOCATION, SCENE_SAND_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mScene->sandMapTO);
        
        glActiveTexture(GL_TEXTURE0 + SCENE_GRASS_MAP_TEXTURE_BINDING);
        glProgramUniform1i(*mSceneSP, SCENE_GRASS_MAP_UNIFORM_LOCATION, SCENE_GRASS_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mScene->grassMapTO);
        
        glActiveTexture(GL_TEXTURE0 + SCENE_ROCK_MAP_TEXTURE_BINDING);
        glProgramUniform1i(*mSceneSP, SCENE_ROCK_MAP_UNIFORM_LOCATION, SCENE_ROCK_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mScene->rockMapTO);

        glActiveTexture(GL_TEXTURE0 + SCENE_SNOW_MAP_TEXTURE_BINDING);
        glProgramUniform1i(*mSceneSP, SCENE_SNOW_MAP_UNIFORM_LOCATION, SCENE_SNOW_MAP_TEXTURE_BINDING);
        glBindTexture(GL_TEXTURE_2D, mScene->snowMapTO);

        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        glViewport(0, 0, mBackbufferWidth, mBackbufferHeight);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(mScene->newMeshVAO);
        glDrawElementsBaseVertex(GL_TRIANGLES, NUMINDICES, GL_UNSIGNED_INT, 0, 0);
        
        glBindVertexArray(0);
 
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glUseProgram(0);
    }

    // Render ImGui
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mBackbufferFBO);
        ImGui::Render();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // copy to window
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackbufferFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, mBackbufferWidth, mBackbufferHeight,
            0, 0, mBackbufferWidth, mBackbufferHeight,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void* Renderer::operator new(size_t sz)
{
    // zero out the memory initially, for convenience.
    void* mem = ::operator new(sz);
    memset(mem, 0, sz);
    return mem;
}
