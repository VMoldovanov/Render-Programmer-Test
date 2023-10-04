#include "Application.h"		// Precompiled Header

//StreamToFile cerrToFile("cerr.log");          // раскоментировать, если нужно сохранить ошибки в файле
Shader shaderGouraud;
Shader shaderBlinn;
Shader shaderQuad;
Shader shaderQuadBlur;
Shader shaderQuadMixMask;
Shader shaderQuadMixRect;
Shader shaderQuadRect;
Model model;
Quad quad;
Framebuffer framebuffer;
Framebuffer framebufferBlur0, framebufferBlur1;
Texture textureChecker;
Texture textureMask;
glm::mat4 matProjection;
glm::mat4 matView;
glm::mat4 matModel;
glm::vec3 lightPos;
glm::vec3 viewPos;
float cameraRadius = 200.f;
int codeMaskShader = 3;
int codeShadingShader = 0;
int bluringStrength = 20;
glm::ivec2 screenResolution = glm::ivec2(1024, 768);
ArcballCamera arcballCamera(screenResolution.x, screenResolution.y);

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
   arcballCamera.mouseButtonCallback(window, button, action, mods);
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
   arcballCamera.cursorPosCallback(window, x, y);
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
   cameraRadius -= (float)yoffset * 30.f;
   if (cameraRadius < 50.0f)
      cameraRadius = 50.f;
   else if (cameraRadius > 300.0f)
      cameraRadius = 300.f;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action == GLFW_PRESS)
   {
      // закрываем приложение
      if (key == GLFW_KEY_ESCAPE)
      {
         glfwSetWindowShouldClose(window, GL_TRUE);
      }
      // меняем маску
      if (key == GLFW_KEY_SPACE)
      {
         codeMaskShader++;

         int codeMask = codeMaskShader % 5;
         if (codeMask == 0)
            std::cout << "[Mask] Not used: The rendered scene." << std::endl;         
         else if (codeMask == 1)
            std::cout << "[Mask] Not used: Blur of the rendered scene." << std::endl;
         else if (codeMask == 2)
            std::cout << "[Mask] Mixing results (scene and blur) by the Rect-mask in the shader." << std::endl;
         else if (codeMask == 3)
            std::cout << "[Mask] Mixing results (scene and blur) by the Texture-mask." << std::endl;
         else
            std::cout << "[Mask] Mixing results (scene and blur) by the Stencil-mask." << std::endl;
      }
      // меняем модель затенения
      if (key == GLFW_KEY_ENTER)
      {
         codeShadingShader++;

         if (codeShadingShader % 2 == 0)
            std::cout << "[Shading] Blinn-mode (per-pixel)." << std::endl;
         else
            std::cout << "[Shading] Gouraud-mode (per-vertex)." << std::endl;
      }
      // изменяем интенсивность разымытия
      if (key == GLFW_KEY_UP)
      {
         bluringStrength = std::min(bluringStrength + 1, 20);

         std::cout << "[Blur] Bluring strength = " << bluringStrength  << std::endl;
      }
      if (key == GLFW_KEY_DOWN)
      {
         bluringStrength = std::max(bluringStrength - 1, 1);

         std::cout << "[Blur] Bluring strength = " << bluringStrength << std::endl;
      }
   }
}

void UpdateMVP(int width, int height)
{
   // матрица проекции
   matProjection = glm::perspective((float)glm::radians(40.f), (float)width / (float)height, cameraRadius * 0.25f, 500.f);
   
   // матрица вида (камеры)
   matView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -cameraRadius));
   matView = glm::rotate(matView, glm::radians(arcballCamera.GetAngle()), arcballCamera.GetRotationalAxis());

   // матрица модели
   matModel = glm::rotate(glm::mat4(1.0f), (float)glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));

   // переменные для шейдеров
   lightPos = glm::vec3(sin(glfwGetTime()), 1.0f, cos(glfwGetTime())) * cameraRadius;
   viewPos = glm::vec3(matView[3]);
}

