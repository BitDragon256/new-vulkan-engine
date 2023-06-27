#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "nve_types.h"

class Texture
{
public:
	void load_from(std::string file);
	VkImage m_tex;
};

class Material
{
public:
	Texture* m_pDiffuseTexture;
	Texture* m_pNormalTexture;
	VkShaderModule* m_pShader;
	Vector4 m_diffuseColor;
	Vector3 m_ambientColor;
	float m_alphaTest;

	void load_shader(std::string file);
	void load_diff_tex(std::string file);
	void load_normal_tex(std::string file);

	bool operator==(const Material& other);
};