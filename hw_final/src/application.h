#ifndef CGL_APPLICATION_H
#define CGL_APPLICATION_H

// STL
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// libCGL
#include "CGL/CGL.h"
#include "CGL/osdtext.h"
#include "CGL/renderer.h"

#include "CGL/matrix4x4.h"

using namespace std;

namespace CGL {

struct AppConfig {
  //POSITIVE_X, NEGATIVE_X, POSITIVE_Y, NEGATIVE_Y, POSITIVE_Z, NEGATIVE_Z
  string sky_images[6];
  float water_size_x;
  float water_size_z;
  float water_height;
};

class Application : public Renderer {
public:
  Application(AppConfig config);
  ~Application();

  void init();
  void render();
  void resize(size_t w, size_t h);

  std::string name();
  std::string info();

  void keyboard_event(int key, int event, unsigned char mods);
  // void cursor_event(float x, float y);
  // void scroll_event(float offset_x, float offset_y);
  // void mouse_event(int key, int event, unsigned char mods);

private:
  AppConfig config;

  size_t screen_width;
  size_t screen_height;

  Matrix4x4 projection;
  Matrix4x4 view;

  GLuint sky_shader;
  GLuint sky_cubemap;
  GLuint sky_cube;
  GLuint sky_mesh_VAO;
  GLuint sky_mesh_VBO;
  GLuint sky_mesh_EBO;
  GLuint sky_mesh_triagnle_size;
  
  GLuint water_shader;
  GLuint water_mesh_VAO;
  GLuint water_mesh_VBO;
  GLuint water_mesh_EBO;
  GLuint water_mesh_triagnle_size;
}; // class Application

} // namespace CGL

#endif // CGL_APPLICATION_H
