#pragma once

// включаем _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4996 )

// С библиотеки
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STL библиотеки
#include <functional>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <chrono>

// сгенерированный для ядра OpenGL|ES 3.2
#include <GLAD/glad.h>

// API для создания окон, контекстов и поверхностей, получения входных данных и событий
#include <GLFW/glfw3.h>
#pragma comment(lib, "GLFW/glfw3.lib")

// загрузчик файлов-изображений
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

// математическая библиотека на спецификации GLSL
#include <GLM/glm.hpp>
#include <GLM/ext.hpp>

// API для загрузки 3d-моделей
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment(lib, "assimp/assimp-vc142-mt.lib")

// файлы проекта
#include "Texture/Texture.h"
#include "Shader/Shader.h"
#include "StreamToFile/StreamToFile.h"
#include "Mesh/Mesh.h"
#include "Model/Model.h"
#include "ArcballCamera/ArcballCamera.h"
#include "Quad/Quad.h"
#include "Framebuffer/Framebuffer.h"
