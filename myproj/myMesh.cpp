#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"
#include <myFace.h>

using namespace std;

myMesh::myMesh(void) {
    //todo
}



myMesh::~myMesh(void) {
    //todo
}


void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex *> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge *> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace *> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge *>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
}


// Fonction pour lire un fichier .obj et stocker ses données dans la structure halfedge.
bool myMesh::readFile(std::string filename)
{
	// Déclaration de variables pour stocker les lignes du fichier, les types d'éléments (comme "v" pour les sommets ou "f" pour les faces) et les données temporaires.
	string s, t, u;
	vector<int> faceids; // Pour stocker les identifiants des sommets des faces.
	myHalfedge** hedges; // Pointeur vers un tableau de halfedges.

	// Ouvrir le fichier .obj pour la lecture.
	ifstream fin(filename);
	// Si le fichier ne peut pas être ouvert, afficher un message d'erreur et retourner false.
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	// Stocker le nom du fichier pour une utilisation ultérieure.
	name = filename;

	// Créer une map pour stocker les halfedges et leurs jumeaux (twins).
	map<pair<int, int>, myHalfedge*> twin_map;
	map<pair<int, int>, myHalfedge*>::iterator it;

	// Lire le fichier ligne par ligne.
	while (getline(fin, s))
	{
		// Utiliser un stringstream pour extraire les données de la ligne.
		stringstream myline(s);
		myline >> t; // Extraire le type d'élément (comme "v" ou "f").

		// Si l'élément est un groupe ("g"), ne rien faire.
		if (t == "g") {}
		// Si l'élément est un sommet ("v").
		else if (t == "v")
		{
			float x, y, z;
			// Extraire les coordonnées x, y et z du sommet.
			myline >> x >> y >> z;
			// Afficher les coordonnées pour le débogage.
			cout << "v " << x << " " << y << " " << z << endl;
			myPoint3D* Pt = new myPoint3D(x, y, z);
			myVertex* v = new myVertex();
			v->point = Pt;
			vertices.push_back(v);

		}
		else if (t == "mtllib") {}

		else if (t == "usemtl") {}

		else if (t == "s") {}

		else if (t == "f")
		{
			faceids.clear();
			while (myline >> u) // read indices of vertices from a face into a container - it helps to access them later 
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);
			if (faceids.size() < 3) // ignore degenerate faces
				continue;

			hedges = new myHalfedge * [faceids.size()]; // allocate the array for storing pointers to half-edges
			for (unsigned int i = 0; i < faceids.size(); i++)
				hedges[i] = new myHalfedge(); // pre-allocate new half-edges

			myFace* f = new myFace(); // allocate the new face
			f->adjacent_halfedge = hedges[0]; // connect the face with incident edge
			

			for (unsigned int i = 0; i < faceids.size(); i++)
			{
				int iplusone = (i + 1) % faceids.size();
				int iminusone = (i - 1 + faceids.size()) % faceids.size();

				// YOUR CODE COMES HERE!

				// connect prevs, and next
				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];
				hedges[i]->adjacent_face = f;


				// search for the twins using twin_map
				pair<int, int> edgeKey = make_pair(faceids[iplusone], faceids[i]);
				it = twin_map.find(edgeKey);
				if (it != twin_map.end()) {
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				}
				else {
					twin_map[make_pair(faceids[i], faceids[iplusone])] = hedges[i];
				}

				// set originof
				vertices[faceids[i]]->originof = hedges[i];
				hedges[i]->source = vertices[faceids[i]];

				// push edges to halfedges in myMesh
				halfedges.push_back(hedges[i]);

			}
			// push faces to faces in myMesh
			faces.push_back(f);

			cout << "f";
			// Extraire les identifiants des sommets de la face.
			while (myline >> u) cout << " " << atoi((u.substr(0, u.find("/"))).c_str());
			cout << endl;
		}
	}

	// Vérifier la cohérence du maillage.
	checkMesh();
	// Normaliser les coordonnées des sommets.
	normalize();

	// Retourner true pour indiquer que la lecture a réussi.
	return true;
}



