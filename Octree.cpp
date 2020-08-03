#include "Octree.h"

OctreeObject::OctreeObject()
{

}

const bool &OctreeObject::alive() const
{
	return m_alive;
}

const AABB &OctreeObject::getAABB() const
{
	return m_aabb;
}

Octree::Octree(const AABB &region)
{
	m_region = region;
	m_ready = true;
}

Octree::Octree(const AABB &region, const std::list<OctreeObject *> &objects)
{
	m_region = region;
	m_pending = objects;
	m_ready = false;
}

Octree::Octree(const glm::vec3 &extents)
{
	m_region = AABB(-extents, extents);
	m_ready = true;
}

Octree::Octree(const glm::vec3 &extents, const std::list<OctreeObject *> &objects)
{
	m_region = AABB(-extents, extents);
	m_pending = objects;
	m_ready = false;
}

Octree::~Octree()
{
	for (unsigned int i = 0; i < 8; ++i)
	{
		if (m_children[i] != nullptr)
		{
			delete m_children[i];
			m_children[i] = nullptr;
		}
	}
}

size_t Octree::countChildren()
{
	int count = 0;

	for (int i = 0; i < 8; ++i)
	{
		if(m_children[i] != nullptr)
		{
			++count;
		}
	}

	return count;
}

size_t Octree::countObjects()
{
	return m_objects.size();
}

size_t Octree::countTreeObjects()
{
	size_t count = 0;

	count += m_objects.size();

	for (Octree * child : m_children)
	{
		if (child != nullptr)
		{
			count += child->countTreeObjects();
		}
	}

	return count;
}

void Octree::queue(OctreeObject *object)
{
	m_pending.push_back(object);

	m_ready = false;
}

void Octree::queue(std::list<OctreeObject *> objects)
{
	for (OctreeObject * object : objects)
	{
		m_pending.push_back(object);
	}

	m_ready = false;
}

void Octree::updateTree(std::vector<Intersection> &intersections)
{
	if (!m_ready || m_pending.size() > 0)
	{
		for (OctreeObject *object : m_pending)
		{
			if (!this->insert(object))
			{
				// Failed
				object->m_alive = false;
			}
		}

		m_pending.clear();
		m_ready = true;
	}

	// Lifespan for empty nodes, doubled each time a node is reused before death
	// This allows us to keep frequently reused nodes alive longer
	if (this->countObjects() == 0)
	{
		if (this->countChildren() == 0)
		{
			if (m_life == -1)
			{
				m_life = m_lifeSpan;
			}
			else if (m_life > 0)
			{
				--m_life;
			}
		}
	}
	else
	{
		if (m_lifeSpan <= m_maxLife)
		{
			m_lifeSpan *= 2;
		}

		m_life = -1;
	}

	// Remove dead objects
	m_objects.remove_if([](OctreeObject *& i) { return i->m_alive == false; });
	
	// Remove dead branches
	for (unsigned int i = 0; i < 8; ++i)
	{
		if (m_children[i] != nullptr)
		{
			if (m_children[i]->m_life == 0)
			{
				if (m_children[i]->countChildren() > 0)
				{
					// Trying to prune a branch thats still in use..
					m_children[i]->m_life = -1; // reset life
				}
				else
				{
					delete m_children[i];
					m_children[i] = nullptr;
				}
			}
		}
	}

	// Update children
	for (unsigned int i = 0; i < 8; ++i)
	{
		if (m_children[i] != nullptr)
		{
			m_children[i]->updateTree(intersections);
		}
	}

	// Build a list of "moved" objects
	std::list<OctreeObject *> movedObjects;

	for (OctreeObject *object : m_objects)
	{
		if (object->hasMoved())
		{
			movedObjects.push_back(object);
		}
	}

	// TODO: Implement "moved" objects
	for (OctreeObject *object : movedObjects)
	{
		AABB objectAABB = object->getAABB();

		// Remove from current region
		m_objects.remove(object);

		Octree *currentTree = this;

		// Iterate upwards finding the first region that now contains the object
		while (!currentTree->m_region.contains(objectAABB))
		{
			currentTree = currentTree->m_parent;

			if (currentTree == nullptr)
			{
				break; // Object has moved outside of root tree
			}
		}

		if (currentTree != nullptr)
		{
			// Insert at the first enclosing region
			// This will automatically propegate down to the smallest enclosing region
			if(!currentTree->insert(object))
			{
				// Something went wrong
				object->m_alive = false;
			}
		}
		else
		{
			// Failed
			object->m_alive = false;
		}
	}

	// Check for octree intersections if we're the parent node
	if (this->m_parent == nullptr)
	{
		std::list<OctreeObject *> objects;
		unsigned int checks = this->getIntersections(objects, intersections);
	}

}

