#include "grid.h"
#include "mesh.h"
#include <vector>
#ifdef WIN32
#include <windows.h>
#endif

int Grid::isContainedAt(const Vec3Df & pos){
    //returns index that contains the position
	float cubeLength = size / r;

	Vec3Df v = pos - origin;
	int x = v[0] / cubeLength;
	int y = v[1] / cubeLength;
	int z = v[2] / cubeLength;

	int indice = x + r * y + r * r*z;
	return indice;
}

void Grid::addToCell(const Vec3Df & vertexPos) {
	int nr = isContainedAt(vertexPos);
	std::vector<Vec3Df> list = verticesInCell[nr];
	list.push_back(vertexPos);
	verticesInCell[nr] = list;
}

void Grid::putVertices(const std::vector<Vertex> & vertices){
    //put vertices in the corresponding voxels.
	for (int i = 0; i < vertices.size(); i++) {
		addToCell(vertices[i].p);
	}
}

void Grid::computeRepresentatives() {
	for (int i = 0; i < r*r*r; i++) {
		std::vector<Vec3Df> list = verticesInCell[i];
		if (list.size() > 0) {
			Vec3Df p = Vec3Df(0, 0, 0);
			Vec3Df n = Vec3Df(0, 0, 0);
			for (std::vector<Vec3Df>::iterator it = list.begin(); it != list.end(); ++it) {
				p = p + *it;
			}
			p = p / list.size();
			representatives[i] = Vertex(p, n);
		}
	}
}
//
//void Grid::drawCell(const Vec3Df & Min,const Vec3Df& Max) {
//
//    const Vec3Df corners[8] =
//    {
//        Vec3Df(Min[0],Min[1],Min[2]),
//        Vec3Df(Max[0],Min[1],Min[2]),
//        Vec3Df(Min[0],Max[1],Min[2]),
//        Vec3Df(Max[0],Max[1],Min[2]),
//        Vec3Df(Min[0],Min[1],Max[2]),
//        Vec3Df(Max[0],Min[1],Max[2]),
//        Vec3Df(Min[0],Max[1],Max[2]),
//        Vec3Df(Max[0],Max[1],Max[2])
//    };
//
//
//    static const unsigned short faceCorners[6][4] =
//    {
//        { 0,4,6,2 },
//        { 5,1,3,7 },
//        { 0,1,5,4 },
//        { 3,2,6,7 },
//        { 0,2,3,1 },
//        { 6,4,5,7 }
//    };
//
//    glBegin(GL_QUADS);
//    for (unsigned short f=0;f<6;++f)
//    {
//        const unsigned short* face = faceCorners[f];
//        for(unsigned int v = 0; v<4; v++)
//            glVertex3f(corners[face[v]][0], corners[face[v]][1], corners[face[v]][2]);
//
//    }
//    glEnd();
//
//}
//
//void Grid::drawGrid(){
//    glPushAttrib(GL_ALL_ATTRIB_BITS);
//    glLineWidth(1.0f);
//    glColor3f(1.0f,1.0f,0.0f);
//    glDisable(GL_LIGHTING);
//    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
//
//    //Complete this function by calling draw cell several times.
//	float cubeLength = size / r;
//
//	for (int z = 0; z < r; z++) {
//		for (int y = 0; y < r; y++) {
//			for (int x = 0; x < r; x++) {
//				int nr = x + r * y + r * r*z;
//				if (verticesInCell[nr].size() > 0) {
//					float xmin = origin[0] + x * cubeLength;
//					float ymin = origin[1] + y * cubeLength;
//					float zmin = origin[2] + z * cubeLength;
//
//					float xmax = origin[0] + (x + 1) * cubeLength;
//					float ymax = origin[1] + (y + 1) * cubeLength;
//					float zmax = origin[2] + (z + 1) * cubeLength;
//
//					Vec3Df min = Vec3Df();
//					min[0] = xmin;
//					min[1] = ymin;
//					min[2] = zmin;
//
//					Vec3Df max = Vec3Df();
//					max[0] = xmax;
//					max[1] = ymax;
//					max[2] = zmax;
//
//					drawCell(min, max);
//				}
//			}
//		}
//	}
//
//    glPopAttrib();
//}

Mesh Grid::simplifyMesh(Mesh mesh, unsigned int r) {
	//Create a grid that covers the bounding box of the mesh. 
	//Be thorough and check all functions, as some of the calls below might NOT directly work and need to be written by you.
	//It should be considered a guideline, NOT the solution.
	//Also, use your graphics knowledge to draw for debugging! (e.g., draw the bounding box, the grid etc.)  
	double offset = 0.01;
	Vec3Df vecOffset = Vec3Df(offset, offset, offset);
	Grid grid = Grid(mesh.bbOrigin - vecOffset, mesh.bbEdgeSize + 2 * offset, r);

	//work with a local reference on the vertices and triangles
	const std::vector<Vertex> & vertices = mesh.vertices;
	const std::vector<Triangle> & triangles = mesh.triangles;

	//   //Put all the vertices in the grid
	grid.putVertices(mesh.vertices);

	//	  //calculate a representative vertex for each grid cell
	grid.computeRepresentatives();

	// //Create a new list of vertices for the simplified model
	// //What is the effect of the code below?
	std::map<unsigned int, unsigned int > newIndexRemapping;
	std::vector<Vertex> simplifiedVertices;

	int count = 0;
	for (RepresentativeList::iterator it = grid.representatives.begin(); it != grid.representatives.end(); it++, count++) {
		newIndexRemapping[(*it).first] = count;
		simplifiedVertices.push_back((*it).second);
	}


	// //Create a new list of triangles
	// //This is NOT COMPLETE and you need to add code here
	// //Think about what simplifiedVertices and newIndexRemapping contain
	std::vector<Triangle> simplifiedTriangles;
	for (int i = 0; i < triangles.size(); i++) {
		Triangle tr = triangles[i];
		int indice1 = grid.isContainedAt(vertices[tr.v[0]].p);
		int indice2 = grid.isContainedAt(vertices[tr.v[1]].p);
		int indice3 = grid.isContainedAt(vertices[tr.v[2]].p);

		if (indice1 == indice2 && indice1 == indice3) {
			continue;
		}

		Triangle trSimple = Triangle(newIndexRemapping[indice1],
			newIndexRemapping[indice2],
			newIndexRemapping[indice3]);
		simplifiedTriangles.push_back(trSimple);
	}

	// //Build the simplified mesh from the CORRECT lists
	Mesh simplified = Mesh(simplifiedVertices, simplifiedTriangles);

	// //recalculate the normals.
	simplified.centerAndScaleToUnit();
	simplified.computeVertexNormals();
	simplified.computeBoundingCube();
	return simplified;
}



