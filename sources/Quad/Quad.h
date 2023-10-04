#pragma once

class Quad final
{
public:
   struct Vertex
   {
      glm::vec3 position;
      glm::vec2 texCoords;
   };

public:
   Quad() : _idVAO(0)
   {
   }

   void Release()
   {
      glDeleteVertexArrays(1, &_idVAO);
   }

   void Render(const Shader& shader, std::function<void(const Shader&)> DynamicUniformsCallback = nullptr,
      const Texture* texture0 = nullptr, const Texture* texture1 = nullptr,
      const Texture* texture2 = nullptr, const Texture* texture3 = nullptr) const
   {
      glBindVertexArray(_idVAO);
      {
         if (texture0)  texture0->Bind(0);
         if (texture1)  texture1->Bind(1);
         if (texture2)  texture2->Bind(2);
         if (texture3)  texture3->Bind(3);

         shader.Bind(DynamicUniformsCallback);
         glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
         shader.Unbind();
      }
      glBindVertexArray(0);
   }

   void Build()
   {
      Quad::Vertex vertex[] = {
         {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f,  1.0f)},		// верхняя левая 
         {glm::vec3(-1.0f,-1.0f, 0.0f), glm::vec2(0.0f,  0.0f)},		// нижняя левая		 
         {glm::vec3(1.0f,  1.0f, 0.0f), glm::vec2(1.0f,  1.0f)},		// верхняя правая
         {glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f,  0.0f)},		// нижняя правая
      };
      unsigned int indices[] = { 0, 1, 2, 3 };

      if (_idVAO == 0)
         glGenVertexArrays(1, &_idVAO);

      GLuint idVBO;
      glGenBuffers(1, &idVBO);
      GLuint idEBO;
      glGenBuffers(1, &idEBO);

      glBindVertexArray(_idVAO);
      {
         // загружаем данные в вершинный буфер
         glBindBuffer(GL_ARRAY_BUFFER, idVBO);
         glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Quad::Vertex), vertex, GL_STATIC_DRAW);

         // загружаем данные в индексный буфер
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idEBO);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

         // устанавливаем указатель вершинных атрибутов: для текстурных координат
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Quad::Vertex), (void*)0);

         // устанавливаем указатель вершинных атрибутов: для нормалей
         glEnableVertexAttribArray(1);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Quad::Vertex), (void*)offsetof(Quad::Vertex, texCoords));
      }
      glBindVertexArray(0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glDeleteBuffers(1, &idVBO);
      glDeleteBuffers(1, &idEBO);
   }

private:
   std::filesystem::path _pathVS;
   std::filesystem::path _pathFS;
   unsigned int _idVAO;
};