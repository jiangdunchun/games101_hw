#include "graphic_interface.h"
#include <GLFW/glfw3.h>

#include "shader_files.h"

const vector<string> sky_images = {
    "assert/right.jpg",
    "assert/left.jpg",
    "assert/top.jpg",
    "assert/bottom.jpg",
    "assert/front.jpg",
    "assert/back.jpg"};
const vector<vertex> sky_vertices = {
    //front
    {vec3(-1.0f, -1.0f, -1.0f)},
    {vec3(1.0f, -1.0f, -1.0f)},
    {vec3(1.0f,  1.0f, -1.0f)},
    {vec3(-1.0f,  1.0f, -1.0f)},
    //back
    {vec3(1.0f, -1.0f,  1.0f)},
    {vec3(-1.0f, -1.0f,  1.0f)},
    {vec3(-1.0f,  1.0f,  1.0f)},
    {vec3(1.0f,  1.0f,  1.0f)},
    //left
    {vec3(-1.0f, -1.0f,  1.0f)},
    {vec3(-1.0f, -1.0f, -1.0f)},
    {vec3(-1.0f,  1.0f, -1.0f)},
    {vec3(-1.0f,  1.0f,  1.0f)},
    //right
    {vec3(1.0f, -1.0f, -1.0f)},
    {vec3(1.0f, -1.0f,  1.0f)},
    {vec3(1.0f,  1.0f,  1.0f)},
    {vec3(1.0f,  1.0f, -1.0f)},
    //up
    {vec3(-1.0f,  1.0f, -1.0f)},
    {vec3(1.0f,  1.0f, -1.0f)},
    {vec3(1.0f,  1.0f,  1.0f)},
    {vec3(-1.0f,  1.0f,  1.0f)},
    //down
    {vec3(-1.0f, -1.0f,  1.0f)},
    {vec3(1.0f, -1.0f,  1.0f)},
    {vec3(1.0f, -1.0f, -1.0f)},
    {vec3(-1.0f, -1.0f, -1.0f)}};
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
    21, 22, 23};
const float zoom = 90.0f;
const float z_near = 0.1f;
const float z_far = 1000.0f;

unsigned int scr_width = 800;
unsigned int scr_height = 600;
vec3 camera_position = vec3(0.0f, 5.0f, 0.0f);
vec3 camera_rotation = vec3(0.0f, 0.0f, 0.0f);
mat4 view;
mat4 projection;

GLuint sky_shader;
GLuint sky_cubemap;
GLuint sky_mesh_VAO;
GLuint sky_mesh_VBO;
GLuint sky_mesh_EBO;
GLuint sky_mesh_triagnle_size;
  
GLuint water_shader;
GLuint water_mesh_VAO;
GLuint water_mesh_VBO;
GLuint water_mesh_EBO;
GLuint water_mesh_triagnle_size;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);//回调函数原型声明
void processInput(GLFWwindow *window);
void create_resource(void);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "FFT water simulation", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }
    create_resource();

    
    construct_projection(zoom, float(scr_height)/ scr_width, z_near, z_far, projection);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(!glfwWindowShouldClose(window)) {
        processInput(window);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, scr_width, scr_height);
        // draw sky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        construct_view(vec3(0.0f, 0.0f, 0.0f), camera_rotation, view);
        glUseProgram(sky_shader);
        glUniformMatrix4fv(glGetUniformLocation(sky_shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(sky_shader, "uView"), 1, GL_FALSE, &view[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(sky_shader, "uSky"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky_cubemap);
        glBindVertexArray(sky_mesh_VAO);
        glDrawElements(GL_TRIANGLES, sky_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);

        // draw water
        glClear(GL_DEPTH_BUFFER_BIT);
        construct_view(camera_position, camera_rotation, view);
        glUseProgram(water_shader);
        glUniformMatrix4fv(glGetUniformLocation(sky_shader, "uProjection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(sky_shader, "uView"), 1, GL_FALSE, &view[0][0]);
        glBindVertexArray(water_mesh_VAO);
        glDrawElements(GL_TRIANGLES, water_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 1;
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    scr_width = width;
    scr_height = height;

    construct_projection(zoom, float(height)/ width, z_near, z_far, projection);
}

void create_resource(void) {
    // create sky
    create_shader(sky_vertex_shader, sky_fragment_shader, sky_shader);
    create_cubemap(sky_images, sky_cubemap);
    create_vertex_buffer(sky_vertices, sky_indices, sky_mesh_VAO, sky_mesh_VBO, sky_mesh_EBO);
    sky_mesh_triagnle_size = sky_indices.size() / 3;

    // create water
    create_shader(water_vertex_shader, water_fragment_shader, water_shader);
    vector<vertex> water_vertices;
    vector<unsigned int> water_indices;
    generate_plane(1000.0f, 1000.0f, 1000, 1000, water_vertices, water_indices);
    create_vertex_buffer(water_vertices, water_indices, water_mesh_VAO, water_mesh_VBO, water_mesh_EBO);
    water_mesh_triagnle_size = water_indices.size() / 3;
}