void myMesh::computeNormals()
{
	// Parcourir toutes les faces et calculer leurs normales
	for (myFace* f : faces)
	{
		f->computeNormal();
	}
	// Vérifier si 'vertices' est non vide avant d'appeler computeNormal
	if (!vertices.empty()) {
		// Parcourir tous les sommets et calculer leurs normales
		for (myVertex* v : vertices)
		{
			//A ce stade la, v est censé avoir un point et un originof du readfile et va obtenir une normal basée sur son originof.
			if (v) v->computeNormal();
		}
	}


}


void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}


void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
	/**** TODO ****/
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}


void myMesh::triangulate() {
	std::vector<myFace*> triangles;

	// Création d'une copie de la liste des faces pour éviter les modifications pendant l'itération
	std::vector<myFace*> originalFaces = faces;

	for (myFace* f : originalFaces) {
		if (!triangulate(f)) {
			std::vector<myVertex*> vertices;
			myHalfedge* startEdge = f->adjacent_halfedge;
			myHalfedge* currentEdge = startEdge;
			do {
				vertices.push_back(currentEdge->source);
				currentEdge = currentEdge->next;
			} while (currentEdge != startEdge);

			myVector3D referenceVector(0, 0, 1);
			bool isFaceUpwards = (*f->normal) * referenceVector > 0;

			for (size_t i = 0; i < vertices.size(); ++i) {
				myVertex* A = vertices[i];
				myVertex* B = vertices[(i + 1) % vertices.size()];
				myVertex* C = vertices[(i + 2) % vertices.size()];

				if (isFaceUpwards) {
					bool isEar = true;

					for (myVertex* vertex : vertices) {
						if (vertex != A && vertex != B && vertex != C) {
							// Vérification si le point est dans le triangle
							myVector3D v0 = *(C->point) - *(A->point);
							myVector3D v1 = *(B->point) - *(A->point);
							myVector3D v2 = *(vertex->point) - *(A->point);

							float d00 = v0 * v0;
							float d01 = v0 * v1;
							float d11 = v1 * v1;
							float d20 = v2 * v0;
							float d21 = v2 * v1;
							float denom = d00 * d11 - d01 * d01;
							float alpha = (d11 * d20 - d01 * d21) / denom;
							float beta = (d00 * d21 - d01 * d20) / denom;
							float gamma = 1.0f - alpha - beta;
							if (alpha >= 0 && beta >= 0 && gamma >= 0) {
								isEar = false;
								break;
							}
						}
					}

					if (isEar) {
						// Créer un nouveau triangle avec les sommets A, B, et C
						myFace* newTriangle = new myFace();

						// Créer des demi-arêtes pour le nouveau triangle
						myHalfedge* he1 = new myHalfedge();
						myHalfedge* he2 = new myHalfedge();
						myHalfedge* he3 = new myHalfedge();

						he1->source = A;
						he2->source = B;
						he3->source = C;

						he1->next = he2;
						he2->next = he3;
						he3->next = he1;

						newTriangle->adjacent_halfedge = he1;

						triangles.push_back(newTriangle); // Ajouter le triangle à la liste des triangles

						myHalfedge* heExitingB = B->originof;
						myHalfedge* heBeforeB = heExitingB->prev;
						myHalfedge* heAfterB = heExitingB->next;

						heBeforeB->next = heAfterB;
						heAfterB->prev = heBeforeB;

						// Libérer la mémoire associée à la demi-arête sortant de B
						delete heExitingB;

						// Si vous ne prévoyez pas de réutiliser le sommet B ailleurs, vous pouvez également le supprimer
						// delete B;

						break;
					}

				}
			}
		}
	}

	// Ajouter les nouveaux triangles à la liste des faces
	faces.insert(faces.end(), triangles.begin(), triangles.end());
}







bool myMesh::triangulate(myFace* f) {
	// Comptez le nombre de sommets dans la face
	int counter = 0;
	myHalfedge* adj = f->adjacent_halfedge;
	myHalfedge* start = adj;

	do {
		adj = adj->next;
		++counter;
	} while (adj != start);

	// Si la face a déjà 3 sommets (c'est-à-dire qu'elle est déjà un triangle), rien à faire
	return counter<=3; 


}




