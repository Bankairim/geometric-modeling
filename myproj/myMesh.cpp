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
	// Parcourir toutes les  faces et calculer leurs normales
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
	
	std::vector<myFace*> toRemove;
	for (myFace* f : faces)
	{

		if (triangulate(f)) {
			toRemove.push_back(f);
		}
	}
	for (int i = 0; i < toRemove.size(); i++)
	{
		faces.erase(remove(faces.begin(), faces.end(), toRemove[i]),faces.end());
	}
	std::cout << " Triangulated faces : " << faces.size() << endl;

	int triangles = 0;
	int quads = 0;
	int pentagons = 0;
	int hexagons = 0;
	int septagons = 0;
	int octagons = 0;

	for (myFace* f : faces) {
		int vertexCount = 0;
		myHalfedge* startEdge = f->adjacent_halfedge;
		myHalfedge* currentEdge = startEdge;
		do {
			vertexCount++;
			currentEdge = currentEdge->next;
		} while (currentEdge != startEdge);

		switch (vertexCount) {
		case 3: triangles++; break;
		case 4: quads++; break;
		case 5: pentagons++; break;
		case 6: hexagons++; break;
		case 7: septagons++; break;
		case 8: octagons++; break;
			// Add more cases if needed
		}
	}

	// Print the results
	std::cout << "Triangles: " << triangles << std::endl;
	std::cout << "Quads: " << quads << std::endl;
	std::cout << "Pentagons: " << pentagons << std::endl;
	std::cout << "Hexagons: " << hexagons << std::endl;
	std::cout << "Septagons: " << septagons << std::endl;
	std::cout << "Octagons: " << octagons << std::endl;
	checkMesh();
}

bool myMesh::triangulate(myFace* f) {
	
	std::vector<myVertex*> faceVertices;
	myHalfedge* startEdge = f->adjacent_halfedge;
	myHalfedge* currentEdge = startEdge;
	do { //Algorithme de parcours de face
		faceVertices.push_back(currentEdge->source);
		currentEdge = currentEdge->next;
	} while (currentEdge != f->adjacent_halfedge);
	if (faceVertices.size() <= 3)return false;
	// Calcul du centre de la face
	myPoint3D center(0, 0, 0);
	for (myVertex* v : faceVertices) {
		center += *(v->point);
	}
	center /= faceVertices.size();

	// Création du nouveau sommet pour le centre
	myVertex* centerVertex = new myVertex();
	centerVertex->point = new myPoint3D(center.X, center.Y, center.Z);
	vertices.push_back(centerVertex); // Ajoutez le nouveau sommet à la liste des sommets

	// Pour un polygone convexe, chaque paire consécutive de sommets avec le centre forme un triangle.
	for (size_t i = 0; i < faceVertices.size() - 1; ++i) {
		myVertex* A = centerVertex;
		myVertex* B = faceVertices[i];
		myVertex* C = faceVertices[i + 1];

		// Créez un triangle avec les sommets A, B, et C
		myFace* newTriangle = new myFace();

		myHalfedge* AB = new myHalfedge();
		myHalfedge* BC = new myHalfedge();
		myHalfedge* CA = new myHalfedge();

		AB->source = A;
		BC->source = B;
		CA->source = C;

		AB->next = BC;
		BC->next = CA;
		CA->next = AB;

		AB->prev = CA;
		BC->prev = AB;
		CA->prev = BC;

		AB->adjacent_face = newTriangle;
		BC->adjacent_face = newTriangle;
		CA->adjacent_face = newTriangle;

		newTriangle->adjacent_halfedge = AB;
		// Associer les twins
		if (i > 0) { // Si ce n'est pas la première itération
			myHalfedge* previousCA = halfedges.back(); // La dernière demi-arête ajoutée lors de l'itération précédente
			previousCA->twin = AB;
			AB->twin = previousCA;

			myHalfedge* previousBC = halfedges[halfedges.size() - faceVertices.size() + i - 1];
			previousBC->twin = CA;
			CA->twin = previousBC;

			if (i < faceVertices.size() - 2) { // If it's not the second last iteration
				myHalfedge* nextBC = halfedges[halfedges.size() - faceVertices.size() + i + 1];
				BC->twin = nextBC;
				nextBC->twin = BC;
			}

		}
		// Ajoutez le nouveau triangle à la liste des faces
		faces.push_back(newTriangle);

		// Ajoutez les nouvelles demi-arêtes à la liste des demi-arêtes
		halfedges.push_back(AB);
		halfedges.push_back(BC);
		halfedges.push_back(CA);
		
	}
	// Ajoutez le dernier triangle avec le dernier sommet et le centre
	myVertex* A = centerVertex;
	myVertex* B = faceVertices.back(); // Dernier sommet
	myVertex* C = faceVertices.front(); // Premier sommet

	myFace* newTriangle = new myFace();

	myHalfedge* AB = new myHalfedge();
	myHalfedge* BC = new myHalfedge();
	myHalfedge* CA = new myHalfedge();

	AB->source = A;
	BC->source = B;
	CA->source = C;

	AB->next = BC;
	BC->next = CA;
	CA->next = AB;

	AB->prev = CA;
	BC->prev = AB;
	CA->prev = BC;

	AB->adjacent_face = newTriangle;
	BC->adjacent_face = newTriangle;
	CA->adjacent_face = newTriangle;

	newTriangle->adjacent_halfedge = AB;

	myHalfedge* firstAB = halfedges[halfedges.size() - 2 * faceVertices.size()]; // Le premier AB ajouté
	myHalfedge* lastCA = halfedges.back(); // La dernière demi-arête ajoutée
	firstAB->twin = lastCA;
	lastCA->twin = firstAB;
	// Ajoutez le dernier triangle à la liste des faces
	faces.push_back(newTriangle);

	// Ajoutez les nouvelles demi-arêtes à la liste des demi-arêtes
	halfedges.push_back(AB);
	//halfedges.push_back(BC);
	halfedges.push_back(CA);
	std::cout << "checkMesh after triangulation of face " << std::endl;
	checkMesh();
	return true;
}

