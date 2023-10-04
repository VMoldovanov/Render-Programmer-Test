#pragma once

class Mesh final
{
public:
   struct Vertex
   {
      glm::vec3 position;
      glm::vec2 texCoords;
      glm::vec3 normal;
      glm::vec3 tangent;
      glm::vec3 bitangent;
   };

public:
   Mesh(std::vector<Mesh::Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
      : _vertices(std::move(vertices)), _indices(std::move(indices)), _textures(std::move(textures)), _idVAO(0), _idVBO(0), _idEBO(0)
   {  
      // создаем вершинный массив
      BuildVAO();
   }

   void Release()
   {
      // освобождаем ресурсы
      glDeleteVertexArrays(1, &_idVAO);
      for (unsigned int i = 0; i < _textures.size(); i++)
         _textures[i].Release();
   }

   const std::vector<Mesh::Vertex>& GetVertices() const  {    return _vertices;  }
   const std::vector<unsigned int>& GetIndices() const   {    return _indices;   }
   const std::vector<Texture>& GetTextures() const       {    return _textures;  }
   unsigned int GetVAO()                                 {    return _idVAO;     }

   void Render(const Shader& shader, std::function<void(const Shader&)> SetDynamicUniformsCallback = nullptr)
   {
      shader.Bind(SetDynamicUniformsCallback);
      {
         // устанавливаем текстуры в соответствующие текстурные юниты
         for (unsigned int i = 0; i < _textures.size(); i++)
            _textures[i].Bind(i);

         glBindVertexArray(_idVAO);
         glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
         glBindVertexArray(0);
      }
      shader.Unbind();
   }

private:
   void BuildVAO()
   {
      if (_idVAO == 0)
         glGenVertexArrays(1, &_idVAO);

      if (_idVBO == 0)
         glGenBuffers(1, &_idVBO);
      if (_idEBO == 0)
         glGenBuffers(1, &_idEBO);

      glBindVertexArray(_idVAO);
      {
         // загружаем данные в вершинный буфер
         glBindBuffer(GL_ARRAY_BUFFER, _idVBO);
         glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Mesh::Vertex), &_vertices[0], GL_STATIC_DRAW);

         // загружаем данные в индексный буфер
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _idEBO);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW);

         // устанавливаем указатель вершинных атрибутов: для координат
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)0);

         // устанавливаем указатель вершинных атрибутов: для текстурных координат
         glEnableVertexAttribArray(1);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, texCoords));

         // устанавливаем указатель вершинных атрибутов: для нормалей
         glEnableVertexAttribArray(2);
         glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, normal));

         // устанавливаем указатель вершинных атрибутов: для касательного вектора
         glEnableVertexAttribArray(3);
         glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, tangent));

         // устанавливаем указатель вершинных атрибутов: для вектора бинормали
         glEnableVertexAttribArray(4);
         glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, bitangent));
      }
      glBindVertexArray(0);
   }

private:
   std::vector<Mesh::Vertex> _vertices;
   std::vector<unsigned int> _indices;
   std::vector<Texture> _textures;
   unsigned int _idVAO;
   unsigned int _idVBO;
   unsigned int _idEBO;
};