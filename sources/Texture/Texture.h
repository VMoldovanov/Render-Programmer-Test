#pragma once

class Texture final
{
public:
   Texture() : _idTexture(0) 
   {
   }

   void Release()
   {
      // освобождаем ресурсы
      glDeleteTextures(1, &_idTexture);
   }

   const char* GetPath()      {  return _path.c_str();   }
   unsigned int GetID()       {  return _idTexture;      }

   void Bind(unsigned int textureUnit) const
   {
      // устанавливаем текстуру в текстурный юнит
      glActiveTexture(GL_TEXTURE0 + textureUnit);
      glBindTexture(GL_TEXTURE_2D, _idTexture);
   }

   void GenerateID()
   {
      // генерируем идентификатор
      if (_idTexture == 0)
         glGenTextures(1, &_idTexture);
   }

   void Load(const std::string& path, bool repeat=true, bool mipmap=true)
   {
      // загружаем из файла данные
      int width, height, nrComponents;
      stbi_set_flip_vertically_on_load(true);
      unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
      if (data)
      {
         // генерируем идентификатор
         GenerateID();

         // учитываем формат данных
         GLenum format = GL_RGB;
         if (nrComponents == 1)
            format = GL_RED;
         else if (nrComponents == 3)
            format = GL_RGB;
         else if (nrComponents == 4)
            format = GL_RGBA;

         // задаем текстурный модуль и его параметры
         glBindTexture(GL_TEXTURE_2D, _idTexture);
         {
            // передаем данные 
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            // генерируем мип-уровни
            if (mipmap)
               glGenerateMipmap(GL_TEXTURE_2D);
            // задаем отсечение текстуры
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
            // задаем фильтрацию текстуры
            if (mipmap)
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            else
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         }
         glBindTexture(GL_TEXTURE_2D, 0);
      }
      else
      {
         std::cerr << "Texture failed to load at path: " << path << std::endl;
      }
      // удаляем файловые данные
      stbi_image_free(data);
   }

private:
   unsigned int _idTexture;
   std::string _path;
};