//bool myMesh::triangulate(myFace* f) {
//	std::vector<myVertex*> faceVertices;
//	myHalfedge* startEdge = f->adjacent_halfedge;
//	myHalfedge* currentEdge = startEdge;
//	do { 
//		faceVertices.push_back(currentEdge->source);
//		currentEdge = currentEdge->next;
//	} while (currentEdge != startEdge);
//
//	// Pour un polygone convexe, chaque triplet de sommets consécutifs forme une oreille.
//	for (size_t i = 0; i < faceVertices.size() - 2; ++i) {
//		myVertex* A = faceVertices[i];
//		myVertex* B = faceVertices[i + 1];
//		myVertex* C = faceVertices[i + 2];
//
//		myFace* newTriangle = new myFace();
//
//		myHalfedge* AB = new myHalfedge();
//		myHalfedge* BC = new myHalfedge();
//		myHalfedge* CA = new myHalfedge();
//
//		AB->source = A;
//		BC->source = B;
//		CA->source = C;
//
//		AB->next = BC;
//		BC->next = CA;
//		CA->next = AB;
//
//		AB->prev = CA;
//		BC->prev = AB;
//		CA->prev = BC;
//
//		AB->adjacent_face = newTriangle;
//		BC->adjacent_face = newTriangle;
//		CA->adjacent_face = newTriangle;
//
//		newTriangle->adjacent_halfedge = AB;
//
//		// Ajoutez le nouveau triangle à la liste des faces
//		faces.push_back(newTriangle);
//
//		// Ajoutez les nouvelles demi-arêtes à la liste des demi-arêtes
//		//halfedges.push_back(AB);
//		halfedges.push_back(BC);
//		halfedges.push_back(CA);
//	}
//
//	return true;
//}