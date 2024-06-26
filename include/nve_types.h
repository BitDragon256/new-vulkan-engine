#pragma once

#include <array>
#include <memory>

#include "vulkan/vulkan_helpers.h"

#include "nve_types_fwd.h"
#include "math-core.h"
#include "reference.h"

// success codes
#define NVE_SUCCESS 0
#define NVE_RENDER_EXIT_SUCCESS 100

// error codes
#define NVE_FAILURE -1

typedef uint32_t Index;
#define NVE_INDEX_TYPE VK_INDEX_TYPE_UINT32

#define VERTEX_ATTRIBUTE_COUNT 5

// assert
#define NVE_ASSERT(X) assert(X);

typedef struct Vertex
{
	Vector3 pos;
	Vector3 normal;
	Vector3 color;
	Vector2 uv;
	uint32_t material;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, VERTEX_ATTRIBUTE_COUNT> getAttributeDescriptions();

	bool operator== (const Vertex& other) const;
	bool operator<(const Vertex& other);
} Vertex;

#define NULL_VERTEX Vertex{ VECTOR_NULL, VECTOR_NULL, VECTOR_NULL, Vector2(0), 0 }

struct CameraPushConstant
{
	glm::mat4 projView;
	alignas(16) Vector3 camPos;
	alignas(16) Vector3 lightPos;
};

#define VECTOR_UP Vector3(0, 0, 1)
#define VECTOR_FORWARD Vector3(1, 0, 0)
#define VECTOR_RIGHT Vector3(0, 1, 0)

#define VECTOR_DOWN Vector3(0, 0, -1)
#define VECTOR_BACK Vector3(-1, 0, 0)
#define VECTOR_LEFT Vector3(0, -1, 0)

#define VECTOR_NULL Vector3(0)

Vector3 vec23(Vector2 vec);

#define PI 3.14159265f

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
};

enum Direction
{
      Forwards, Backwards, Left, Right, Up, Down
};

struct Transform
{
	alignas(16) Vector3 position;
	alignas(16) Vector3 scale;
	alignas(16) Quaternion rotation;
	uint32_t materialStart;

	Transform();
	Transform(Vector3 position, Vector3 scale, Quaternion rotation);
};

bool operator== (const Transform& a, const Transform& b);

