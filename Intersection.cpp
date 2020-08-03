#include "Intersection.h"
#include "Octree.h"

Intersection::Intersection(OctreeObject *object1, OctreeObject *object2)
{
	m_object1 = object1;
	m_object2 = object2;
}

Intersection::~Intersection()
{

}

OctreeObject *Intersection::object1()
{
	return m_object1;
}

OctreeObject *Intersection::object2()
{
	return m_object2;
}