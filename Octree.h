#ifndef OCTREE_H
#define OCTREE_H

#include <list>
#include <vector>

#include "AABB.h"
#include "Intersection.h"

class OctreeObject
{
public:
	OctreeObject();
	const AABB &getAABB() const;
	const bool &alive() const;

	virtual void calculateAABB() = 0;
	const virtual bool hasMoved() const = 0;

protected:
	bool m_alive = true;
	AABB m_aabb;

	friend class Octree;
};

class Octree
{
public:
	const unsigned int MIN_SIZE = 1; // Minimum size for a bounding region is 1x1x1

	Octree(const AABB &region);
	Octree(const AABB &region, const std::list<OctreeObject *> &objects);

	Octree(const glm::vec3 &extents);
	Octree(const glm::vec3 &extents, const std::list<OctreeObject *> &objects);

	~Octree();

	size_t countChildren();
	size_t countObjects();
	size_t countTreeObjects();

	void queue(OctreeObject *object);
	void queue(std::list<OctreeObject *> objects);

	void updateTree(std::vector<Intersection> &intersections);

private:

	bool insert(OctreeObject *object);

	unsigned int getIntersections(std::list<OctreeObject *> parent_objects, std::vector<Intersection> &intersections);

	AABB m_region;

	Octree *m_parent = nullptr;
	Octree *m_children[8] = { nullptr };

	bool m_ready = false;

	std::list<OctreeObject *> m_objects;
	std::list<OctreeObject *> m_pending;

	int m_life = -1;
	int m_lifeSpan = 8;
	const int m_maxLife = 64;
	bool m_leaf = false;
};

#endif