#include "myFace.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <GL/glew.h>
#include <iostream>

myFace::myFace(void)
{
	adjacent_halfedge = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
}

myFace::~myFace(void)
{
	if (normal) delete normal;
}

void myFace::computeNormal()
{
    if (!adjacent_halfedge) return; // Si aucune halfedge adjacente

	myVertex* v1 = adjacent_halfedge->source;
	myVertex* v2 = adjacent_halfedge->next->source;
	myVertex* v3 = adjacent_halfedge->prev->source;


	myVector3D edge1 = *(v2->point) - *(v1->point);
	myVector3D edge2 = *(v3->point) - *(v1->point);

	myVector3D computedNormal = edge1.crossproduct(edge2);
	
	computedNormal.normalize();
	std::cout << "computedNormal face : " << computedNormal.dX << std::endl;

	*normal = computedNormal;
	std::cout << "normal face : " << normal->dX << std::endl;
}