void LoadShaders()
{
   const auto shadingStaticUniformsCallback{ [](const Shader& shader) {
      shader.SetUniformMat4("u_matModel", matModel);      
      shader.SetUniformMat3("u_matNormal", glm::mat3(glm::transpose(glm::inverse(matModel))));
      shader.SetUniformInt("u_diffuseMap", 0);
   } };
   // шейдер для 3D-модели (затенение по Гуро)
   shaderGouraud.Load("shaders/gouraud.vert", "shaders/gouraud.frag", shadingStaticUniformsCallback);
   // шейдер для 3D-модели (затенение по Блину)
   shaderBlinn.Load("shaders/blinn.vert", "shaders/blinn.frag", shadingStaticUniformsCallback);

   const auto quadStaticUniformsCallback{ [](const Shader& shader) {
      shader.SetUniformInt("screenMap", 0);
   } };
   // шейдер для вывода полноэкранного квада с текстурой
   shaderQuad.Load("shaders/quad.vert", "shaders/quad.frag", quadStaticUniformsCallback);
   // шейдер для вывода полноэкранного квада с текстурой, для разытия по Гаусу
   shaderQuadBlur.Load("shaders/quad.vert", "shaders/quad-blur.frag", quadStaticUniformsCallback);

   const auto MixMaskStaticUniformsCallback{ [](const Shader& shader) {
      shader.SetUniformInt("screenMap", 0);
      shader.SetUniformInt("blurMap", 1);
      shader.SetUniformInt("maskMap", 2);
   } };
   // шейдер для смешивания по маске, результатов рендера сцены и размытия этой сцены
   shaderQuadMixMask.Load("shaders/quad.vert", "shaders/quad-mix-mask.frag", MixMaskStaticUniformsCallback);

   const auto MixRectStaticUniformsCallback{ [](const Shader& shader) {
      shader.SetUniformInt("screenMap", 0);
      shader.SetUniformInt("blurMap", 1);
      shader.SetUniformVec4("u_rectScreen", glm::vec4(0.f, screenResolution.y >> 1, screenResolution.x, screenResolution.y >> 1) );
   } };
   // шейдер для смешивания по условию в шейдере, результатов рендера сцены и размытия этой сцены
   shaderQuadMixRect.Load("shaders/quad.vert", "shaders/quad-mix-rect.frag", MixRectStaticUniformsCallback);

   const auto rectStaticUniformsCallback{ [](const Shader& shader) {
      shader.SetUniformInt("screenMap", 0);
      shader.SetUniformVec4("u_rectScreen", glm::vec4(0.f, 0.f, screenResolution.x, screenResolution.y >> 1));
   } };
   // шейдер для вывода полноэкранного квада с текстурой по прямоугольной маске
   shaderQuadRect.Load("shaders/quad.vert", "shaders/quad-rect.frag", rectStaticUniformsCallback);
}

void InitData(GLFWwindow* window)
{
   // задаем константные переменные
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClearStencil(0);

   // загружаем модель
   model.Load("models/model.fbx");
   // загружаем текстуру фона
   textureChecker.Load("textures/checker.png", false, false);
   // загружаем текстуру маски
   textureMask.Load("textures/mask.png", false, false);

   // создаем фреймбуфер для основной сцены
   framebuffer.Build(screenResolution.x, screenResolution.y, true);
   // создаем фреймбуферы (уменьшенные в 2 раза) для двухпроходного размытия по Гаусу
   framebufferBlur0.Build(screenResolution.x >> 1, screenResolution.y >> 1, false);
   framebufferBlur1.Build(screenResolution.x >> 1, screenResolution.y >> 1, false);

   // создаем полноэкранный квад 
   quad.Build();

   // обновляем матрицу MVP
   UpdateMVP(screenResolution.x, screenResolution.y);
   // загружаем шейдера
   LoadShaders();
}

void ReleaseData()
{
   shaderGouraud.Release();
   shaderBlinn.Release();
   shaderQuad.Release();
   shaderQuadBlur.Release();
   shaderQuadMixMask.Release();
   shaderQuadMixRect.Release();
   shaderQuadRect.Release();

   model.Release();
   quad.Release();

   framebuffer.Release();
   framebufferBlur0.Release();
   framebufferBlur1.Release();

   textureChecker.Release();
   textureMask.Release();
}

