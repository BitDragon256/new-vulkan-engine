#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 uv;
layout(location = 4) in uint inMaterial;

struct Material
{
    vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 transmittance;
	vec3 emission;
	float specularHighlight;
	float refraction;
	float dissolve;
    uint texIndex;
};

struct FragMaterial
{
    vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 transmittance;
	vec3 emission;
	float specularHighlight;
	float refraction;
	float dissolve;
};

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;
layout(location = 3) out flat uint outTex;
layout(location = 4) out FragMaterial outMat;

layout(std140,set = 0, binding = 0) readonly buffer MaterialBuffer
{
    Material mats[];
} MaterialBufferObjects;

struct Transform
{
    vec3 position;
    vec3 scale;
    vec4 rotation;
};

layout(std140,set = 0, binding = 3) readonly buffer ObjectBuffer
{
    Transform objects[];
} ObjectInfoBuffer;

layout( push_constant ) uniform constants
{
    mat4 projView;
    vec3 camPos;
    vec3 lightPos;
} CPC;

// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5, 0.5),
//     vec2(-0.5, 0.5)
// );

FragMaterial convert_mat(Material mat)
{
    return FragMaterial(
        mat.ambient,
        mat.diffuse,
        mat.specular,
        mat.transmittance,
        mat.emission,
        mat.specularHighlight,
        mat.refraction,
        mat.dissolve
    );
}

vec3 even_faster_rot(vec3 v, vec4 q)
{
    vec3 p = q.xyz;
    float w = q.w;
    return (p * dot(p, v) + cross(p, v) * w) * 2 + v * (w * w - dot(p, p));
}

void main()
{
    Transform transform = ObjectInfoBuffer.objects[gl_BaseInstance];
    vec3 vertexPos = even_faster_rot(inPosition * transform.scale, transform.rotation) + transform.position;
    gl_Position = CPC.projView * vec4(vertexPos, 1.0);

    outMat = convert_mat(MaterialBufferObjects.mats[inMaterial]);
    outPos = vertexPos;
    outNormal = inNormal.xzy;
    outUV = uv;
    outTex = MaterialBufferObjects.mats[inMaterial].texIndex;
}