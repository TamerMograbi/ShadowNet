
#include <list>
#include <vector>
#include <nori/ray.h>
NORI_NAMESPACE_BEGIN

class Node
{
public:
	Node(BoundingBox3f box, std::list<int> triangles) {
		m_box = box;
		m_triangles = triangles;
		m_child = std::vector<Node*>(8,nullptr);
	}

	
	BoundingBox3f m_box;
	std::list<int> m_triangles;
	std::vector<Node*> m_child;
};

NORI_NAMESPACE_END