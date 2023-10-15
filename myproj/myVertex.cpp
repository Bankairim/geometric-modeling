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
    if (!originof) return; // Si aucune halfedge d'origine, retournez.

    myVector3D accumulatedNormal(0.0, 0.0, 0.0);
    int faceCount = 0;

    // Commencer par la halfedge d'origine.
    myHalfedge* start = originof;
    myHalfedge* current = start;
    int maxIterations = 2000;  // Choisissez une valeur raisonnable pour votre maillage.
    int iteration = 0;

    do {
        // V�rifier les pointeurs avant de les d�r�f�rencer.
        std::cout << "Nous entrons dans la condition myVertex" << std::endl;
        if (current && current->adjacent_face && current->adjacent_face->normal) {
            std::cout << "condition adjacent face normal v�rifi�e. Il existe une normale" << std::endl;
            accumulatedNormal += *(current->adjacent_face->normal); // Utilisez l'op�rateur d'addition surcharg� de myVector3D
            faceCount++;
        }

        if (current && current->twin) {
            current = current->twin->next;
        }
        else {
            break;  // Si current->twin est nullptr, sortez de la boucle.
        }

        iteration++;
        if (iteration >= maxIterations) {
            std::cout << "Warning: Exceeded max iterations in computeNormal(). Exiting loop." << std::endl;
            break;
        }

    } while (current != start);

    // Average the normal.
    if (faceCount > 0) {
        accumulatedNormal = accumulatedNormal / static_cast<float>(faceCount); // Utilisez l'op�rateur de division surcharg� de myVector3D
    }

    // Normaliser la normale.
    accumulatedNormal.normalize();

    // Mettez � jour la normale du sommet
    *normal = accumulatedNormal; // Assurez-vous que "normal" est bien un pointeur vers myVector3D
}



