#include "scene.h"

#include "preamble.glsl"

#include "tiny_obj_loader.h"
#include "stb_image.h"

#include <map>

#include <iostream>

void Scene::Init()
{
    // Need to specify size up front. These numbers are pretty arbitrary.
    DiffuseMaps = packed_freelist<DiffuseMap>(512);
    Materials = packed_freelist<Material>(512);
    Meshes = packed_freelist<Mesh>(512);
    Transforms = packed_freelist<Transform>(4096);
    Instances = packed_freelist<Instance>(4096);
}

void LoadMeshesFromFile(
    Scene* scene,
    const std::string& filename,
    std::vector<uint32_t>* loadedMeshIDs)
{
        // assume mtl is in the same folder as the obj
    std::string mtl_basepath = filename;
    size_t last_slash = mtl_basepath.find_last_of("/");
    if (last_slash == std::string::npos)
        mtl_basepath = "./";
    else
        mtl_basepath = mtl_basepath.substr(0, last_slash + 1);

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    if (!tinyobj::LoadObj(
        shapes, materials, err, 
        filename.c_str(), mtl_basepath.c_str(),
        tinyobj::calculate_normals | tinyobj::triangulation))
    {
        fprintf(stderr, "tinyobj::LoadObj(%s) error: %s\n", filename.c_str(), err.c_str());
        return;
    }
    
    if (!err.empty())
    {
        fprintf(stderr, "tinyobj::LoadObj(%s) warning: %s\n", filename.c_str(), err.c_str());
    }

    // Add materials to the scene
    std::map<std::string, uint32_t> diffuseMapCache;
    std::vector<uint32_t> newMaterialIDs;
    for (const tinyobj::material_t& materialToAdd : materials)
    {
        Material newMaterial;

        newMaterial.Name = materialToAdd.name;

        newMaterial.Ambient[0] = materialToAdd.ambient[0];
        newMaterial.Ambient[1] = materialToAdd.ambient[1];
        newMaterial.Ambient[2] = materialToAdd.ambient[2];
        newMaterial.Diffuse[0] = materialToAdd.diffuse[0];
        newMaterial.Diffuse[1] = materialToAdd.diffuse[1];
        newMaterial.Diffuse[2] = materialToAdd.diffuse[2];
        newMaterial.Specular[0] = materialToAdd.specular[0];
        newMaterial.Specular[1] = materialToAdd.specular[1];
        newMaterial.Specular[2] = materialToAdd.specular[2];
        newMaterial.Shininess = materialToAdd.shininess;

        newMaterial.DiffuseMapID = -1;

        if (!materialToAdd.diffuse_texname.empty())
        {
            auto cachedTexture = diffuseMapCache.find(materialToAdd.diffuse_texname);

            if (cachedTexture != end(diffuseMapCache))
            {
                newMaterial.DiffuseMapID = cachedTexture->second;
            }
            else
            {
                std::string diffuse_texname_full = mtl_basepath + materialToAdd.diffuse_texname;
                int x, y, comp;
                stbi_set_flip_vertically_on_load(1);
                stbi_uc* pixels = stbi_load(diffuse_texname_full.c_str(), &x, &y, &comp, 4);
                stbi_set_flip_vertically_on_load(0);

                if (!pixels)
                {
                    fprintf(stderr, "stbi_load(%s): %s\n", diffuse_texname_full.c_str(), stbi_failure_reason());
                }
                else
                {
                    float maxAnisotropy;
                    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

                    GLuint newDiffuseMapTO;
                    glGenTextures(1, &newDiffuseMapTO);
                    glBindTexture(GL_TEXTURE_2D, newDiffuseMapTO);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, 0);

                    DiffuseMap newDiffuseMap;
                    newDiffuseMap.DiffuseMapTO = newDiffuseMapTO;
                    
                    uint32_t newDiffuseMapID = scene->DiffuseMaps.insert(newDiffuseMap);

                    diffuseMapCache.emplace(materialToAdd.diffuse_texname, newDiffuseMapID);

                    newMaterial.DiffuseMapID = newDiffuseMapID;

                    stbi_image_free(pixels);
                }
            }
        }

        uint32_t newMaterialID = scene->Materials.insert(newMaterial);

        newMaterialIDs.push_back(newMaterialID);
    }

    // Add meshes (and prototypes) to the scene
    for (const tinyobj::shape_t& shapeToAdd : shapes)
    {
        const tinyobj::mesh_t& meshToAdd = shapeToAdd.mesh;

        Mesh newMesh;


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

        newMesh.PositionBO = newPositionBO;


        GLuint newIndexBO;
        glGenBuffers(1, &newIndexBO);
        // Why not bind to GL_ELEMENT_ARRAY_BUFFER?
        // Because binding to GL_ELEMENT_ARRAY_BUFFER attaches the EBO to the currently bound VAO, which might stomp somebody else's state.
        glBindBuffer(GL_ARRAY_BUFFER, newIndexBO);
        glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(int), newIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        newMesh.IndexBO = newIndexBO;

        // Hook up VAO
        GLuint newMeshVAO;
        glGenVertexArrays(1, &newMeshVAO);

        glBindVertexArray(newMeshVAO);

        glBindBuffer(GL_ARRAY_BUFFER, newMesh.PositionBO);
        glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.IndexBO);

        glBindVertexArray(0);

        newMesh.MeshVAO = newMeshVAO;

        uint32_t newMeshID = scene->Meshes.insert(newMesh);
        if (loadedMeshIDs)
        {
            loadedMeshIDs->push_back(newMeshID);
        }
    }
}

void AddMeshInstance(
    Scene* scene,
    uint32_t meshID,
    uint32_t* newInstanceID)
{
    Transform newTransform;
    newTransform.Scale = glm::vec3(1.0f);

    uint32_t newTransformID = scene->Transforms.insert(newTransform);

    Instance newInstance;
    newInstance.MeshID = meshID;
    newInstance.TransformID = newTransformID;

    uint32_t tmpNewInstanceID = scene->Instances.insert(newInstance);
    if (newInstanceID)
    {
        *newInstanceID = tmpNewInstanceID;
    }
}