#include <iostream>

#include "application.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "sky_shader.h"

namespace CGL {
inline void check_compile_errors(
  GLuint id,
  const string& type,
  const string& code) {
  GLint success;
  GLchar info_log[1024];

  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (!success) {
      glGetShaderInfoLog(id, 1024, NULL, info_log);
      cout <<
          "graphic_interface::gl3plus_shader:compile error\n"
          + type + "------------>\n"
          + code + +"\n"
          + "=====================\n"
          + "info: " + info_log + "\n";
  }
}

    inline void check_link_errors(
        GLuint id, 
        const string& v_code, 
        const string& f_code) {
        GLint success;
        GLchar info_log[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(id, 1024, NULL, info_log);
            cout << 
                "graphic_interface::gl3plus_shader:link error\nVERTEX------------>\n"
                + v_code + "\n"
                + "FRAGMENT------------>\n"
                + f_code + "\n"
                + "=====================\n"
                + "info: " + info_log + "\n";
        }
    }

void create_shader(const string& vertex_code, const string& fragment_code, GLuint& shader) {
  unsigned int v_id;
  const char* v_code = vertex_code.c_str();
  v_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(v_id, 1, &v_code, NULL);
  glCompileShader(v_id);
  check_compile_errors(v_id, "VERTEX", vertex_code);

  unsigned int f_id;
  const char* f_code = fragment_code.c_str();
  f_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(f_id, 1, &f_code, NULL);
  glCompileShader(f_id);
  check_compile_errors(f_id, "FRAGMENT", fragment_code);

  shader = glCreateProgram();
  glAttachShader(shader, v_id);
  glAttachShader(shader, f_id);
  glLinkProgram(shader);
  check_link_errors(
      shader,
      vertex_code,
      fragment_code);

  glDeleteShader(v_id);
  glDeleteShader(f_id);
}

struct vertex {
  Vector3D position;
  Vector2D tex_coord;
  Vector3D normal;
  Vector3D tangent;
};

void create_vertex_buffer(const vector<vertex>& vertices, const vector<unsigned int>& indices, GLuint& VAO, GLuint& VBO, GLuint& EBO) {
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(vertex), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tex_coord));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_DOUBLE, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_DOUBLE, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tangent));
}

Application::Application(AppConfig config) { this->config = config; }

Application::~Application() {
  glDeleteTextures(1, &sky_cubemap);
  glDeleteBuffers(1, &sky_mesh_VBO);
  glDeleteBuffers(1, &sky_mesh_EBO);
  glDeleteVertexArrays(1, &sky_mesh_VAO);
  glDeleteShader(sky_shader);

  glDeleteBuffers(1, &water_mesh_VBO);
  glDeleteBuffers(1, &water_mesh_EBO);
  glDeleteVertexArrays(1, &water_mesh_VAO);
  glDeleteShader(water_shader);
}

