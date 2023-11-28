#include "myVertex.h"  // Pour myVertex
#include "myFace.h"    // Pour myFace
#include "myHalfedge.h"
#include <iostream>


myHalfedge::myHalfedge(void)
{
	source = NULL; 
	adjacent_face = NULL; 
	next = NULL;  
	prev = NULL;  
	twin = NULL;  
}

void myHalfedge::copy(myHalfedge *ie)
{
/**** TODO ****/
}

myHalfedge::~myHalfedge(void)
{
}

void myHalfedge::displayProperties() const {
    std::cout << "Halfedge index: " << index << std::endl;
    std::cout << "  Source vertex index: " << (source ? source->index : -1) << std::endl;
    std::cout << "  Adjacent face index: " << (adjacent_face ? adjacent_face->index : -1) << std::endl;
    std::cout << "  Next halfedge index: " << (next ? next->index : -1) << std::endl;
    std::cout << "  Prev halfedge index: " << (prev ? prev->index : -1) << std::endl;
    std::cout << "  Twin halfedge index: " << (twin ? twin->index : -1) << "\n" << std::endl;
}
