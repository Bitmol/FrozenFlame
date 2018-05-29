#ifndef MESH_H
#define MESH_H

#include "Vertex.h"
#include <vector>

/************************************************************
 * Triangle Class
 ************************************************************/
class Triangle {
public:
    inline Triangle () {
        v[0] = v[1] = v[2] = 0;
    }
    inline Triangle (const Triangle & t) {
        v[0] = t.v[0];
        v[1] = t.v[1];
        v[2] = t.v[2];
    }
    inline Triangle (unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
    }
    inline virtual ~Triangle () {}
    inline Triangle & operator= (const Triangle & t) {
        v[0] = t.v[0];
        v[1] = t.v[1];
        v[2] = t.v[2];
        return (*this);
    }
    unsigned int v[3];
};

/************************************************************
 * Basic Mesh Class
 ************************************************************/
class Mesh {
public:
    Mesh();
    inline Mesh (const std::vector<Vertex> & v, const std::vector<Triangle> & t) : vertices (v), triangles (t)  {}
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;

    bool loadMesh(const char * filename);
    void computeVertexNormals ();
    void centerAndScaleToUnit ();
    void computeBoundingCube();

    //Bounding box information
	//point of bounding box with minimal coordinates ("lower left corner")
    Vec3Df bbOrigin;
	//extent of the bounding box (for simplicity, assumed to be the same along each axis)
    float bbEdgeSize;
};

#endif // MESH_H
