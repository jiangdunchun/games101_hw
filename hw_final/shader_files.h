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
#define PI 3.1415926
#define G 9.81

in vec2 tex_coord;

uniform vec2 uWind;
uniform float uWave_height;

layout(location = 0) out vec4 fH_tilde_0_conj0 ;

float rand_gaussian(float n){ 
    return fract(sin(mod(dot(n ,12.9898),3.14))*43758.5453); 
} 

float phillips(vec2 k) {
    float k_length = length(k);
    k_length = max(0.001f, k_length);
    float k_length2 = k_length * k_length;
    float k_length4 = k_length2 * k_length2;

    float wind_speed = length(uWind);
    float  l = wind_speed * wind_speed / G;
    float l2 = l * l;

    float damping = 0.001f;
    float L2 = l2 * damping * damping;

    return uWave_height * exp(-1.0f / (k_length2 * l2)) / k_length4 * exp(-1.0f * k_length2 * L2);
}

// Donelan-Banner
float directional_spreading(vec2 k) {
    float beta_s;
    float omega_p = 0.855f * G / length(uWind);
    float ratio = sqrt(G * length(k)) / omega_p;

    if (ratio < 0.95f) beta_s = 2.61f * pow(ratio, 1.3f);
    else if(ratio >= 0.95f && ratio < 1.6f) beta_s = 2.28f * pow(ratio, -1.3f);
    else if(ratio > 1.6f) beta_s = pow(10, (-0.4f + 0.8393f * exp(-0.567f * log(ratio * ratio))));

    float theta = atan(k.y, k.x) - atan(uWind.y, uWind.x);

    return beta_s / max(0.0001f, 2.0f * tanh(beta_s * PI) * pow(cosh(beta_s * theta), 2));
}

void main() {
    vec2 k = PI * (2.0 * tex_coord - vec2(1.0f));
    vec2 gaussian = vec2(rand_gaussian(tex_coord.x), rand_gaussian(tex_coord.y));

    // fH_tilde_0_conj0.rg = gaussian * sqrt(abs(phillips(k) * directional_spreading(k)) / 2.0f);
    // fH_tilde_0_conj0.ba = gaussian * sqrt(abs(phillips(-k) * directional_spreading(-k)) / 2.0f);

    fH_tilde_0_conj0.rg = gaussian;
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

uniform sampler2D uH_tilde_0_conj0;

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