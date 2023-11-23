#pragma once
#include <map>
#include <utility>
#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>


class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	//std::map<std::pair<int, int>, myHalfedge*> twin_map;
	std::vector<myFace *> faces;
	std::string name;

	void checkMesh();
	bool checkMeshAdvanced();
	// Assuming 'myMesh mesh' and 'std::vector<myHalfedge*> halfedges' are already defined
	void myMesh::testHalfEdgeProperties(std::vector<myHalfedge*>& halfedges);
	void myMesh::displayHalfEdgeProperties(myHalfedge* he);

	bool readFile(std::string filename);
	void computeNormals();
	void normalize();

	void subdivisionCatmullClark();

	void splitFaceTRIS(myFace *, myPoint3D *);

	void splitEdge(myHalfedge *, myPoint3D *);
	void splitFaceQUADS(myFace *, myPoint3D *);
	void myMesh::clearTwinRelationships();
	void myMesh::establishTwinRelationships();
	void testTriangulate();
	void myMesh::checkHalfEdgeReferences();
	void triangulate();
	bool triangulate(myFace *);;
	//void myMesh::createTriangles(myVertex* centerVertex, const std::vector<myVertex*>& faceVertices, std::vector<myHalfedge*>& newHalfedges, const std::map<std::pair<int, int>, myHalfedge*>& edgeMap);

	void myMesh::createTriangles(myVertex* centerVertex, const std::vector<myVertex*>& faceVertices, std::vector<myHalfedge*>& newHalfedges);
	myVertex* myMesh::createCenterVertex(const std::vector<myVertex*>& faceVertices);
	void updateTwinHalfedges();
	void createTriangle(myVertex* A, myVertex* B, myVertex* C);

	void clear();

	myMesh(void);
	~myMesh(void);
};

