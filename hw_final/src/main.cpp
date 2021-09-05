#include "CGL/CGL.h"
#include "CGL/viewer.h"

#include "application.h"
typedef uint32_t gid_t;

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace CGL;

int main(int argc, char **argv) {
  AppConfig config;
  config.sky_images[0] = "assert/right.jpg";
  config.sky_images[1] = "assert/left.jpg";
  config.sky_images[2] = "assert/top.jpg";
  config.sky_images[3] = "assert/bottom.jpg";
  config.sky_images[4] = "assert/front.jpg";
  config.sky_images[5] = "assert/back.jpg";
  config.water_size_x = 1000.0f;
  config.water_size_z = 1000.0f;
  config.water_height = -10.0f;

  // create application
  Application *app = new Application(config);

  // create viewer
  Viewer viewer = Viewer();

  // set renderer
  viewer.set_renderer(app);

  // init viewer
  viewer.init();

  // start viewer
  viewer.start();

  return 0;
}
