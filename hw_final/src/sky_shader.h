#ifndef FFT_SHADER_H
#define FFT_SHADER_H

#include <string>

using namespace std;

namespace CGL {
    string sky_vertex_shader = 
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPosition;\n"
    "out vec3 tex_coord;\n"
    "uniform mat4 uProjection;\n"
    "uniform mat4 uView;\n"
    "void main() {\n"
    "tex_coord = normalize(aPosition);\n"
    "gl_Position = uProjection * uView * vec4(aPosition, 1.0f);\n"
    "}\n";
    string sky_fragment_shader = 
    "#version 330 core\n"
    "in vec3 tex_coord;\n"
    "uniform samplerCube uSky;\n"
    "layout(location = 0) out vec3 fColor;\n"
    "void main() {\n"
    "fColor = texture(uSky, normalize(tex_coord)).rgb;\n"
    "}\n";
}
#endif