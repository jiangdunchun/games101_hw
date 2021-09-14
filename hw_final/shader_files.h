#ifndef SHADER_FILES_H
#define SHADER_FILES_H

#include <string>

using namespace std;

string fft_ht_computer_shader = R"delimiter(
#version 430 core
 
#define N 512
#define LENGTH 500.0
#define G 9.81
#define PI 3.1415926
 
layout (binding = 0, rgba16f) uniform image2D u_H0_K; 
layout (binding = 1, rgba16f) uniform image2D u_H_K_t;
 
uniform float u_Time;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
 
void main(void){
	ivec2 storePos = ivec2(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
	ivec2 storePos_negative = ivec2(N - 1 - storePos.x, N - 1 - storePos.y);
	
	//根据位置storePos在贴图中采样得到数据
	vec2 h0 = imageLoad(u_H0_K, storePos).xy;
	vec2 h0_negative = imageLoad(u_H0_K, storePos_negative).xy;
 
	vec2 waveDirection;
	waveDirection.x = (float(-N) / 2.0 + gl_GlobalInvocationID.x) * (2.0 * PI / LENGTH);
    waveDirection.y = (float(N) / 2.0 - gl_GlobalInvocationID.y) * (2.0 * PI / LENGTH);
 
	float w2k = G * length(waveDirection);
 
	float wktime = sqrt(w2k) * u_Time;
 
	float cos_wktime = cos(wktime);
	float sin_wktime = sin(wktime);
 
	vec2 ht;
	ht.x = (h0.x * cos_wktime - h0.y * sin_wktime) + (h0_negative.x * cos_wktime - h0_negative.y * sin_wktime); 
	ht.y = (h0.x * sin_wktime + h0.y * cos_wktime) + (h0_negative.x * sin_wktime + h0_negative.y * cos_wktime); 
	//将算出来的高度值存储到贴图当中
	imageStore(u_H_K_t, storePos, vec4(ht, 0.0, 0.0));
}
)delimiter";

string fft_displacement_computer_shader = R"delimiter(
#version 430 core
 
#define N 512
#define PI	3.1415926
 
uniform int u_processColumn;
 
uniform int u_steps;
 
layout (binding = 0, rgba16f) uniform image2D u_imageIn; 
layout (binding = 1, rgba16f) uniform image2D u_imageOut;
layout (binding = 2, rgba16f) uniform image1D u_imageIndices;
 
shared vec2 sharedStore[N];
 
// as N = 512, so local size is 512/2 = 256. Processing two fields per invocation.
layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;
 
vec2 mulc(vec2 a, vec2 b)
{
	vec2 result;
	
	result.x = a.x * b.x - a.y * b.y;
	result.y = a.x * b.y + b.x * a.y;
	
	return result;
}
 
//转换成单位根向量
vec2 rootOfUnityc(int n, int k)
{
	vec2 result;
	
	result.x = cos(2.0 * PI * float(k) / float(n));
	result.y = sin(2.0 * PI * float(k) / float(n));
 
	return result;
}
 
void main(void)
{
	ivec2 leftStorePos;
	ivec2 rightStorePos;
 
	ivec2 leftLoadPos;
	ivec2 rightLoadPos;
 
	int xIndex = int(gl_GlobalInvocationID.x);
	int yIndex = int(gl_GlobalInvocationID.y);
 
	int leftStoreIndex = 2 * xIndex;
	int rightStoreIndex = 2 * xIndex + 1;
 
	//读取索引（每一组有两个索引例如（0,4））
	int leftLoadIndex = int(imageLoad(u_imageIndices, leftStoreIndex).r);
	int rightLoadIndex = int(imageLoad(u_imageIndices, rightStoreIndex).r);
 
	// 加载和存储位置取决于行或列。
	if (u_processColumn == 0)
	{
		leftLoadPos = ivec2(leftLoadIndex, yIndex);
		rightLoadPos = ivec2(rightLoadIndex, yIndex);
 
		leftStorePos = ivec2(leftStoreIndex, yIndex);
		rightStorePos = ivec2(rightStoreIndex, yIndex);
	}
	else
	{
		leftLoadPos = ivec2(yIndex, leftLoadIndex);
		rightLoadPos = ivec2(yIndex, rightLoadIndex);
 
		leftStorePos = ivec2(yIndex, leftStoreIndex);
		rightStorePos = ivec2(yIndex, rightStoreIndex);
	}
 
	// 从贴图中读取数据
	vec2 leftValue = imageLoad(u_imageIn, leftLoadPos).xy;
	vec2 rightValue = imageLoad(u_imageIn, rightLoadPos).xy;
	//放入到共享缓存中
	sharedStore[leftStoreIndex] = leftValue;
	sharedStore[rightStoreIndex] = rightValue;
 
	//确保所有数据都存储完毕（否则后续逻辑将无法读到所需的数据，即要保证时序）
	memoryBarrierShared();
	barrier();
	
	
	int numberSections = N / 2;
	int numberButterfliesInSection = 1;
 
	int currentSection = xIndex;
	int currentButterfly = 0;
 
	// 计算FFT
	for (int currentStep = 0; currentStep < u_steps; currentStep++)
	{	
		//根据位置来获取该组所需的两个索引
		int leftIndex = currentButterfly + currentSection * numberButterfliesInSection * 2;
		int rightIndex = currentButterfly + numberButterfliesInSection + currentSection * numberButterfliesInSection * 2;
		//从共享缓存中获得数据
		leftValue = sharedStore[leftIndex];
		rightValue = sharedStore[rightIndex];
			 						
		vec2 currentW = rootOfUnityc(numberButterfliesInSection * 2, currentButterfly);
	
		vec2 multiply;
		vec2 addition;
		vec2 subtraction;
 
		multiply = mulc(currentW, rightValue);	
		
		addition = leftValue + multiply;
		subtraction = leftValue - multiply; 
 
		sharedStore[leftIndex] = addition;
		sharedStore[rightIndex] = subtraction;		
 
		// 确保所有数据计算并存储完毕	
		memoryBarrierShared();
 
		// 根据蝴蝶算法来改变参数	
		numberButterfliesInSection *= 2;
		numberSections /= 2;
 
		currentSection /= 2;
		currentButterfly = xIndex % numberButterfliesInSection;
 
		// 确保所有的计算着色器都计算完毕
		barrier();
	}
	
	if (u_processColumn == 1)
	{
		if ((leftStorePos.x + leftStorePos.y) % 2 == 0)
		{
			sharedStore[leftStoreIndex] *= -1.0;
		}
		if ((rightStorePos.x + rightStorePos.y) % 2 == 0)
		{
			sharedStore[rightStoreIndex] *= -1.0;			
		}
		
		memoryBarrierShared();
	}
	
	imageStore(u_imageOut, leftStorePos, vec4(sharedStore[leftStoreIndex], 0.0, 0.0));
	imageStore(u_imageOut, rightStorePos, vec4(sharedStore[rightStoreIndex], 0.0, 0.0));
}
)delimiter";


