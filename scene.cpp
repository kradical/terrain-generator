#include "scene.h"
#include "PerlinNoise.h"
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

    InitVertices();
    InitTexture(&waterMapTO, "assets/water.tga");
    InitTexture(&sandMapTO, "assets/sand.tga");
    InitTexture(&grassMapTO, "assets/grass.tga");
    InitTexture(&rockMapTO, "assets/rock.tga");
    InitTexture(&snowMapTO, "assets/snow.tga");

    InitSkyboxVertices();
    InitSkyboxTextures();
}

void Scene::InitVertices() {
    if (pn != NULL) {
        delete pn;
        pn = NULL;
    }

    pn = new PerlinNoise(persistence, frequency, amplitude, octaves, randomseed);

    int min = 0;
    int max = 0;

    float vertices[HEIGHT][WIDTH][3];
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0 ; x < WIDTH; x++) {
            float height = pn->GetHeight(x, y);

            vertices[y][x][0] = (float)x - (WIDTH / 2.0f);
            vertices[y][x][1] = height;
            vertices[y][x][2] = (float)y - (HEIGHT / 2.0f);

            if (height > max) max = height;
            if (height < min) min = height;
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

    float normals[HEIGHT][WIDTH][3] = { 0 };
    for(int w = 0; w < WIDTH - 1; w++) {
        for(int h = 0; h < HEIGHT - 1; h++) {
            int i0 = indices[w][h][0];
            int i1 = indices[w][h][1];
            int i2 = indices[w][h][2];

            glm::vec3 v0 = glm::vec3((*vertices)[i0][0], (*vertices)[i0][1], (*vertices)[i0][2]);
            glm::vec3 v1 = glm::vec3((*vertices)[i1][0], (*vertices)[i1][1], (*vertices)[i1][2]);
            glm::vec3 v2 = glm::vec3((*vertices)[i2][0], (*vertices)[i2][1], (*vertices)[i2][2]);

            glm::vec3 normal1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            (*normals)[i0][0] += normal1.x;
            (*normals)[i0][1] += normal1.y;
            (*normals)[i0][2] += normal1.z;

            (*normals)[i1][0] += normal1.x;
            (*normals)[i1][1] += normal1.y;
            (*normals)[i1][2] += normal1.z;

            (*normals)[i2][0] += normal1.x;
            (*normals)[i2][1] += normal1.y;
            (*normals)[i2][2] += normal1.z;

            int i3 = indices[w][h][3];
            int i4 = indices[w][h][4];
            int i5 = indices[w][h][5];

            glm::vec3 v3 = glm::vec3((*vertices)[i3][0], (*vertices)[i3][1], (*vertices)[i3][2]);
            glm::vec3 v4 = glm::vec3((*vertices)[i4][0], (*vertices)[i4][1], (*vertices)[i4][2]);
            glm::vec3 v5 = glm::vec3((*vertices)[i5][0], (*vertices)[i5][1], (*vertices)[i5][2]);

            glm::vec3 normal2 = glm::normalize(glm::cross(v4 - v3, v5 - v3));

            (*normals)[i3][0] += normal2.x;
            (*normals)[i3][1] += normal2.y;
            (*normals)[i3][2] += normal2.z;

            (*normals)[i4][0] += normal2.x;
            (*normals)[i4][1] += normal2.y;
            (*normals)[i4][2] += normal2.z;

            (*normals)[i5][0] += normal2.x;
            (*normals)[i5][1] += normal2.y;
            (*normals)[i5][2] += normal2.z;
        }
    }

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0 ; x < WIDTH; x++) {
            glm::vec3 v = glm::vec3(normals[y][x][0], normals[y][x][1], normals[y][x][2]);
            glm::vec3 normal = glm::normalize(v);

            normals[y][x][0] = normal.x; 
            normals[y][x][1] = normal.y;
            normals[y][x][2] = normal.z;
        }
    }

    GLuint newPositionBO;
    glGenBuffers(1, &newPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glBufferData(GL_ARRAY_BUFFER, NUMVERTICES * sizeof(float), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint newNormalBO;
    glGenBuffers(1, &newNormalBO);
    glBindBuffer(GL_ARRAY_BUFFER, newNormalBO);
    glBufferData(GL_ARRAY_BUFFER, NUMVERTICES * sizeof(float), normals, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint newIndexBO;
    glGenBuffers(1, &newIndexBO);
    // Why not bind to GL_ELEMENT_ARRAY_BUFFER?
    // Because binding to GL_ELEMENT_ARRAY_BUFFER attaches the EBO to the currently bound VAO, which might stomp somebody else's state.
    glBindBuffer(GL_ARRAY_BUFFER, newIndexBO);
    glBufferData(GL_ARRAY_BUFFER, NUMINDICES * sizeof(int), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Hook up VAO
    glGenVertexArrays(1, &newMeshVAO);

    glBindVertexArray(newMeshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);

    glBindBuffer(GL_ARRAY_BUFFER, newNormalBO);
    glVertexAttribPointer(SCENE_NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(SCENE_NORMAL_ATTRIB_LOCATION);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newIndexBO);
}

void Scene::InitTexture(GLuint* mapTO, std::string texname) {
    int x, y, comp;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* pixels = stbi_load(texname.c_str(), &x, &y, &comp, 4);
    stbi_set_flip_vertically_on_load(0);

    if (!pixels) {
        fprintf(stderr, "stbi_load(%s): %s\n", texname.c_str(), stbi_failure_reason());
    } else {
        float maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);

        glGenTextures(1, mapTO);
        glBindTexture(GL_TEXTURE_2D, *mapTO);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(pixels);
    }
}

void Scene::InitSkyboxVertices() {
    float vertices[8][3] = {
        { -1.0f, -1.0f, -1.0f },
        { -1.0f, -1.0f, 1.0f },
        { -1.0f, 1.0f, -1.0f },
        { -1.0f, 1.0f, 1.0f },
        { 1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, 1.0f },
        { 1.0f, 1.0f, -1.0f },
        { 1.0f, 1.0f, 1.0f },
    };

    int indices[36] {
        0, 1, 2,
        2, 3, 1,
        1, 3, 7,
        7, 5, 1,
        1, 5, 0,
        0, 4, 5,
        0, 4, 2,
        2, 4, 6,
        2, 3, 6,
        6, 7, 3,
        7, 6, 4,
        4, 7, 5,
    };

    GLuint newPositionBO;
    glGenBuffers(1, &newPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint newIndexBO;
    glGenBuffers(1, &newIndexBO);
    // Why not bind to GL_ELEMENT_ARRAY_BUFFER?
    // Because binding to GL_ELEMENT_ARRAY_BUFFER attaches the EBO to the currently bound VAO, which might stomp somebody else's state.
    glBindBuffer(GL_ARRAY_BUFFER, newIndexBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(int), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Hook up VAO
    glGenVertexArrays(1, &skyboxVAO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
    glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newIndexBO);
}

void Scene::InitSkyboxTextures() {
    glGenTextures(1, &skyboxMapTO);
    glActiveTexture(GL_TEXTURE0);

    std::string cubeMapNames[6] = {
        "assets/right.jpg",
        "assets/left.jpg",
        "assets/top.jpg",
        "assets/bottom.jpg",
        "assets/back.jpg",
        "assets/front.jpg",
    };

    int width,height, comp;
    stbi_uc* pixels;
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxMapTO);
    for(GLuint i = 0; i < 6; i++) {
        pixels = stbi_load(cubeMapNames[i].c_str(), &width, &height, &comp, 4);

        if (!pixels) {
            fprintf(stderr, "stbi_load(%s): %s\n", cubeMapNames[i].c_str(), stbi_failure_reason());
        } else {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            stbi_image_free(pixels);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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

        newMesh.Name = shapeToAdd.name;

        newMesh.IndexCount = (GLuint)meshToAdd.indices.size();
        newMesh.VertexCount = (GLuint)meshToAdd.positions.size() / 3;

        if (meshToAdd.positions.empty())
        {
            // should never happen
            newMesh.PositionBO = 0;
        }
        else
        {
            GLuint newPositionBO;
            glGenBuffers(1, &newPositionBO);
            glBindBuffer(GL_ARRAY_BUFFER, newPositionBO);
            glBufferData(GL_ARRAY_BUFFER, meshToAdd.positions.size() * sizeof(meshToAdd.positions[0]), meshToAdd.positions.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            newMesh.PositionBO = newPositionBO;
        }

        if (meshToAdd.texcoords.empty())
        {
            newMesh.TexCoordBO = 0;
        }
        else
        {
            GLuint newTexCoordBO;
            glGenBuffers(1, &newTexCoordBO);
            glBindBuffer(GL_ARRAY_BUFFER, newTexCoordBO);
            glBufferData(GL_ARRAY_BUFFER, meshToAdd.texcoords.size() * sizeof(meshToAdd.texcoords[0]), meshToAdd.texcoords.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            newMesh.TexCoordBO = newTexCoordBO;
        }

        if (meshToAdd.normals.empty())
        {
            newMesh.NormalBO = 0;
        }
        else
        {
            GLuint newNormalBO;
            glGenBuffers(1, &newNormalBO);
            glBindBuffer(GL_ARRAY_BUFFER, newNormalBO);
            glBufferData(GL_ARRAY_BUFFER, meshToAdd.normals.size() * sizeof(meshToAdd.normals[0]), meshToAdd.normals.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            newMesh.NormalBO = newNormalBO;
        }

        if (meshToAdd.indices.empty())
        {
            // should never happen
            newMesh.IndexBO = 0;
        }
        else
        {
            GLuint newIndexBO;
            glGenBuffers(1, &newIndexBO);
            // Why not bind to GL_ELEMENT_ARRAY_BUFFER?
            // Because binding to GL_ELEMENT_ARRAY_BUFFER attaches the EBO to the currently bound VAO, which might stomp somebody else's state.
            glBindBuffer(GL_ARRAY_BUFFER, newIndexBO);
            glBufferData(GL_ARRAY_BUFFER, meshToAdd.indices.size() * sizeof(meshToAdd.indices[0]), meshToAdd.indices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            newMesh.IndexBO = newIndexBO;
        }

        // Hook up VAO
        {
            GLuint newMeshVAO;
            glGenVertexArrays(1, &newMeshVAO);

            glBindVertexArray(newMeshVAO);

            if (newMesh.PositionBO)
            {
                glBindBuffer(GL_ARRAY_BUFFER, newMesh.PositionBO);
                glVertexAttribPointer(SCENE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glEnableVertexAttribArray(SCENE_POSITION_ATTRIB_LOCATION);
            }

            if (newMesh.TexCoordBO)
            {
                glBindBuffer(GL_ARRAY_BUFFER, newMesh.TexCoordBO);
                glVertexAttribPointer(SCENE_TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glEnableVertexAttribArray(SCENE_TEXCOORD_ATTRIB_LOCATION);
            }

            if (newMesh.NormalBO)
            {
                glBindBuffer(GL_ARRAY_BUFFER, newMesh.NormalBO);
                glVertexAttribPointer(SCENE_NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glEnableVertexAttribArray(SCENE_NORMAL_ATTRIB_LOCATION);
            }

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.IndexBO);

            glBindVertexArray(0);

            newMesh.MeshVAO = newMeshVAO;
        }

        // split mesh into draw calls with different materials
        int numFaces = (int)meshToAdd.indices.size() / 3;
        int currMaterialFirstFaceIndex = 0;
        for (int faceIdx = 0; faceIdx < numFaces; faceIdx++)
        {
            bool isLastFace = faceIdx + 1 == numFaces;
            bool isNextFaceDifferent = isLastFace || meshToAdd.material_ids[faceIdx + 1] != meshToAdd.material_ids[faceIdx];
            if (isNextFaceDifferent)
            {
                GLDrawElementsIndirectCommand currDrawCommand;
                currDrawCommand.count = ((faceIdx + 1) - currMaterialFirstFaceIndex) * 3;
                currDrawCommand.primCount = 1;
                currDrawCommand.firstIndex = currMaterialFirstFaceIndex * 3;
                currDrawCommand.baseVertex = 0;
                currDrawCommand.baseInstance = 0;

                uint32_t currMaterialID = newMaterialIDs[meshToAdd.material_ids[faceIdx]];

                newMesh.DrawCommands.push_back(currDrawCommand);
                newMesh.MaterialIDs.push_back(currMaterialID);

                currMaterialFirstFaceIndex = faceIdx + 1;
            }
        }

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