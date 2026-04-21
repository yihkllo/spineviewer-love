#ifndef SLOT_MESH_DATA_H_
#define SLOT_MESH_DATA_H_

#include <vector>

struct SlotMeshData
{
	std::vector<float> worldVertices;
	std::vector<float> uvs;
	std::vector<unsigned short> triangles;
	int hullLength = 0;
	int textureHandle = -1;
	bool isRegion = false;
};

#endif
