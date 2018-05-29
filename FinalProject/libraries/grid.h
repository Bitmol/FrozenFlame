#ifndef GRID_H
#define GRID_H

#include "Vec3D.h"
#include <vector>
#include "Vertex.h"
#include "mesh.h"
#include <map>

typedef std::map<unsigned, std::vector<Vec3Df> > CellContent;
typedef std::map<unsigned, Vertex> RepresentativeList;

//The above structures can be used almost like a vector!
//The difference is that they are sparse. This means: each entry will only exist ONCE in memory and only after it has been used. 
//One big difference occurs when iterating over the elements:
//Try out something like this:
//std::map<unsigned int, unsigned int> t;
//t[2]=3;
//t[3]=5;
//for (std::map<unsigned int, unsigned int>::iterator iter=t.begin; iter!=t.end();++iter)
//{
// std::cout<< "t["<<iter->first<<"]<<"="<<iter->second<<endl;
//}
//empty indices will be skipped
//btw. in C++ 11 you can replace: 
//for (std::map<unsigned int, unsigned int>::iterator iter=t.begin; iter!=t.end();++iter) 
//by for (auto iter=t.begin; iter!=t.end();++iter)


class Grid
{
public:
    Grid(){}
    inline Grid (const Vec3Df & origin, float size, unsigned int r) : origin (origin), size(size) , r(r) {}

	//The corners with the smallest and largest coordinates.
    void drawCell(const Vec3Df & min,const Vec3Df& Max);
	//draw all the cells
    void drawGrid();

	Mesh simplifyMesh(Mesh mesh, unsigned int r);

	//number of grid cells
    unsigned int r;
	//position of the grid (min corner is at origin and its extent is defined by size).
    Vec3Df origin;
    float size;
	//remark: for simplicity, we work with a cube and the size/extent values are the same for all axes.

    //add a point to a cell
	void addToCell(const Vec3Df & vertexPos);
    //add all vertices of the model to the cells
	void putVertices(const std::vector<Vertex> & vertices);
    //find the index containing the given point, return -1 if it cannot be found
	int isContainedAt(const Vec3Df & pos);

	//for each cell, compute a representative point of all contained points
	void computeRepresentatives();
    //retrieve the representative from representatives by first finding the cell containing pos 
	//then looking up the representative
	Vec3Df getRepresentative(const Vec3Df & pos);

	
    CellContent verticesInCell;
	RepresentativeList representatives;
};

#endif // GRID_H
