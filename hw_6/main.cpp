#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().

void using_naive_bvh(const std::string& obj) {
    Scene scene(1280, 960);

    MeshTriangle bunny(obj, BVHAccel::SplitMethod::NAIVE);

    scene.Add(&bunny);
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 1));
    scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), 1));
    scene.buildBVH(BVHAccel::SplitMethod::NAIVE);

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene, "navie.ppm");
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
}

void using_sah_bvh(const std::string& obj) {
    Scene scene(1280, 960);

    MeshTriangle bunny(obj, BVHAccel::SplitMethod::SAH);

    scene.Add(&bunny);
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 1));
    scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), 1));
    scene.buildBVH(BVHAccel::SplitMethod::SAH);

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene, "sah.ppm");
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
}

int main(int argc, char** argv)
{
    std::string obj_path = "E:/jdc_code/games101/games101_hw_2021/hw_6/models/bunny/bunny.obj";
    std::cout << "read model " << obj_path << "\n";
    std::cout << "\n\n\n";

    std::cout << "using navie bvh--------------------------> \n";
    using_naive_bvh(obj_path);

    std::cout << "\n\n\n";
    std::cout << "using sah bvh----------------------------> \n";
    using_sah_bvh(obj_path);

    return 0;
}