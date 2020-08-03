#ifndef AABB_H
#define AABB_H

#include <glm\glm.hpp>

class AABB
{
public:
	AABB();
	AABB(const glm::vec3 &min, const glm::vec3 &max);

	const bool contains(const AABB &aabb) const;
	const bool intersects(const AABB &aabb) const;

//private:
	glm::vec3 m_min;
	glm::vec3 m_max;
};

#endif