void Application::init() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  // enable anti-aliasing
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

  // create sky cubemap
  glGenTextures(1, &sky_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sky_cubemap);
  for (int i = 0; i < 6; i++) {
		int component, width, height;
		void* sky_image_data = stbi_load(config.sky_images[i].c_str(), &width, &height, &component, 3);

    glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB8,
			width,
			height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			sky_image_data);

    stbi_image_free(sky_image_data);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  vector<vertex> sky_vertices = {
    //front
    {Vector3D(-1.0f, -1.0f, -1.0f)},
    {Vector3D(1.0f, -1.0f, -1.0f)},
    {Vector3D(1.0f,  1.0f, -1.0f)},
    {Vector3D(-1.0f,  1.0f, -1.0f)},
    //back
    {Vector3D(1.0f, -1.0f,  1.0f)},
    {Vector3D(-1.0f, -1.0f,  1.0f)},
    {Vector3D(-1.0f,  1.0f,  1.0f)},
    {Vector3D(1.0f,  1.0f,  1.0f)},
    //left
    {Vector3D(-1.0f, -1.0f,  1.0f)},
    {Vector3D(-1.0f, -1.0f, -1.0f)},
    {Vector3D(-1.0f,  1.0f, -1.0f)},
    {Vector3D(-1.0f,  1.0f,  1.0f)},
    //right
    {Vector3D(1.0f, -1.0f, -1.0f)},
    {Vector3D(1.0f, -1.0f,  1.0f)},
    {Vector3D(1.0f,  1.0f,  1.0f)},
    {Vector3D(1.0f,  1.0f, -1.0f)},
    //up
    {Vector3D(-1.0f,  1.0f, -1.0f)},
    {Vector3D(1.0f,  1.0f, -1.0f)},
    {Vector3D(1.0f,  1.0f,  1.0f)},
    {Vector3D(-1.0f,  1.0f,  1.0f)},
    //down
    {Vector3D(-1.0f, -1.0f,  1.0f)},
    {Vector3D(1.0f, -1.0f,  1.0f)},
    {Vector3D(1.0f, -1.0f, -1.0f)},
    {Vector3D(-1.0f, -1.0f, -1.0f)}
};
  vector<unsigned int> sky_indices = {
    //front
    0, 1, 3,
    1, 2, 3,
    //back
    4, 5, 7,
    5, 6, 7,
    //left
    8, 9, 11,
    9, 10, 11,
    //right
    12, 13, 15,
    13, 14, 15,
    //up
    16, 17, 19,
    17, 18, 19,
    //down
    20, 21, 23,
    21, 22, 23
};
  
  create_vertex_buffer(sky_vertices, sky_indices, sky_mesh_VAO, sky_mesh_VBO, sky_mesh_EBO);
  sky_mesh_triagnle_size = sky_indices.size() / 3;

  create_shader(sky_vertex_shader, sky_fragment_shader, sky_shader);

  // create water mesh
  float half_x = config.water_size_x / 2.0f;
  float half_z = config.water_size_z / 2.0f;
  Vector3D normal(0.0f, 1.0f, 0.0f);
  Vector3D tangent(1.0f, 0.0f, 0.0f);
  vector<vertex> water_vertices = {
    {Vector3D(-1.0f * half_x, 0.0f, half_z), Vector2D(0.0f, 0.0f), normal, tangent},
    {Vector3D(half_x, 0.0f, half_z), Vector2D(1.0f, 0.0f), normal, tangent},
    {Vector3D(half_x, 0.0f, -1.0f * half_z), Vector2D(1.0f, 1.0f), normal, tangent},
    {Vector3D(-1.0f * half_x, 0.0f, -1.0f * half_z), Vector2D(0.0f, 1.0f), normal, tangent}
  };
  vector<unsigned int> water_indices = {0, 1, 2, 0, 2, 3};

  create_vertex_buffer(water_vertices, water_indices, water_mesh_VAO, water_mesh_VBO, water_mesh_EBO);
  water_mesh_triagnle_size = water_indices.size() / 3;
}

void Application::render() { 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(sky_shader);
  glUniformMatrix4dv(glGetUniformLocation(sky_shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
  glUniformMatrix4dv(glGetUniformLocation(sky_shader, "uView"), 1, GL_FALSE, &view[0][0]);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(sky_shader, "uSky"), 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, sky_cubemap);

  glBindVertexArray(sky_mesh_VAO);
  glDrawElements(GL_TRIANGLES, sky_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);
  
  glFlush(); 
}

void Application::resize(size_t w, size_t h) {
  glViewport(0, 0, screen_width, screen_width);
}

void Application::keyboard_event(int key, int event, unsigned char mods) {
  switch (key) {
  case 'a':
    break;
  case 'w':
    break;
  case 's':
    break;
  case 'd':
    break;
  }
}

string Application::name() { return "fft water simulation"; }

string Application::info() {
  return "";
}
}
