#pragma once
#include <vector>
#include <glm/ext/vector_float3.hpp>
class LightManager
{
private:
	int m_max;

	std::vector<glm::vec3> lightPoses;
	std::vector<glm::vec3> diffuseColor;
	std::vector<glm::vec3> specularColor;
public:
	glm::vec3* GetLightPosesPointer();
	glm::vec3* GetDiffuseColorPointer();
	glm::vec3* GetSpecularColorPointer();

	glm::vec3* GetLightAt(int i);
	glm::vec3* GetDiffuseAt(int i);
	glm::vec3* GetSpecularAt(int i);

	inline int GetSize() const { return lightPoses.size(); };
	inline int GetMax() const { return m_max; };

	bool AddLight(glm::vec3 pos, glm::vec3 diffuse, glm::vec3 specular);
	void RemoveLight(int at);

	inline LightManager(int max) {
		m_max = max;
	}
};

