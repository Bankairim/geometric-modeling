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
#include <utility>


class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	/*std::map<myFace*, myVertex*> facePoints;
	std::map<myHalfedge*, myVertex*> edgePoints;*/
	//std::map<std::pair<int, int>, myHalfedge*> twin_map;
	std::vector<myFace *> faces;
	std::string name;
	std::map<std::pair<int, int>, myHalfedge*> twinMap;

	void checkMesh();
	bool checkMeshAdvanced();
	void myMesh::checkVertice(myVertex* v);
	// Assuming 'myMesh mesh' and 'std::vector<myHalfedge*> halfedges' are already defined
	void myMesh::testHalfEdgeProperties(std::vector<myHalfedge*>& halfedges);
	void myMesh::displayHalfEdgeProperties(myHalfedge* he);

	bool readFile(std::string filename);
	void computeNormals();
	void normalize();

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

	void myMesh::createTriangles(myVertex* centerVertex, const std::vector<myVertex*>& faceVertices);
	myVertex* myMesh::createCenterVertex(const std::vector<myVertex*>& faceVertices);
	myVertex* myMesh::createCenterVertex(myFace* f);
	void updateTwinHalfedges();
	void createTriangle(myVertex* A, myVertex* B, myVertex* C);

	void clear();

	myMesh(void);
	~myMesh(void);

	//For simplification

	bool myMesh::collapse(myHalfedge* e);
	bool myMesh::collapse();
	myPoint3D* bestPosition(myPoint3D* p1, myPoint3D* p2);
	myHalfedge* findMinimalHalfedge();
	void myMesh::displayAllHalfEdgeProperties();
	//void myMesh::allCollapse(double d);
	


	// For subdivision
	void myMesh::subdivisionCatmullClark();
	myVertex* myMesh::computeFacePoints(myFace* f);
	myPoint3D* myMesh::averageOfFourPoints(myPoint3D* p1, myPoint3D* p2, myPoint3D* f1, myPoint3D* f2);
};