#include "graphic_interface.h"
#include <GLFW/glfw3.h>

#include "shader_files.h"

const vec2 wind_dir = vec2(1.0f, 1.0f);
const float wind_speed = 1.0f;
const float wave_height = 2.0f;
const float water_size_x = 1000.0f;
const float water_size_z = 1000.0f;
const unsigned int water_num_x = 1000;
const unsigned int water_num_z = 1000;
const float zoom = 90.0f;
const float z_near = 0.1f;
const float z_far = 1000.0f;
const vector<vertex> scr_vertices = {
    {vec3(-1.0f, -1.0f, 0.0f), vec2(0.0f, 0.0f)},
    {vec3(1.0f, -1.0f, 0.0f), vec2(1.0f, 0.0f)},
    {vec3(1.0f, 1.0f, 0.0f), vec2(1.0f, 1.0f)},
    {vec3(-1.0f, 1.0f, 0.0f), vec2(0.0f, 1.0f)}
};
const vector<unsigned int> scr_indices = {
    0, 1, 3,
    1, 2, 3
};
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
const vector<unsigned int> sky_indices = {
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

unsigned int scr_width = 800;
unsigned int scr_height = 600;
vec3 camera_position = vec3(0.0f, 5.0f, 0.0f);
vec3 camera_rotation = vec3(0.0f, 0.0f, 0.0f);
mat4 view;
mat4 projection;

GLuint frame_buffer;
vector<GLuint> frame_attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
GLuint height_spectrumRT;
GLuint displace_spectrumRT_x;
GLuint displace_spectrumRT_z;
GLuint displaceRT;
GLuint water_map;
GLuint fft_pre_shader;
GLuint fft_displacement_shader;
GLuint fft_map_shader;
GLuint scr_mesh_VAO;
GLuint scr_mesh_VBO;
GLuint scr_mesh_EBO;
GLuint scr_mesh_triagnle_size;

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

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
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

    // //fft pre compute
    // glViewport(0, 0, water_size_x, water_size_z);
    // glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    // glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, height_spectrumRT, 0);
    // glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, displace_spectrumRT_x, 0);
    // glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, displace_spectrumRT_z, 0);
    // glDrawBuffers(3, &frame_attachments[0]);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glUseProgram(fft_pre_shader);
    // glDrawElements(GL_TRIANGLES, scr_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);


    while(!glfwWindowShouldClose(window)) {
        processInput(window);

        //fft pre compute
        glViewport(0, 0, water_size_x, water_size_z);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, height_spectrumRT, 0);
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, displace_spectrumRT_x, 0);
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, displace_spectrumRT_z, 0);
        glDrawBuffers(3, &frame_attachments[0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(fft_pre_shader);
        glDrawElements(GL_TRIANGLES, scr_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);

        glViewport(0, 0, water_size_x, water_size_z);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
        //fft displacement compute
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displaceRT, 0);
        glDrawBuffers(1, &frame_attachments[0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(fft_displacement_shader);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(sky_shader, "uHeightSpectrumRT"), 0);
        glBindTexture(GL_TEXTURE_2D, height_spectrumRT);
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(sky_shader, "uDisplaceXSpectrumRT"), 1);
        glBindTexture(GL_TEXTURE_2D, displace_spectrumRT_x);
        glActiveTexture(GL_TEXTURE2);
        glUniform1i(glGetUniformLocation(sky_shader, "uDisplaceZSpectrumRT"), 2);
        glBindTexture(GL_TEXTURE_2D, displace_spectrumRT_z);
        glDrawElements(GL_TRIANGLES, scr_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);

        // fft water map computer
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, water_map, 0);
        glDrawBuffers(1, &frame_attachments[0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(fft_map_shader);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(sky_shader, "uDisplaceRT"), 0);
        glBindTexture(GL_TEXTURE_2D, displaceRT);
        glDrawElements(GL_TRIANGLES, scr_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);

        glViewport(0, 0, scr_width, scr_height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(sky_shader, "uSky"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky_cubemap);
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(sky_shader, "uMap"), 1);
        glBindTexture(GL_TEXTURE_2D, water_map);
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
    // create fft
    glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    create_empty_texture(water_size_x, water_size_z, height_spectrumRT);
    create_empty_texture(water_size_x, water_size_z, displace_spectrumRT_x);
    create_empty_texture(water_size_x, water_size_z, displace_spectrumRT_z);
    create_empty_texture(water_size_x, water_size_z, displaceRT);
    create_empty_texture(water_size_x, water_size_z, water_map);
    create_shader(fft_pre_vertex_shader, fft_pre_fragment_shader, fft_pre_shader);
    create_shader(fft_displacement_vertex_shader, fft_displacement_fragment_shader, fft_displacement_shader);
    create_shader(fft_map_vertex_shader, fft_map_fragment_shader, fft_map_shader);
    create_vertex_buffer(scr_vertices, scr_indices, scr_mesh_VAO, scr_mesh_VBO, scr_mesh_EBO);
    scr_mesh_triagnle_size = scr_indices.size() / 3; 

    // create sky
    create_shader(sky_vertex_shader, sky_fragment_shader, sky_shader);
    create_cubemap(sky_images, sky_cubemap);
    create_vertex_buffer(sky_vertices, sky_indices, sky_mesh_VAO, sky_mesh_VBO, sky_mesh_EBO);
    sky_mesh_triagnle_size = sky_indices.size() / 3;

    // create water
    create_shader(water_vertex_shader, water_fragment_shader, water_shader);
    vector<vertex> water_vertices;
    vector<unsigned int> water_indices;
    generate_plane(water_size_x, water_size_z, water_num_x, water_num_z, water_vertices, water_indices);
    create_vertex_buffer(water_vertices, water_indices, water_mesh_VAO, water_mesh_VBO, water_mesh_EBO);
    water_mesh_triagnle_size = water_indices.size() / 3;
}