// Takes an octree item, and inserts it into the current tree
bool Octree::insert(OctreeObject *object)
{
	// Don't add dead objects into the tree
	if (!object->m_alive)
	{
		return false;
	}

	// Object dimensions
	AABB objectAABB = object->getAABB();
	glm::vec3 objectDimensions = objectAABB.m_max - objectAABB.m_min;

	// Invalid AABB?
	if (objectDimensions == glm::vec3(0.0f))
	{
		return false;
	}

	// Region dimensions
	glm::vec3 regionDimensions = m_region.m_max - m_region.m_min;

	// Region encloses object?
	if (objectDimensions.x > regionDimensions.x || objectDimensions.y > regionDimensions.y || objectDimensions.z > regionDimensions.z)
	{
		if (m_parent != nullptr)
		{
			// Push upwards to parent
			return m_parent->insert(object);
		}
		else
		{
			// Item won't fit into tree
			return false;
		}
	}

	// Empty leaf node?
	if (this->m_leaf && this->countTreeObjects() == 0)
	{
		m_objects.push_back(object);
		return true;
	}

	// Reached smallest enclosing region?
	if (regionDimensions.x <= MIN_SIZE && regionDimensions.y <= MIN_SIZE && regionDimensions.z <= MIN_SIZE)
	{
		m_objects.push_back(object);
		return true;
	}

	// Split region into a 8 children
	glm::vec3 half = regionDimensions / 2.0f;
	glm::vec3 centre = m_region.m_min + half;

	//std::cout << "Half: " << glm::to_string(half) << std::endl;
	//std::cout << "Centre: " << glm::to_string(centre) << std::endl;

	AABB childRegions[8];
	childRegions[0] = (m_children[0] != nullptr) ? m_children[0]->m_region : AABB(m_region.m_min, centre);
	childRegions[1] = (m_children[1] != nullptr) ? m_children[1]->m_region : AABB(glm::vec3(centre.x, m_region.m_min.y, m_region.m_min.z), glm::vec3(m_region.m_max.x, centre.y, centre.z));
	childRegions[2] = (m_children[2] != nullptr) ? m_children[2]->m_region : AABB(glm::vec3(centre.x, m_region.m_min.y, centre.z), glm::vec3(m_region.m_max.x, centre.y, m_region.m_max.z));
	childRegions[3] = (m_children[3] != nullptr) ? m_children[3]->m_region : AABB(glm::vec3(m_region.m_min.x, m_region.m_min.y, centre.z), glm::vec3(centre.x, centre.y, m_region.m_max.z));
	childRegions[4] = (m_children[4] != nullptr) ? m_children[4]->m_region : AABB(glm::vec3(m_region.m_min.x, centre.y, m_region.m_min.z), glm::vec3(centre.x, m_region.m_max.y, centre.z));
	childRegions[5] = (m_children[5] != nullptr) ? m_children[5]->m_region : AABB(glm::vec3(centre.x, centre.y, m_region.m_min.z), glm::vec3(m_region.m_max.x, m_region.m_max.y, centre.z));
	childRegions[6] = (m_children[6] != nullptr) ? m_children[6]->m_region : AABB(centre, m_region.m_max);
	childRegions[7] = (m_children[7] != nullptr) ? m_children[7]->m_region : AABB(glm::vec3(m_region.m_min.x, centre.y, centre.z), glm::vec3(centre.x, m_region.m_max.y, m_region.m_max.z));

	// Push into child region?
	for (unsigned int i = 0; i < 8; ++i)
	{
		if (childRegions[i].contains(objectAABB))
		{
			if (m_children[i] != nullptr)
			{
				return m_children[i]->insert(object);
			}
			else
			{
				m_children[i] = new Octree(childRegions[i]);
				m_children[i]->m_parent = this;
				m_children[i]->m_leaf = true;
				return m_children[i]->insert(object);
			}
		}
	}

	// Last resort, add to current region
	m_objects.push_back(object);

	return true;
}

unsigned int Octree::getIntersections(std::list<OctreeObject *> parent_objects, std::vector<Intersection> &intersections)
{
	unsigned int checks = 0;

	// As this is recursive, we assume parent_object vs parent_object tests have already been done.
	for (OctreeObject *parent_object : parent_objects)
	{
		AABB parent_aabb = parent_object->getAABB();

		for (OctreeObject *object : m_objects)
		{
			AABB object_aabb = object->getAABB();

			++checks;

			if (object_aabb.intersects(parent_aabb))
			{
				intersections.emplace_back(parent_object, object);
			}
		}
	}

	// Now check local objects vs local objects
	// Two nested loops here would result in O(N^2) runtime, our method results in O(N(N+1)/2) runtime.
	// Copy objects into a temporary list, iterate back to front and remove object once it has been tested against all others
	// TODO: Only moved objects actually need to be tested?
	std::list<OctreeObject *> temp_objects(m_objects);

	while (temp_objects.size() > 0)
	{
		OctreeObject *temp_object = temp_objects.back();

		for (OctreeObject *object : temp_objects)
		{
			// Don't test objects against themselves..
			if (temp_object == object)
				continue;

			AABB temp_aabb = temp_object->getAABB();
			AABB object_aabb = object->getAABB();

			++checks;

			if (temp_aabb.intersects(object_aabb))
			{
				intersections.emplace_back(object, temp_object);
			}
		}

		// Add into the parent list and remove from our temporary list
		parent_objects.push_back(temp_object);
		temp_objects.remove(temp_object);
	}

	// Check children
	for (unsigned int i = 0; i < 8; ++i)
	{
		if (m_children[i] != nullptr)
		{
			checks += m_children[i]->getIntersections(parent_objects, intersections);
		}
	}

	return checks;
}