#include "myVertex.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myFace.h"
#include <iostream>

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
    if (!originof) {
        std::cout << "Vertex " << index << ": originof est NULL" << std::endl;
        return;
    }

    myVector3D accumulatedNormal(0.0, 0.0, 0.0);
    int faceCount = 0;

    // Commencer par la halfedge d'origine.
    myHalfedge* start = originof;
    myHalfedge* current = start;
    int maxIterations = 2000;  // Si ca boucle trop longtemps on s'arrete à ce chiffre
    int iteration = 0;

    do {
        // Vérifier les pointeurs avant de les déréférencer.
        //std::cout << "Nous entrons dans la condition myVertex" << std::endl;

        if (current && current->adjacent_face && current->adjacent_face->normal) {

            //std::cout << "condition adjacent face normal vérifiée. Il existe une normale" << std::endl;
            accumulatedNormal += *(current->adjacent_face->normal); // Utilisez l'opérateur d'addition surchargé de myVector3D
            faceCount++;
        }
        else
        {
            std::cout << "Pas de current ou de face adjacente  disponible." << std::endl;
        }
        

        if (current && current->twin) {
            current = current->twin->next;
        }
        else {
            break;  // Si current->twin est nullptr, on sort de la boucle.
        }

        iteration++;
        if (iteration >= maxIterations) {
            std::cout << "Warning: Exceeded max iterations in computeNormal(). Exiting loop." << std::endl;
            break;
        }

    } while (current != start);

    // Average the normal.
    if (faceCount > 0) {
        accumulatedNormal = accumulatedNormal / static_cast<float>(faceCount); // Utilisez l'opérateur de division surchargé de myVector3D
    }

    // Normaliser la normale.
    accumulatedNormal.normalize();

    // Mettez à jour la normale du sommet
    *normal = accumulatedNormal;
}



