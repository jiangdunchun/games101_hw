#ifndef __GRAPHIC_INTERFACE_H_
#define __GRAPHIC_INTERFACE_H_

#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace glm;


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
  vec3 position;
  vec3 tex_coord;
  vec3 normal;
  vec3 tangent;
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tex_coord));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tangent));
}

void create_cubemap(const vector<string>& images, GLuint& cubemap) {
    glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    for (int i = 0; i < 6; i++) {
        int component, width, height;
        void* sky_image_data = stbi_load(images[i].c_str(), &width, &height, &component, 3);

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
}

void construct_projection(float zoom, float aspect, float near, float far, mat4& projection) {
    projection = perspective(radians(zoom), aspect, near, far);
}

void construct_view(vec3 position, vec3 rotation, mat4& view) {
    mat4 transform = mat4(1.0f);
    transform = rotate(transform, radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
    transform = rotate(transform, radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
    transform = rotate(transform, radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));

    vec4 direct = transform * vec4(0.0f, 0.0f, -1.0f, 0.0f);
    vec4 up = transform * vec4(0.0f, 1.0f, 0.0f, 0.0f);

    view = lookAt(position, position + normalize(vec3(direct.x, direct.y, direct.z)), normalize(vec3(up.x, up.y, up.z)));
}
#endif