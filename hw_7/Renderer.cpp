//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <thread>
#include <mutex>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

Scene* now_scene = nullptr;
std::vector<Vector3f> framebuffer;
std::mutex mutex;
int column_count = 0;

void threadfunc(
    int spp,
    float scale,
    float imageAspectRatio,
    Vector3f eye_pos,
    Vector2f start, 
    Vector2f end) {
    for (uint32_t i = start.x; i < end.x; ++i) {
        for (uint32_t j = start.y; j < end.y; ++j) {
            int m = j * now_scene->width + i;
            float x = (2 * (i + 0.5) / (float)now_scene->width - 1) *
                imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)now_scene->height) * scale;

            Vector3f dir = normalize(Vector3f(-x, y, 1));
            for (int k = 0; k < spp; k++) {
                framebuffer[m] += now_scene->castRay(Ray(eye_pos, dir), 0) / spp;
            }
        }
        mutex.lock();
        UpdateProgress(++column_count / (float)now_scene->height);
        mutex.unlock();
    }
}

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene, int thread)
{
    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);

    // change the spp value to change sample ammount
    int spp = 8;
    std::cout << "SPP: " << spp << "\n";
    std::cout << "thread count: " << thread << "\n";

    now_scene = const_cast<Scene*>(&scene);
    framebuffer.resize(scene.width * scene.height);

    std::vector<std::thread> tread_pool;
    for (int i = 0; i < thread; i++) {
        int start_x = i * scene.width / thread;
        int end_x = (i + 1) * scene.width / thread;
        if (i == thread - 1) end_x = scene.width;

        tread_pool.push_back(std::thread(threadfunc,
            spp,
            scale,
            imageAspectRatio,
            eye_pos,
            Vector2f(float(start_x), 0.0f),
            Vector2f(float(end_x), float(scene.height))));
    }

    for (auto& t : tread_pool) t.join();

    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
