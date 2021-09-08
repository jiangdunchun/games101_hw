#ifndef SHADER_FILES_H
#define SHADER_FILES_H

#include <string>

using namespace std;

string fft_pre_vertex_shader = R"delimiter(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTex_coord;

out vec2 tex_coord;

void main() {
    tex_coord = aTex_coord;
    gl_Position = vec4(aPosition, 1.0f);
}
)delimiter";

string fft_pre_fragment_shader = R"delimiter(
#version 330 core
in vec2 tex_coord;

layout(location = 0) out vec3 fHeightSpectrumRT;
layout(location = 1) out vec3 fDisplaceXSpectrumRT;
layout(location = 2) out vec3 fDisplaceZSpectrumRT;

void main() {
    
}
)delimiter";

string fft_displacement_vertex_shader = R"delimiter(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTex_coord;

out vec2 tex_coord;

void main() {
    tex_coord = aTex_coord;
    gl_Position = vec4(aPosition, 1.0f);
}
)delimiter";

string fft_displacement_fragment_shader = R"delimiter(
#version 330 core
in vec2 tex_coord;

uniform sampler2D uHeightSpectrumRT;
uniform sampler2D uDisplaceXSpectrumRT;
uniform sampler2D uDisplaceZSpectrumRT;

layout(location = 0) out vec3 fDisplaceRT;

void main() {
    
}
)delimiter";

string fft_map_vertex_shader = R"delimiter(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTex_coord;

out vec2 tex_coord;

void main() {
    tex_coord = aTex_coord;
    gl_Position = vec4(aPosition, 1.0f);
}
)delimiter";

string fft_map_fragment_shader = R"delimiter(
#version 330 core
in vec2 tex_coord;

uniform sampler2D uDisplaceRT;

layout(location = 0) out vec4 fMap;

void main() {
    
}
)delimiter";

string sky_vertex_shader = R"delimiter(
#version 330 core
layout(location = 0) in vec3 aPosition;

uniform mat4 uProjection;
uniform mat4 uView;

out vec3 tex_coord;

void main() {
    tex_coord = normalize(aPosition);
    gl_Position = uProjection * uView * vec4(aPosition, 1.0f);
}
)delimiter";

string sky_fragment_shader = R"delimiter(
#version 330 core
in vec3 tex_coord;

uniform samplerCube uSky;

out vec3 fColor;

void main() {
    fColor = texture(uSky, normalize(tex_coord)).rgb;
}
)delimiter";

string water_vertex_shader = R"delimiter(
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTex_coord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;

uniform mat4 uProjection;
uniform mat4 uView;

out vec3 position;
out vec2 tex_coord;
out vec3 normal;
out mat3 TBN;

void main() {
    position = aPosition;
    tex_coord = aTex_coord;
    normal = aNormal;
    vec3 B = normalize(cross(aNormal,aTangent));
    TBN = transpose(mat3(aTangent, B, aNormal));

    gl_Position = uProjection * uView * vec4(aPosition, 1.0f);
}
)delimiter";

    
string water_fragment_shader = R"delimiter(
#version 330 core
in vec3 position;
in vec2 tex_coord;
in vec3 normal;
in mat3 TBN;

uniform samplerCube uSky;
uniform sampler2D uMap;

out vec3 fColor;

void main() {
    fColor = vec3(0.5f, 0.5f, 0.5f);
}
)delimiter";

#endif