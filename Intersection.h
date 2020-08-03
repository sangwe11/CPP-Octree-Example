#ifndef INTERSECTION_H
#define INTERSECTION_H

class OctreeObject;

class Intersection
{
public:
	Intersection(OctreeObject *object1, OctreeObject *object2);
	~Intersection();

	OctreeObject *object1();
	OctreeObject *object2();

private:
	OctreeObject *m_object1;
	OctreeObject *m_object2;
};

#endif