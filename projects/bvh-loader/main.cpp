
#include <stdio.h>
#include <iostream>

#include <functional>

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include "unistd.h"

#include "shader.h"

#include "bvh/bvh.h"

#include "camera.h"

#include "meshdata.h"
#include "meshgraphics.h"
#include "meshloader.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"

#include "glcheck.h"

#define BUFFER_OFFSET(offset) ((void *)(offset))

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
    exit(0);
  }
}

int main() {
  GLFWwindow* window = nullptr;

  if (!glfwInit()) {
    fprintf( stderr, "ERROR: could not start GLFW3\n" );
    return 1;
  }

#if APPLE
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(640, 480, "Hello world", nullptr, nullptr);

  if (!window) {
    fprintf( stderr, "ERROR: could not open window with GLFW3\n" );
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  glewInit();

  glfwSetKeyCallback(window, key_callback);

  const GLubyte* renderer;
  const GLubyte* version;
  /* get version info */
  renderer = glGetString( GL_RENDERER ); /* get renderer string */
  version = glGetString( GL_VERSION );	 /* version as a string */
  printf( "Renderer: %s\n", renderer );
  printf( "OpenGL version supported %s\n", version );

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glDepthFunc(GL_LESS);

  int width = 800;
  int height = 600;

  ilInit();
  iluInit();
  ilutInit();
  ilutRenderer(ILUT_OPENGL);

//  GLuint texture;
//  glGenTextures(1, &texture);
//  glBindTexture(GL_TEXTURE_2D, texture);
//  glActiveTexture(GL_TEXTURE0);

  k::Camera camera(width, height);
  glm::mat4 Model = glm::mat4(1.0f);
  glm::mat4 mvp = camera.matrix() * Model;

  k::MeshData meshData("shader4");
  k::MeshGraphics meshGraphics;

  GLfloat* g_uv_buffer_data = new GLfloat[9] {
      0.0f, 0.0f,  0.0f, 1.0f,   1.0f, 1.0f
  };

  {
    GLuint* cube_elements = new GLuint[3] {
      0, 1, 2
    };

    GLfloat* g_vertex_buffer_data = new GLfloat[9] {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,
    };

    k::LoadedMeshData loadedData;
    loadedData.elements = cube_elements;
    loadedData.lenElements = 3;
    loadedData.lenVertexBufferData = 9;
    loadedData.vertexBufferData = g_vertex_buffer_data;

    meshGraphics.loadMeshData(meshData, loadedData);

    delete[] cube_elements;
    delete[] g_vertex_buffer_data;
  }

  k::Shader& shader = meshData.shader();
  meshGraphics.loadVerticesToShader(meshData, "vPosition");





  k::GLMeshData& glMeshData = meshData.data();
  glBindVertexArray(glMeshData.vao);
  GLuint uvbuf;
  glGenBuffers(1, &uvbuf);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuf);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(GLfloat) * 6, g_uv_buffer_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  shader.use();
  GLint uvPos = glGetAttribLocation(shader.getProgram(), "uvPos");
  std::cout << shader.getProgram() << "vertexuv pos" << uvPos << std::endl;

  glVertexAttribPointer(uvPos, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
  glEnableVertexAttribArray(uvPos);
  glBindVertexArray(0);



  ILuint ImageName;
  ilGenImages(1, &ImageName);
  ilBindImage(ImageName);
  ilLoadImage("../data/images/texture.png");
//  ILubyte *Data = ilGetData();
  ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  std::cout << "format 1" << (ilGetInteger(IL_IMAGE_FORMAT) == GL_RGB) << std::endl;
  std::cout << "type 1" << (ilGetInteger(IL_IMAGE_TYPE) == GL_UNSIGNED_BYTE) << std::endl;
//  glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), width, height, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, ilGetInteger(IL_IMAGE_TYPE), ilGetData());
  ilDeleteImage(ImageName);
  GLuint samplerId = glGetUniformLocation(shader.getProgram(), "tex");
  std::cout << "sampler id" << samplerId << std::endl;
  glBindTexture(GL_TEXTURE_2D, 0);

  shader.use();
  GLuint MatrixID = shader.uniform("mvp");
  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(mvp));
  shader.unuse();

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    GLuint MatrixID = shader.uniform("mvp");
    glm::mat4 mvp_ = camera.matrix() * glm::mat4(1.0f);
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(mvp_));

    meshGraphics.renderMesh(meshData);
    shader.unuse();

    camera.move(glm::vec3(-0.01f, 0.0f, 0.0f));

    glfwSwapBuffers( window );
    glfwPollEvents();
  }

  meshGraphics.cleanupMeshData(meshData);

  return 0;
}
