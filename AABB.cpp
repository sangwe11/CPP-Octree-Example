#include "AABB.h"

AABB::AABB()
{
	m_min = glm::vec3(0.0f);
	m_max = glm::vec3(0.0f);
}

AABB::AABB(const glm::vec3 &min, const glm::vec3 &max)
{
	m_min = min;
	m_max = max;

	assert(m_max.x >= m_min.x && m_max.y >= m_min.y && m_max.z >= m_min.z);
}

const bool AABB::contains(const AABB &aabb) const
{
	if (aabb.m_min.x >= m_min.x && aabb.m_min.y >= m_min.y && aabb.m_min.z >= m_min.z && aabb.m_max.x <= m_max.x && aabb.m_max.y <= m_max.y && aabb.m_max.z <= m_max.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

const bool AABB::intersects(const AABB &aabb) const
{
	// Is there at least one seperating axis
	if (this->m_min.x > aabb.m_max.x) return false;
	if (this->m_min.y > aabb.m_max.y) return false;
	if (this->m_min.z > aabb.m_max.z) return false;
	if (this->m_max.x < aabb.m_min.x) return false;
	if (this->m_max.y < aabb.m_min.y) return false;
	if (this->m_max.z < aabb.m_min.z) return false;

	return true;
}