#pragma once

class Framebuffer final
{
public:
   Framebuffer() : _idFBO(0), _width(0), _height(0)
   {
   }

   void Release()
   {
      // освобождаем ресурсы
      _colorBuffer.Release();
      glDeleteFramebuffers(1, &_idFBO);
   }

   void Bind() const
   {
      // устанавливаем фреймбуфер и его окно просмотра
      glBindFramebuffer(GL_FRAMEBUFFER, _idFBO);
      glViewport(0, 0, _width, _height);
   }

   void Unbind() const
   {
      // сбрасываем фреймбуфер
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   const Texture& GetColorBuffer() const
   {
      // возвращаем идентификатор цветового буфера
      return _colorBuffer;
   }

   unsigned int GetWidth()
   {
      // возвращаем ширину фреймбуфера
      return _width;
   }

   unsigned int GetHeight()
   {
      // возвращаем высоту фреймбуфера
      return _height;
   }

   void Build(unsigned int width, unsigned int height, bool depth)
   {
      // сохраняем размеры
      _width = width;
      _height = height;

      if (_idFBO == 0)
         glGenFramebuffers(1, &_idFBO);

      glBindFramebuffer(GL_FRAMEBUFFER, _idFBO);
      {
         // Создание текстуры для прикрепляемого объекта цвета
         _colorBuffer.GenerateID();
         glBindTexture(GL_TEXTURE_2D, _colorBuffer.GetID());
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer.GetID(), 0);

         // Создание объекта рендербуфера дла прикрепляемых объектов глубины и трафарета (сэмплирование мы не будет здесь проводить)
         if (depth)
         {
            unsigned int rbo;
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // использование одного объекта рендербуфера для буферов глубины и трафарета
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // теперь прикрепляем это дело
         }

        // Теперь, когда мы создали фреймбуфер и прикрепили все необходимые объекты, проверяем завершение формирования фреймбуфера
         if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

private:
   unsigned int _idFBO;
   unsigned int _width, _height;
   Texture _colorBuffer;
};