void display(GLFWwindow* window)
{
   // обновляем матрицы
   UpdateMVP(screenResolution.x, screenResolution.y);

   // ВНИМАНИЕ! 
   // Это отладочный механизм пересборки шейдерной программы "на лету", когда отслеживается время последней перезаписи файла.
   // Если отладка в реальном времени не нужна, то эту строчку нужно закоментировать!
   LoadShaders();

   //---------------------------------------------------------------
   // рисуем сцену во фреймбуфер исходного размера
   //---------------------------------------------------------------
   framebuffer.Bind();
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);

      // выводим модель
      auto shadingDynamicUniformsCallback{ [](const Shader& shader) {
         shader.SetUniformMat4("u_matProjection", matProjection);
         shader.SetUniformMat4("u_matView", matView);
         shader.SetUniformVec3("u_lightPos", lightPos);
         shader.SetUniformVec3("u_viewPos", viewPos);
      } };
      int codeShading = codeShadingShader % 2;
      if (codeShading == 0)
         model.Render(shaderBlinn, shadingDynamicUniformsCallback);
      else
         model.Render(shaderGouraud, shadingDynamicUniformsCallback);

      // выводим полноэкранный квад фона      
      quad.Render(shaderQuad, nullptr, &textureChecker);
   }
   framebuffer.Unbind();

   //---------------------------------------------------------------
   // запускаем механизм размытия по Гаусу
   //---------------------------------------------------------------

   // выводим полноэкранный квад рендера сцены в уменьшенный фреймбуфер
   framebufferBlur0.Bind();
   {
      quad.Render(shaderQuad, nullptr, &framebuffer.GetColorBuffer());
   }
   framebufferBlur0.Unbind();

   // запускаем Ping-Pong циклы разымытия
   for (int i = 0; i < bluringStrength; i++)
   {
      // горизонтальное размытие уменьшенного фреймбуфера
      framebufferBlur1.Bind();
      {
         const auto DynamicUniformsCallback{ [](const Shader& shader) {
            shader.SetUniformFloat("u_horizontal", true);
         } };
         quad.Render(shaderQuadBlur, DynamicUniformsCallback, &framebufferBlur0.GetColorBuffer());
      }
      framebufferBlur1.Unbind();

      // вертикальное размытие уменьшенного фреймбуфера
      framebufferBlur0.Bind();
      {
         // выводим полноэкранный квад фона      
         const auto DynamicUniformsCallback{ [](const Shader& shader) {
            shader.SetUniformFloat("u_horizontal", false);
         } };
         quad.Render(shaderQuadBlur, DynamicUniformsCallback, &framebufferBlur1.GetColorBuffer());
      }
      framebufferBlur0.Unbind();
   }

   //---------------------------------------------------------------
   // осуществляем смешивание результатов размытия и рендера сцены
   //---------------------------------------------------------------

   // задаем окно просмотра
   glViewport(0, 0, screenResolution.x, screenResolution.y);
   // чистим фреймбуфер
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glDisable(GL_DEPTH_TEST);

   // выводим микс с результатами рендера сцены и результатом размытия этой сцены
   int codeMask = codeMaskShader % 5;
   if (codeMask == 0)
      quad.Render(shaderQuad, nullptr, &framebuffer.GetColorBuffer()); 
   else if (codeMask == 1)
      quad.Render(shaderQuad, nullptr, &framebufferBlur0.GetColorBuffer());
   else if (codeMask == 2)
      quad.Render(shaderQuadMixRect, nullptr, &framebuffer.GetColorBuffer(), &framebufferBlur0.GetColorBuffer());
   else if (codeMask == 3)
      quad.Render(shaderQuadMixMask, nullptr, &framebuffer.GetColorBuffer(), &framebufferBlur0.GetColorBuffer(), &textureMask);
   else
   {  
      glEnable(GL_STENCIL_TEST);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

      // выводим маску только в буффер трафарета
      glStencilFunc(GL_ALWAYS, 1, 0xFF); 
      glStencilMask(0xFF);
      glColorMask(false, false, false, false);      
      quad.Render(shaderQuadRect, nullptr);
      glColorMask(true, true, true, true);

      // выводим не размытое изображение там, где не выводилась маска
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      quad.Render(shaderQuad, nullptr, &framebuffer.GetColorBuffer());

      // выводим размытое изображение там, где выводилась маска
      glStencilFunc(GL_EQUAL, 1, 0xFF);
      glStencilMask(0x00);
      quad.Render(shaderQuad, nullptr, &framebufferBlur0.GetColorBuffer());

      glStencilMask(0xFF);
      glDisable(GL_STENCIL_TEST);
   }
}