string fft_normal_computer_shader = R"delimiter(
#version 430 core
 
#define N 512
#define LENGTH	500.0f
 
#define VERTEX_STEP (LENGTH / float(N - 1))
#define DIAGONAL_VERTEX_STEP sqrt(VERTEX_STEP * VERTEX_STEP * 2.0)
 
layout (binding = 0, rgba16f) uniform image2D u_imageIn; 
layout (binding = 1, rgba16f) uniform image2D u_imageOut;
 
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
 
float sampleImage(ivec2 pos)
{
	ivec2 cpos = clamp(pos, 0, N - 1);
	
	return imageLoad(u_imageIn, cpos).r;
}
 
vec3 calculateNormal(ivec2 texCoord)
{
	vec3 normal = vec3(0.0f, 0.0f, 0.0f);
 
	ivec2 right = ivec2(texCoord.s + 1, texCoord.t);
	ivec2 top = ivec2(texCoord.s, texCoord.t + 1);
	ivec2 left = ivec2(texCoord.s - 1, texCoord.t);
	ivec2 bottom = ivec2(texCoord.s, texCoord.t - 1);
 
	float slopeHorizontal = sampleImage(right) - sampleImage(left);
	float slopeVertical = sampleImage(top) - sampleImage(bottom);
	
	vec3 tangent = normalize(vec3(2.0 * VERTEX_STEP, slopeHorizontal, 0.0));
	vec3 bitangent = normalize(vec3(0.0, slopeVertical, -2.0 * VERTEX_STEP));
	
	normal += normalize(cross(tangent, bitangent));
	
	ivec2 rightTop = ivec2(texCoord.s + 1, texCoord.t + 1);
	ivec2 leftTop = ivec2(texCoord.s - 1, texCoord.t + 1);
	ivec2 leftBottom = ivec2(texCoord.s - 1, texCoord.t - 1);
	ivec2 rightBottom = ivec2(texCoord.s + 1, texCoord.t - 1);
	
	float slopeDown = sampleImage(rightBottom) - sampleImage(leftTop);
	float slopeUp = sampleImage(rightTop) - sampleImage(leftBottom);
 
	tangent = normalize(vec3(2.0 * DIAGONAL_VERTEX_STEP, slopeDown, 2.0 * DIAGONAL_VERTEX_STEP));
	bitangent = normalize(vec3(2.0 * DIAGONAL_VERTEX_STEP, slopeUp, -2.0 * DIAGONAL_VERTEX_STEP));
 
	normal += normalize(cross(tangent, bitangent));
		
	return normalize(normal);
}
 
void main(void)
{
	ivec2 storePos = ivec2(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
 
	vec3 normal = calculateNormal(storePos);
 
	imageStore(u_imageOut, storePos, vec4(normal, 0.0));
}
)delimiter";

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

vec2 rand_gaussian(vec2 co){
    float x1 = fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
    float x2 = fract(sin(dot(co.yx, vec2(12.9898,78.233))) * 43758.5453);

    x1 = max(1e-6f, x1);
    x2 = max(1e-6f, x2);

    float g1 = sqrt(-2.0f * log(x1)) * cos(2.0f * PI * x2);
    float g2 = sqrt(-2.0f * log(x1)) * sin(2.0f * PI * x2);

    return vec2(g1, g2);
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
    vec2 gaussian = rand_gaussian(tex_coord);

    fH_tilde_0_conj0.rg = gaussian * sqrt(abs(phillips(k) * directional_spreading(k)) / 2.0f);
    fH_tilde_0_conj0.ba = gaussian * sqrt(abs(phillips(-k) * directional_spreading(-k)) / 2.0f);

    //fH_tilde_0_conj0.rg = gaussian;
    //fH_tilde_0_conj0.ba = vec2(0.0f);
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