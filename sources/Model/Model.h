#pragma once

class Model final
{
public:
   Model()
   {
   }

   void Release()
   {
      for (unsigned int i = 0; i < meshes.size(); i++)
         meshes[i].Release();
   }

   void Render(const Shader& shader, std::function<void(const Shader&)> SetDynamicUniformsCallback = nullptr)
   {
      for (unsigned int i = 0; i < meshes.size(); i++)
         meshes[i].Render(shader, SetDynamicUniformsCallback);
   }

   void Load(const std::string& path)
   {
      // Чтение файла с помощью Assimp 
      Assimp::Importer importer;
      const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);

      // Проверка на ошибки
      if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
      {
         std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
         return;
      }

      // Получение пути к файлу
      _folderPath = path.substr(0, path.find_last_of('/'));

      // Рекурсивная обработка корневого узла Assimp
      ProcessNode(scene->mRootNode, scene);
   }

private:
   void ProcessNode(aiNode* node, const aiScene* scene)
   {
      // Обрабатываем каждый меш текущего узла
      for (unsigned int i = 0; i < node->mNumMeshes; i++)
      {
         // Узел содержит только индексы объектов в сцене.
         // Сцена же содержит все данные; узел - это лишь способ организации данных
         aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
         meshes.push_back(std::move(ProcessMesh(mesh, scene)));
      }
      // После того, как мы обработали все меши (если таковые имелись), мы начинаем рекурсивно обрабатывать каждый из дочерних узлов
      for (unsigned int i = 0; i < node->mNumChildren; i++)
      {
         ProcessNode(node->mChildren[i], scene);
      }
   }

   Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
   {
      // Данные для заполнения
      std::vector<Mesh::Vertex> vertices;
      std::vector<unsigned int> indices;
      std::vector<Texture> textures;

      // Цикл по всем вершинам меша
      for (unsigned int i = 0; i < mesh->mNumVertices; i++)
      {
         Mesh::Vertex vertex;
         glm::vec3 vector;

         // Координаты
         vector.x = mesh->mVertices[i].x;
         vector.y = mesh->mVertices[i].y;
         vector.z = mesh->mVertices[i].z;
         vertex.position = vector;

         // Нормали
         vector.x = mesh->mNormals[i].x;
         vector.y = mesh->mNormals[i].y;
         vector.z = mesh->mNormals[i].z;
         vertex.normal = vector;

         // Текстурные координаты
         if (mesh->mTextureCoords[0]) // если меш содержит текстурные координаты
         {
            glm::vec2 vec;
            // Вершина может содержать до 8 различных текстурных координат. Мы предполагаем, что не будем использовать модели,
            // в которых вершина может содержать несколько текстурных координат, поэтому всегда берем первый набор (0)
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
         }
         else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

         // Касательный вектор
         vector.x = mesh->mTangents[i].x;
         vector.y = mesh->mTangents[i].y;
         vector.z = mesh->mTangents[i].z;
         vertex.tangent = vector;

         // Вектор бинормали
         vector.x = mesh->mBitangents[i].x;
         vector.y = mesh->mBitangents[i].y;
         vector.z = mesh->mBitangents[i].z;
         vertex.bitangent = vector;
         vertices.push_back(vertex);
      }

      // Теперь проходимся по каждой грани меша (грань - это треугольник меша) и извлекаем соответствующие индексы вершин
      for (unsigned int i = 0; i < mesh->mNumFaces; i++)
      {
         aiFace face = mesh->mFaces[i];
         // Получаем все индексы граней и сохраняем их в векторе indices
         for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
      }
      // Обрабатываем материалы
      aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

      // загружаем диффузные текстуры
      std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
      textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

      // загружаем текстуры нормалей
      //std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS);
      //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

      // возвращаем Mesh, на основе полученных данных
      return Mesh(vertices, indices, textures);
   }

   // Проверяем все текстуры материалов заданного типа и загружам текстуры, если они еще не были загружены.
   // Необходимая информация возвращается в виде struct Texture
   std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type)
   {
      std::vector<Texture> textures;
      for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
      {
         aiString str;
         mat->GetTexture(type, i, &str);

         // Проверяем, не была ли текстура загружена ранее, и если - да, то пропускаем загрузку новой текстуры и переходим к следующей итерации
         bool skip = false;
         for (unsigned int j = 0; j < textures_loaded.size(); j++)
         {
            if (std::strcmp(textures_loaded[j].GetPath(), str.C_Str()) == 0)
            {
               textures.push_back(textures_loaded[j]);
               skip = true; // текстура с тем же путем к файлу уже загружена, переходим к следующей (оптимизация)
               break;
            }
         }

         if (!skip)
         {   // Если текстура еще не была загружена, то загружаем её
            Texture texture;
            texture.Load(this->_folderPath + '/' + str.C_Str());
            textures.push_back(texture);

            textures_loaded.push_back(texture);  // сохраняем её в массиве с уже загруженными текстурами, тем самым гарантируя, что у нас не появятся дубликаты текстур
         }
      }
      return textures;
   }

private:
   std::vector<Texture> textures_loaded; // (оптимизация) сохраняем все загруженные текстуры, чтобы убедиться, что они не загружены более одного раза
   std::vector<Mesh> meshes;
   std::string _folderPath;
};
