#include "graphic_interface.h"
#include <GLFW/glfw3.h>

#include "shader_files.h"

#define PI 3.1415926
#define G 9.81

const vec2 wind = vec2(10.0f, -10.0f);
const float wave_height = 2.0f;
const float water_size = 500.0f;
const int water_vertices_num = 512;

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

float n_time = 0.0f;
unsigned int scr_width = 800;
unsigned int scr_height = 600;
vec3 camera_position = vec3(0.0f, 5.0f, 0.0f);
vec3 camera_rotation = vec3(0.0f, 0.0f, 0.0f);
mat4 view;
mat4 projection;

int fft_steps = 0;
GLuint fft_ht_shader;
GLuint fft_displacement_shader;
GLuint fft_normal_shader;
GLuint h0_texture;
GLuint ht_texture;
GLuint displacement_textures[2];
GLuint normal_texture;
GLuint butterfly_indices_texture;

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

    clock_t start, end;
    while(!glfwWindowShouldClose(window)) {
        start = clock();
        processInput(window);

        // ht
        glUseProgram(fft_ht_shader);
        glBindImageTexture(0, h0_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	    glBindImageTexture(1, ht_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        glUniform1f(glGetUniformLocation(fft_ht_shader, "u_totalTime"), n_time);
        glDispatchCompute(water_vertices_num, water_vertices_num, 1);
	    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // ifft
        glUseProgram(fft_displacement_shader);
	    glBindImageTexture(0, ht_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	    glBindImageTexture(1, displacement_textures[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	    glBindImageTexture(2, butterfly_indices_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	    glUniform1i(glGetUniformLocation(fft_displacement_shader, "u_processColumn"), 0);
	    glUniform1i(glGetUniformLocation(fft_displacement_shader, "u_steps"), fft_steps);
        glDispatchCompute(1, water_vertices_num, 1);
	    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(0, displacement_textures[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        glBindImageTexture(1, displacement_textures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        glUniform1i(glGetUniformLocation(fft_displacement_shader, "u_processColumn"), 1);
        glDispatchCompute(1, water_vertices_num, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // normal
        glUseProgram(fft_normal_shader);
        glBindImageTexture(0, displacement_textures[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        glBindImageTexture(1, normal_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        glDispatchCompute(water_vertices_num, water_vertices_num, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

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
        glBindTexture(GL_TEXTURE_2D, normal_texture);
        glBindVertexArray(water_mesh_VAO);
        glDrawElements(GL_TRIANGLES, water_mesh_triagnle_size * 3, GL_UNSIGNED_INT, 0);


        glfwSwapBuffers(window);
        glfwPollEvents();
        end = clock();
		n_time = float(end - start) / 1000.0f;
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

GLfloat rand(const GLfloat start, const GLfloat end) {
	return ((GLfloat)rand() / (GLfloat)RAND_MAX) * (end - start) + start;
}
 
GLfloat rand_normal(const GLfloat mean, const GLfloat standardDeviation) {
	GLfloat x1, x2;
	x1 = rand(0.000001f, 1.0f - 0.000001f);
	x2 = rand(0.0f, 1.0f);
 
	return mean + standardDeviation * (sqrtf(-2.0f * logf(x1)) * cosf(2.0f * PI * x2));
}
 
GLfloat phillips(GLfloat A, GLfloat L, glm::vec2 waveDirection, glm::vec2 windDirection) {
	GLfloat k = glm::length(waveDirection);
	GLfloat waveDotWind = glm::dot(waveDirection, windDirection);
 
	if (L == 0.0f || k == 0.0f) return 0.0f;
	return A * expf(-1.0f / (k * L * k * L)) / (k * k * k * k) * waveDotWind * waveDotWind;
}

void butterfly_shuffle_indices(GLfloat* indices, int num) {
    struct butterfly_shuffle_item{
        vector<GLfloat> indices;
        butterfly_shuffle_item* left = nullptr;
        butterfly_shuffle_item* right = nullptr;

        butterfly_shuffle_item() {}
        ~butterfly_shuffle_item() {
            if (left) delete left;
            if (right) delete right;
        }
        static void split(butterfly_shuffle_item* n_item) {
            if (n_item->indices.size() <= 2) return;

            n_item->left = new butterfly_shuffle_item();
            n_item->right = new butterfly_shuffle_item();
            for (int i = 0; i < n_item->indices.size(); i++) {
                if (i % 2 == 0) n_item->left->indices.push_back(n_item->indices[i]);
                else n_item->right->indices.push_back(n_item->indices[i]);
            }
            split(n_item->left);
            split(n_item->right);
        }
        static void tranverse(butterfly_shuffle_item* n_item, vector<GLfloat>& indices) {
            if (n_item->indices.size() <= 2) {
                indices.push_back(n_item->indices[0]);
                indices.push_back(n_item->indices[1]);
            }
            else {
                tranverse(n_item->left, indices);
                tranverse(n_item->right, indices);
            }
        }
    };
    butterfly_shuffle_item* n_item = new butterfly_shuffle_item();
    for (int i = 0; i < num; i++) n_item->indices.push_back(indices[i]);
    butterfly_shuffle_item::split(n_item);
    vector<GLfloat> buffer;
    butterfly_shuffle_item::tranverse(n_item, buffer); 
    for (int i = 0; i < num; i++) indices[i] = buffer[i];
    delete n_item;
}

void create_resource(void) {
    // create fft
	GLint temp = water_vertices_num;
	while (!(temp & 0x1)) {
		temp = temp >> 1;
		fft_steps++;
	}
    create_shader(fft_ht_computer_shader, fft_ht_shader);
    create_shader(fft_displacement_computer_shader, fft_displacement_shader);
    create_shader(fft_normal_computer_shader, fft_normal_shader);

    GLfloat* h0_data = (GLfloat*)malloc(water_vertices_num * water_vertices_num * 2 * sizeof(GLfloat));
    vec2 wind_dir = normalize(wind);
    float wind_speed = length(wind);
    vec2 wave_dir;
    for (int i = 0; i < water_vertices_num; i++) {
		wave_dir.y = ((GLfloat)i - (GLfloat)water_vertices_num / 2.0f) * (2.0f * PI / water_size);
		for (int j = 0; j < water_vertices_num; j++)
		{
			wave_dir.x = ((GLfloat)j - (GLfloat)water_vertices_num / 2.0f) * (2.0f * PI / water_size);
			float phillips_value = phillips(wave_height, wind_speed * wind_speed / G, wave_dir, wind_dir);
			h0_data[i * 2 * water_vertices_num + j * 2 + 0] = 1.0f / sqrtf(2.0f) * rand_normal(0.0f, 1.0f) * phillips_value;
			h0_data[i * 2 * water_vertices_num + j * 2 + 1] = 1.0f / sqrtf(2.0f) * rand_normal(0.0f, 1.0f) * phillips_value;
		}
	}
    create_texture_2d(water_vertices_num, water_vertices_num, h0_texture, GL_RG, h0_data);
    free(h0_data);
    create_texture_2d(water_vertices_num, water_vertices_num, ht_texture, GL_RG);
    create_texture_2d(water_vertices_num, water_vertices_num, displacement_textures[0], GL_RG);
    create_texture_2d(water_vertices_num, water_vertices_num, displacement_textures[1], GL_RG);
    create_texture_2d(water_vertices_num, water_vertices_num, normal_texture, GL_RGBA);

    GLfloat* butterfly_indices_data = (GLfloat*)malloc(water_vertices_num * sizeof(GLfloat));
	for (int i = 0; i < water_vertices_num; i++) butterfly_indices_data[i] = i;
    butterfly_shuffle_indices(butterfly_indices_data, water_vertices_num);
    create_texture_1d(water_vertices_num, butterfly_indices_texture, GL_RED, butterfly_indices_data);
    free(butterfly_indices_data);


    // create sky
    create_shader(sky_vertex_shader, sky_fragment_shader, sky_shader);
    create_cubemap(sky_images, sky_cubemap);
    create_vertex_buffer(sky_vertices, sky_indices, sky_mesh_VAO, sky_mesh_VBO, sky_mesh_EBO);
    sky_mesh_triagnle_size = sky_indices.size() / 3;

    // create water
    create_shader(water_vertex_shader, water_fragment_shader, water_shader);
    vector<vertex> water_vertices;
    vector<unsigned int> water_indices;
    generate_plane(water_size, water_size, water_vertices_num, water_vertices_num, water_vertices, water_indices);
    create_vertex_buffer(water_vertices, water_indices, water_mesh_VAO, water_mesh_VBO, water_mesh_EBO);
    water_mesh_triagnle_size = water_indices.size() / 3;
}