int main()
{
   // инициализируем оконную библиотеку
   if (!glfwInit())
      exit(EXIT_FAILURE);

   // задаем версию OpenGL|ES
   glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   // настройки окна
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   // инициализируем окно
   GLFWwindow* window = glfwCreateWindow(screenResolution.x, screenResolution.y, "Ubisoft - Render Programmer Test by Vladimir Moldovanov (vlad.moldovanov@gmail.com), 27.Sep-29.Sep.2023", NULL, NULL);
   if (window == NULL)
   {
      std::cerr << "GLFW: Error creating an application window." << std::endl;
      glfwTerminate();
      exit(EXIT_FAILURE);
   }

   // выводим в консоли "шапку"
   std::cout << "--------------------------------------------------------------------------------------------------------------------\n";
   std::cout << "The following publicly available libraries are used in the project:\n";
   std::cout << " + GLAD, generated for the OpenGL 3.3 core (https://glad.dav1d.de/)\n";
   std::cout << " + GLFW 3.3.8, API for creating windows, contextsand surfaces, getting input dat aand events (https://www.glfw.org/)\n";
   std::cout << " + stb_image 2.28, image file loader (https://github.com/nothings/stb/blob/master/stb_image.h)\n";
   std::cout << " + GLM 0.9.9.7, mathematics library on the GLSL specifications (https://github.com/g-truc/glm)\n";
   std::cout << " + Assimp 5.3.1, loader of 3D-models (https://github.com/assimp/assimp)\n";
   std::cout << "--------------------------------------------------------------------------------------------------------------------\n";
   std::cout << "Use mouse left-button to rotate the camera.\n";
   std::cout << "Use mouse-roll to zoom in/out the camera.\n";
   std::cout << "Press 'Enter' to change variants of the shading.\n";
   std::cout << "Press 'Space' to change variants of the splitting mask.\n";
   std::cout << "Press 'Up'/'Down' key to change strenght of the bluring.\n";
   std::cout << "Press 'Esc' to exit the program.\n";
   std::cout << "--------------------------------------------------------------------------------------------------------------------\n";

   // задаем текущий GL-контекст
   glfwMakeContextCurrent(window);

   // связываем адреса GL-методов с реализацией драйвера нужной версии OpenGL|ES
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      std::cerr << "GLAD: Address initialization error." << std::endl;
      exit(EXIT_FAILURE);
   }

   // инициализируем данные в GL-контексте
   InitData(window);

   // задаем обработчик нажатий клавиш клавиатуры
   glfwSetKeyCallback(window, KeyCallback);
   // задаем обработчик мышки
   glfwSetCursorPosCallback(window, cursorPosCallback);
   glfwSetMouseButtonCallback(window, mouseButtonCallback);
   glfwSetScrollCallback(window, mouseScrollCallback);

   // запускаем основной цикл
   while (!glfwWindowShouldClose(window))
   {
      // вывод во фреймбуфер
      display(window);
      // переключение буферов
      glfwSwapBuffers(window);
      // обработка событий GLFW
      glfwPollEvents();
   }

   // освобождаем наши ресурсы
   ReleaseData();

   // освобождаем ресурсы
   glfwDestroyWindow(window);
   glfwTerminate();
   exit(EXIT_SUCCESS);
}