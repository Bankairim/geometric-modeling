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
    facePoint = NULL;
}

myFace::~myFace(void)
{
	if (normal)
	{
		delete normal;
		normal = nullptr;
	}
}
myFace& myFace::operator=(const myFace& other) {
    // Protection contre l'auto-affectation
    if (this == &other) {
        return *this;
    }

    // Copiez la halfedge adjacente 
    this->adjacent_halfedge = other.adjacent_halfedge;
    this->facePoint = other.facePoint;
    // Copiez la normale (créez un nouvel objet myVector3D pour éviter le partage de la mémoire)
    if (other.normal) {
        if (this->normal) {
            *this->normal = *other.normal;
        }
        else {
            this->normal = new myVector3D(*other.normal);
        }
    }
    else {
        if (this->normal) {
            delete this->normal;
            this->normal = nullptr;
        }
    }
    // Retourne la référence à cet objet
    return *this;
}

bool myFace::isDegenerate() {
    if (!adjacent_halfedge) return true; // Si aucune halfedge adjacente, la face est considérée comme dégénérée

    myHalfedge* start = adjacent_halfedge;
    myHalfedge* current = start;
    int count = 0;

    // Parcourir les halfedges de la face pour compter le nombre de sommets uniques
    do {
        // Ajouter une vérification pour s'assurer que 'current' et 'current->next' ne sont pas nuls
        if (!current || !current->next) {
            return true; // Si 'current' ou 'current->next' est nul, la face est considérée comme dégénérée
        }
        count++;
        current = current->next;
    } while (current && current != start && count <= 3); // Arrêter si on revient au départ ou si on a compté plus que 3 halfedges

    // Si on a moins de 3 sommets uniques, la face est dégénérée
    return count < 3;
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
	//std::cout << "computedNormal face : " << computedNormal.dX << std::endl;

	*normal = computedNormal;
	//std::cout << "normal face : " << normal->dX << std::endl;
}