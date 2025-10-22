#pragma once

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <unordered_map>
#include <string>
#include <filesystem>
#include <future>
#include <thread>
#include <atomic>
#include <vector>

#include "Objects.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "Materials/Lambertian.hpp"
#include "SimplifiedData.hpp"
class ModelLoader
{
    using PtrImporter = std::unique_ptr<Assimp::Importer>;
    using ModelLoadFuture = std::future<std::pair<const aiScene*, PtrImporter>>;

private:
    inline static Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;

            if (!mesh->mNormals) // 检查法线指针是否有效 (非空)
            {
                throw (std::runtime_error("Mesh has no normals!"));
            }
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates? //默认不同纹理,纹理坐标相同,所以取0
            {
                vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
                vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
            }
            else
                vertex.texCoord = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices, std::move(Lambertian(color4(0.8f, 0.3f, 0.3f, 1.f)))); // 暂时不处理材质
    }

    /* [in]: loadedScene : Obj file imported in memory
     *  [out]: ptr Model : Model object, which is loaded to OpenGL context
     *  [process]: OpenGL Object Binding needs synchrours operation
     */
    inline static std::vector<Mesh> PostProcess(const aiScene& loadedScene)
    {
        std::vector<Mesh> meshes;
        for (size_t i = 0; i < loadedScene.mRootNode->mNumChildren; ++i)
        {
            // DebugOutput::AddLog("nums of Children of Node{}:{}\n", i, loadedScene.mRootNode->mChildren[i]->mNumChildren);
            auto& node = *(loadedScene.mRootNode->mChildren[i]);
            auto& mesh_id = node.mMeshes[0];
            auto& mesh = loadedScene.mMeshes[mesh_id];
            meshes.emplace_back(ProcessMesh(mesh, &loadedScene));
        }
        return meshes;
    }

    inline static void LoadAndProcessModel(Scene& scene, ModelLoadFuture& model_future)
    {
        try
        {
            auto [raw_model, importer] = model_future.get();
            auto&& meshes = PostProcess(*raw_model);
            for (auto&& mesh : meshes)
            {
                scene.addObject(std::make_shared<Mesh>(mesh));
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

public:
    struct ImportingContext
    {
        ModelLoadFuture model_future;
        std::string file_path;
    };
    inline static std::filesystem::path current_file_path;
    inline static std::vector<ImportingContext> importing_vec;

public:
    ModelLoader()
    {
    }
    ModelLoadFuture inline static LoadModelAsync(const std::string& path)
    {
        // aiScene 必须在 Importer 上下文环境才有效
        // aiScene 生命周期必须与 Importer 一致
        return std::async(
            std::launch::async, [path]
            {
                auto importer = std::make_unique<Assimp::Importer>();
                const aiScene* scene = importer->ReadFile(
                    path,
                    aiProcess_CalcTangentSpace |
                    aiProcess_Triangulate |
                    aiProcess_JoinIdenticalVertices |
                    aiProcess_SortByPType);

                if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
                    throw std::runtime_error("Load Failed: " + std::string(importer->GetErrorString()));
                }
                return std::make_pair(scene, std::move(importer)); });
    }

    inline static void LoadModelFileSync(const std::string& path, Scene& scene)
    {
        auto importer = std::make_unique<Assimp::Importer>();
        const aiScene* raw_model = importer->ReadFile(
            path,
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        try
        {
            auto&& meshes = PostProcess(*raw_model);
            for (auto&& mesh : meshes)
            {
                scene.addObject(std::make_shared<Mesh>(mesh));
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    // 发送加载模型请求
    inline static void LoadModelFile(const std::string& pFile)
    {
        importing_vec.emplace_back(ImportingContext{ LoadModelAsync(pFile), pFile });
    }

    // TODO 异常处理优化 ; 进度输出;
    /*
    [in]: scene 场景对象
    importing_vec 中将模型加载器加载好的文件 处理 并 加入到 scene 中
    */
    inline static void Run(Scene& scene)
    {
        auto it = importing_vec.begin();
        while (it != importing_vec.end())
        {
            if (it->model_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
            {
                current_file_path = std::filesystem::path(it->file_path);
                LoadAndProcessModel(scene, it->model_future);
                it = importing_vec.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};

namespace SimplifiedData
{
    class ModelLoader
    {
        using PtrImporter = std::unique_ptr<Assimp::Importer>;
        using ModelLoadFuture = std::future<std::pair<const aiScene*, PtrImporter>>;

    private:
        // ISSUE: 模型不规范(V,N,T不完整)时,无法加载转换
        inline static sd::Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
        {
            std::vector<sd::Vertex> vertices;
            std::vector<unsigned int> indices;

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                sd::Vertex vertex;
                vertex.position.x = mesh->mVertices[i].x;
                vertex.position.y = mesh->mVertices[i].y;
                vertex.position.z = mesh->mVertices[i].z;
                if (mesh->mNormals) // 检查法线指针是否有效 (非空)
                {
                    vertex.normal.x = mesh->mNormals[i].x;
                    vertex.normal.y = mesh->mNormals[i].y;
                    vertex.normal.z = mesh->mNormals[i].z;
                }

                if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates? //默认不同纹理,纹理坐标相同,所以取0
                {
                    vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
                    vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
                }
                else
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);
                vertices.push_back(vertex);
            }
            // process indices
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }
            return sd::Mesh(*pDataStroage, vertices, indices, Lambertian(glm::vec4(0.8f, 0.3f, 0.3f, 1.f))); // 暂时不处理材质
        }

        /* [in]: loadedScene : Obj file imported in memory
         *  [out]: ptr Model : Model object, which is loaded to OpenGL context
         *  [process]: OpenGL Object Binding needs synchrours operation
         */
        inline static std::vector<uint32_t> PostProcess(const aiScene& loadedScene)
        {
            std::vector<uint32_t> meshNodeIndices;
            for (size_t i = 0; i < loadedScene.mRootNode->mNumChildren; ++i)
            {
                // DebugOutput::AddLog("nums of Children of Node{}:{}\n", i, loadedScene.mRootNode->mChildren[i]->mNumChildren);
                auto& node = *(loadedScene.mRootNode->mChildren[i]);
                auto& mesh_id = node.mMeshes[0];
                auto& mesh = loadedScene.mMeshes[mesh_id];
                auto processedMesh = ProcessMesh(mesh, &loadedScene);
                meshNodeIndices.emplace_back(processedMesh.meshNodeIndex);
            }
            return meshNodeIndices;
        }

    private:
        inline static sd::DataStorage* pDataStroage = nullptr;

    public:
        ModelLoader()
        {
        }
        inline static uint32_t LoadModelFileSync(const std::string& path)
        {
            auto importer = std::make_unique<Assimp::Importer>();
            const aiScene* raw_model = importer->ReadFile(
                path,
                aiProcess_CalcTangentSpace |
                aiProcess_Triangulate |
                aiProcess_JoinIdenticalVertices |
                aiProcess_SortByPType);

            auto meshIndices = PostProcess(*raw_model);
            uint32_t root = sd::BVH::BuildBVHFromNodes(pDataStroage->nodeStorage, meshIndices.data(), 0, meshIndices.size()); // 问题
            return root;
        }
        inline static void SetDataStorage(sd::DataStorage* ptr)
        {
            pDataStroage = ptr;
        }
    };

}