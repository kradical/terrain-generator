#pragma once

#define HEIGHT 100
#define WIDTH 100
#define NUMVERTICES 30000 // width * height * 3
#define NUMINDICES 58806 // (widht - 1) * (height - 1) * 6

#include "opengl.h"
#include "packed_freelist.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <string>

struct DiffuseMap
{
    GLuint DiffuseMapTO;
};

struct Material
{
    std::string Name;

    float Ambient[3];
    float Diffuse[3];
    float Specular[3];
    float Shininess;
    uint32_t DiffuseMapID;
};

struct Mesh
{
    std::string Name;

    GLuint MeshVAO;
    GLuint PositionBO;
    GLuint TexCoordBO;
    GLuint NormalBO;
    GLuint IndexBO;

    GLuint IndexCount;
    GLuint VertexCount;

    std::vector<GLDrawElementsIndirectCommand> DrawCommands;
    std::vector<uint32_t> MaterialIDs;
};

struct Transform
{
    glm::vec3 Scale;
    glm::vec3 RotationOrigin;
    glm::quat Rotation;
    glm::vec3 Translation;
};

struct Instance
{
    uint32_t MeshID;
    uint32_t TransformID;
};

struct Camera
{
    // View
    glm::vec3 Eye;
    glm::vec3 Look;
    glm::vec3 Up;

    bool isManual;

    // Projection
    float FovY;
};

class Scene
{
public:
    packed_freelist<DiffuseMap> DiffuseMaps;
    packed_freelist<Material> Materials;
    packed_freelist<Mesh> Meshes;
    packed_freelist<Transform> Transforms;
    packed_freelist<Instance> Instances;

    Camera MainCamera;

    GLuint newMeshVAO;
    GLuint waterMapTO;

    void Init();
};

void LoadMeshesFromFile(
    Scene* scene,
    const std::string& filename,
    std::vector<uint32_t>* loadedMeshIDs);

void AddMeshInstance(
    Scene* scene,
    uint32_t meshID,
    uint32_t* newInstanceID);
