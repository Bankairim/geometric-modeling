#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"
#include <vector>
#include <algorithm>
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

//Detect twins inconsistency. 
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

//Detects lack of twins, inconsistent twins and isolated halfedges.
bool myMesh::checkMeshAdvanced()
{
	bool isMeshValid = true;

	// A helper lambda function to check if a face is isolated
	auto isIsolatedFace = [](myFace* f) -> bool {
		myHalfedge* startEdge = f->adjacent_halfedge;
		myHalfedge* currentEdge = startEdge;
		do {
			if (currentEdge->twin != nullptr) {
				return false; // Found a twin, not isolated
			}
			currentEdge = currentEdge->next;
		} while (currentEdge != startEdge);
		return true; // No twins found, face is isolated
		};

	for (auto& he : halfedges) {
		if (he->twin == nullptr) {
			cout << "Error! Half-edge from vertex " << he->source->index;
			if (he->next) {
				cout << " to vertex " << he->next->source->index;
			}
			else {
				cout << " to an unknown vertex";
			}
			cout << " lacks a twin." << endl;
			isMeshValid = false;
		}
		else if (he->twin->twin != he) {
			cout << "Error! Inconsistent twin: Half-edge from vertex " << he->source->index;
			if (he->next) {
				cout << " to vertex " << he->next->source->index;
			}
			else {
				cout << " to an unknown vertex";
			}
			cout << " has an inconsistent twin. ";
			if (he->twin) {
				cout << "Twin goes from vertex " << he->twin->source->index;
				if (he->twin->next) {
					cout << " to vertex " << he->twin->next->source->index;
				}
				else {
					cout << " to an unknown vertex";
				}
			}
			else {
				cout << "Twin is missing.";
			}
			// Additional diagnostics
			cout << " | Adjacent face of this half-edge: " << (he->adjacent_face ? "Present" : "Missing");
			cout << " | Adjacent face of twin half-edge: " << (he->twin && he->twin->adjacent_face ? "Present" : "Missing");
			
			isMeshValid = false;
			cout << endl;
		}

	}
	if (isMeshValid) {
		cout << "Each edge has a twin and the twin relationship is mutual." << endl;
	}
	else {
		cout << "There are issues with edge twins in the mesh." << endl;
	}

	return isMeshValid;
}

void myMesh::checkVertice(myVertex* v) {
	if (!v) {
		std::cout << "Le vertex est NULL." << std::endl;
		return;
	}

	if (!v->originof) {
		std::cout << "Vertex " << v->index << ": originof est NULL." << std::endl;
		return;
	}

	myHalfedge* start = v->originof;
	myHalfedge* current = start;
	int maxIterations = 2000;
	int iteration = 0;

	do {
		if (!current) {
			std::cout << "Vertex " << v->index << ": Demi-arête actuelle est NULL." << std::endl;
			break;
		}

		if (!current->twin) {
			std::cout << "Vertex " << v->index << ": Demi-arête n'a pas de jumeau." << std::endl;
		}
		else if (current->twin->twin != current) {
			std::cout << "Vertex " << v->index << ": Inconsistance de jumeau detectee." << std::endl;
		}

		if (!current->adjacent_face) {
			std::cout << "Vertex " << v->index << ": Pas de face adjacente à la demi-arête actuelle." << std::endl;
		}

		current = current->twin ? current->twin->next : nullptr;
		iteration++;
		if (iteration >= maxIterations) {
			std::cout << "Vertex " << v->index << ": Attention, dépassement du nombre maximum d'itérations dans checkVertice()." << std::endl;
			break;
		}
	} while (current && current != start);
}

// Fonction pour lire un fichier .obj et stocker ses données dans la structure halfedge.
bool myMesh::readFile(std::string filename)
{
	// Déclaration de variables pour stocker les lignes du fichier, les types d'éléments (comme "v" pour les sommets ou "f" pour les faces) et les données temporaires.
	string s, t, u;
	vector<int> faceids; // Pour stocker les identifiants des sommets des faces.
	myHalfedge** hedges; // Pointeur vers le tableau de halfedges.

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
	//map<pair<int, int>, myHalfedge*> twin_map; 
	map<pair<int, int>, myHalfedge*>::iterator it;
	int verticeNumber = 0;
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
			cout << "v " << x << " " << y << " " << z << " index : " << verticeNumber << endl;
			myPoint3D* Pt = new myPoint3D(x, y, z);
			myVertex* v = new myVertex();
			v->point = Pt;

			// 23/11:2023 : Set the index of the vertex
			v->index = vertices.size() -1;
			vertices.push_back(v);
			verticeNumber++;
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
			{
				hedges[i] = new myHalfedge(); // pre-allocate new half-edges
				// Set the source vertex of the half-edge
				hedges[i]->source = vertices[faceids[i]];

				// Debug print to check the source index
				//cout << "Hedges[" << i << "] -> source-> index = " << hedges[i]->source->index << endl;
			}
				

			myFace* f = new myFace(); // allocate the new face
			f->index = faces.size();
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
				//set index. 
				hedges[i]->index = i;

				// search for the twins using twin_map
				pair<int, int> edgeKey = make_pair(faceids[iplusone], faceids[i]);
				it = twinMap.find(edgeKey);
				if (it != twinMap.end()) {
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				}
				else {
					twinMap[make_pair(faceids[i], faceids[iplusone])] = hedges[i];
				}

				// set originof
				vertices[faceids[i]]->originof = hedges[i];
				hedges[i]->source = vertices[faceids[i]];

				// push edges to halfedges in myMesh
				halfedges.push_back(hedges[i]);
				//cout << "Hedges[" << i << "] -> source-> index = " << hedges[i]->source->index << endl;
				
			}
			// push faces to faces in myMesh
			faces.push_back(f);

			//cout << "f";
			// Extraire les identifiants des sommets de la face.
			while (myline >> u) cout << " " << atoi((u.substr(0, u.find("/"))).c_str());
			//cout << endl;
			
		}
	}
	cout << " Readfile terminé. Vertices size : " << vertices.size() << endl;
	//clearTwinRelationships();
	//establishTwinRelationships();
	// Vérifier la cohérence du maillage.
	//checkMeshAdvanced();
	// Normaliser les coordonnées des sommets.
	//checkHalfEdgeReferences();
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
		cout << "Vertices size : " << vertices.size() << endl;
		for (int i=0;i < vertices.size();i++)
		{
			//A ce stade la, v est censé avoir un point et un originof du readfile et va obtenir une normal basée sur son originof.
			//cout << "vertices[" << i << "] " << endl;
			//checkVertice(vertices[i]);
			if (vertices[i]) vertices[i]->computeNormal();
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


void myMesh::clearTwinRelationships() {
	for (auto& he : halfedges) {
		if (he) {
			he->twin = nullptr;
		}
	}
}

//void myMesh::establishTwinRelationships() {
//	std::map<std::pair<int, int>, myHalfedge*> edgeMap;
//
//	for (std::size_t i = 0;i<halfedges.size();i++) {
//		if (halfedges[i]) {
//			std::pair<int, int> edgeKey(halfedges[i]->source->index, halfedges[i]->next->source->index);
//			std::pair<int, int> twinKey(halfedges[i]->next->source->index, halfedges[i]->source->index);
//
//			// Check if the twin is already in the map
//			if (edgeMap.count(twinKey)) {
//				//cout << "Twin déjà existant" << endl;
//				// Establish the twin relationship
//				halfedges[i]->twin = edgeMap[twinKey];
//				edgeMap[twinKey]->twin = halfedges[i];
//			}
//			else {
//				// Add the half-edge to the map for future twin finding
//				edgeMap[edgeKey] = halfedges[i];
//			}
//		}
//	}
//
//	// search for the twins using twin_map
//}

void myMesh::testTriangulate()
{
	triangulate();
	checkMeshAdvanced();
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
}
void myMesh::checkHalfEdgeReferences() {
	cout << "Checking Half-Edge References..." << endl;
	bool hasIssue = false;

	for (auto& he : halfedges) {
		if (!he) continue;  // Skip if the half-edge is null

		// Check Next Reference
		if (!he->next || he->next->source == he->source) {
			cout << "Issue with Next Reference: Half-edge from " << he->source->index << " lacks a proper next reference." << endl;
			hasIssue = true;
		}

		// Check Prev Reference
		if (!he->prev || he->prev->next != he) {
			cout << "Issue with Prev Reference: Half-edge from " << he->source->index << " lacks a proper prev reference." << endl;
			hasIssue = true;
		}

		// Check Twin Reference
		if (he->twin && (he->twin->twin != he || he->twin->source == he->source)) {
			cout << "Issue with Twin Reference: Half-edge from " << he->source->index << " has an inconsistent twin." << endl;
			hasIssue = true;
		}
	}

	if (!hasIssue) {
		cout << "All half-edge references are correct." << endl;
	}
}

void myMesh::triangulate() {

	std::vector<myFace*> toRemove;
	int faceNumber = faces.size();
	cout << "number of faces (face.size()) : " << faces.size() << endl;
	for (int i = 0; i < faceNumber; i++)
	{
		//cout << "Starting triangulate " << i << "." << endl;
		myFace* f = faces[i];
		if (triangulate(f)) {
			toRemove.push_back(f);
		}
		//cout << "End of triangulate " << i << "." << endl;
	}
	for (int i = 0; i < toRemove.size(); i++)
	{
		faces.erase(remove(faces.begin(), faces.end(), toRemove[i]), faces.end());
	}
	std::cout << " Triangulated faces : " << faces.size() << endl;

	checkMeshAdvanced();
	//checkHalfEdgeReferences();
}
bool myMesh::triangulate(myFace* f) {
	myVertex* center = createCenterVertex(f);  // Calculer le point central de la face
	vertices.push_back(center);  // Ajouter le point central à la liste des sommets

	myHalfedge* startEdge = f->adjacent_halfedge;  // Demi-arête de départ pour la face
	myHalfedge* currentEdge = startEdge;  // Demi-arête courante
	myHalfedge* nextEdge = currentEdge;  // Demi-arête suivante
	myHalfedge* twinEdge = nullptr;  // Demi-arête jumelle pour la demi-arête allant du centre vers un sommet

	do {
		nextEdge = nextEdge->next;  // Passer à la demi-arête suivante

		// Création d'une nouvelle face pour le triangle
		myFace* triangleFace = new myFace();
		triangleFace->adjacent_halfedge = currentEdge;
		faces.push_back(triangleFace);  // Ajouter la nouvelle face à la liste des faces

		// Création des deux nouvelles demi-arêtes pour le triangle
		myHalfedge* edge1 = new myHalfedge();  // Demi-arête allant d'un sommet de la face vers le centre
		myHalfedge* edge2 = new myHalfedge();  // Demi-arête allant du centre vers le sommet suivant de la face

		// Configuration de edge1
		edge1->source = currentEdge->next->source;
		edge1->adjacent_face = triangleFace;
		edge1->source->originof = edge1;

		// Configuration de edge2
		edge2->source = center;
		edge2->adjacent_face = triangleFace;
		center->originof = edge2;

		// Connexion des demi-arêtes pour former le triangle
		currentEdge->next = edge1;
		edge1->next = edge2;
		edge2->next = currentEdge;

		currentEdge->prev = edge2;
		edge1->prev = currentEdge;
		edge2->prev = edge1;


		// Ajouter les nouvelles demi-arêtes à la liste des demi-arêtes
		halfedges.push_back(edge1);
		halfedges.push_back(edge2);

		// Configuration des twins (jumelles)
		if (twinEdge != nullptr) {
			edge2->twin = twinEdge;
			twinEdge->twin = edge2;
		}
		twinEdge = edge1;  // edge1 sera le twin de la demi-arête allant du centre vers un sommet pour le triangle suivant

		// Passer à la demi-arête suivante
		currentEdge = nextEdge;
	} while (currentEdge != startEdge);

	// Connecter le twin de la dernière demi-arête créée
	startEdge->prev->twin = twinEdge;
	twinEdge->twin = startEdge->prev;
	return true;
}



myVertex* myMesh::createCenterVertex(const std::vector<myVertex*>& faceVertices) {
	myPoint3D center(0, 0, 0);
	for (myVertex* v : faceVertices) {
		center += *(v->point);
	}
	center /= faceVertices.size();

	myVertex* centerVertex = new myVertex();
	centerVertex->point = new myPoint3D(center.X, center.Y, center.Z);

	// Assign the next available index to the new vertex
	centerVertex->index = vertices.size() -1;

	vertices.push_back(centerVertex);

	//cout << "Created Center Vertex with Index: " << centerVertex->index << endl;

	return centerVertex;
}

myVertex* myMesh::createCenterVertex(myFace* f) {
	myPoint3D center(0, 0, 0);
	int count = 0;

	myHalfedge* edge = f->adjacent_halfedge;
	do {
		center += *(edge->source->point);
		count++;
		edge = edge->next; // Parcourir les demi-arêtes de la face
	} while (edge != f->adjacent_halfedge);

	if (count > 0) {
		center /= count;
	}

	myVertex* centerVertex = new myVertex();
	centerVertex->point = new myPoint3D(center.X, center.Y, center.Z);

	// Assign the next available index to the new vertex
	centerVertex->index = vertices.size();

	vertices.push_back(centerVertex);

	//cout << "Created Center Vertex with Index: " << centerVertex->index << endl;

	return centerVertex;
}


void myMesh::testHalfEdgeProperties(std::vector<myHalfedge*>& halfedges) {
	// Function to display half-edge properties
	auto displayHalfEdgeProperties = [](myHalfedge* he) {
		if (he && he->source && he->next && he->next->source) {
			cout << "Half-edge from vertex he->source->index " << he->source->index << " to vertex he->next->source->index " << he->next->source->index;
			if (he->twin) {
				cout << " | Twin from vertex  he->twin->source->index " << he->twin->source->index << " to vertex he->twin->next->source->index " << he->twin->next->source->index;
			}
			else {
				cout << " | No twin";
			}
			cout << endl;
		}
		};

	cout << "Half-edges before twin assignment:" << endl;
	for (auto& he : halfedges) {
		displayHalfEdgeProperties(he);
	}

	// Correcting twin assignments
	/*correctTwinAssignments(halfedges);*/

	cout << "\nHalf-edges after twin assignment:" << endl;
	for (auto& he : halfedges) {
		displayHalfEdgeProperties(he);
	}
}

void myMesh::displayHalfEdgeProperties(myHalfedge* he) {
	if (!he) {
		cout << "Half-edge is null." << endl;
		return;
	}

	cout << "Half-edge properties: " << endl;

	// Display source vertex
	if (he->source) {
		cout << "  Source Vertex: " << he->source->index << endl;
	}
	else {
		cout << "  Source Vertex: null" << endl;
	}

	// Display destination vertex (next half-edge's source)
	if (he->next && he->next->source) {
		cout << "  Destination Vertex: " << he->next->source->index << endl;
	}
	else {
		cout << "  Destination Vertex: null" << endl;
	}

	// Display twin's source and destination vertices
	if (he->twin) {
		if (he->twin->source) {
			cout << "  Twin's Source Vertex: " << he->twin->source->index;
		}
		else {
			cout << "  Twin's Source Vertex: null";
		}
		if (he->twin->next && he->twin->next->source) {
			cout << ", Twin's Destination Vertex: " << he->twin->next->source->index << endl;
		}
		else {
			cout << ", Twin's Destination Vertex: null" << endl;
		}
	}
	else {
		cout << "  No twin for this half-edge." << endl;
	}
}


bool checkTriangulate(myFace* f) {
	myHalfedge* startEdge = f->adjacent_halfedge;
	myHalfedge* currentEdge = startEdge;
	int count = 0;//Number of halfedges
	do {
		count++;
		currentEdge = currentEdge->next;
	} while (currentEdge != startEdge);

	return(count == 3);
}

myPoint3D* myMesh::bestPosition(myPoint3D* p1, myPoint3D* p2) {
	return new myPoint3D((p1->X + p2->X) / 2.0, (p1->Y + p2->Y) / 2.0, (p1->Z + p2->Z) / 2.0);
}
// Fonction pour calculer la moyenne de quatre points
myPoint3D* myMesh::averageOfFourPoints(myPoint3D* p1, myPoint3D* p2, myPoint3D* f1, myPoint3D* f2) {
	return new myPoint3D((p1->X + p2->X + f1->X + f2->X) / 4.0,
		(p1->Y + p2->Y + f1->Y + f2->Y) / 4.0,
		(p1->Z + p2->Z + f1->Z + f2->Z) / 4.0);
}

double distance(myPoint3D* p1, myPoint3D* p2) {
	//first thought, find the middle of v1 and v2
	//cout << "passe distance";
	return  sqrt(pow(p2->X - p1->X, 2) + pow(p2->Y - p1->Y, 2) + pow(p2->Z - p1->Z, 2));
}

myHalfedge* myMesh::findMinimalHalfedge() {
	if (halfedges.empty()) {
		return nullptr; // No halfedge in the mesh
	}
	double min = std::numeric_limits<double>::max();
	myHalfedge* minEdge = nullptr;

	for (myHalfedge* he : halfedges) {
		if (he && he->source && he->next && he->next->source) {
			double currentDistance = distance(he->source->point, he->next->source->point);
			if (currentDistance < min) {
				min = currentDistance;
				minEdge = he;
			}
		}
	}
	//std::cout << "MinimalHalfedge = " << minEdge->index << ". " << endl;

	return minEdge;
}



void myMesh::displayAllHalfEdgeProperties() {
	for (auto& he : halfedges) {
		if (he) {
			he->displayProperties();
		}
	}
}



bool myMesh::collapse(myHalfedge* e) {
	//myHalfedge* e = findMinimalHalfedge();
	if (!e || !e->twin) return false; // Vérifier la présence de l'halfedge et de son jumeau
	myVertex* newVertex = new myVertex();
	myVertex* v1 = new myVertex();
	myVertex* v2 = new myVertex();
	v1 = e->source;
	v2 = e->next->source;
	newVertex->point = bestPosition(v1->point, v2->point);
	vertices.push_back(newVertex);
	vector<myFace*> deleteFaces;
	vector<myHalfedge*>deleteHalfedge;

	for each (myHalfedge * he in halfedges)
	{
		//Find the two faces connected with halfedge;
		if (he == e || he == e->twin) {
			if (checkTriangulate(he->adjacent_face)) {
				he->next->twin->twin = he->prev->twin;
				deleteFaces.push_back(he->adjacent_face);
				deleteHalfedge.push_back(he->next);
				deleteHalfedge.push_back(he->prev);
				deleteHalfedge.push_back(he);
			}
			else {
				he->next->prev = he->prev;
				he->prev->next = he->next;
				deleteHalfedge.push_back(he);
			}
		}
		if (he->source == v1 || he->source == v2) {
			he->source = newVertex;
			newVertex->originof = he;
		}
	}
	for each (myFace * f in faces) {
		if (f->adjacent_halfedge == e) {
			f->adjacent_halfedge = e->next;
		}
	}
	// Effacer v1 si présent
	auto itV1 = std::remove(vertices.begin(), vertices.end(), v1);
	if (itV1 != vertices.end()) {
		vertices.erase(itV1, vertices.end());
	}

	// Effacer v2 si présent
	auto itV2 = std::remove(vertices.begin(), vertices.end(), v2);
	if (itV2 != vertices.end()) {
		vertices.erase(itV2, vertices.end());
	}



	for (int m = 0; m < deleteHalfedge.size(); m++) {
		halfedges.erase(remove(halfedges.begin(), halfedges.end(), deleteHalfedge[m]));
	}
	if (deleteFaces.size() != 0) {
		for (int j = 0; j < deleteFaces.size(); j++) {
			faces.erase(remove(faces.begin(), faces.end(), deleteFaces[j]));
		}
	}
	computeNormals();
	return true;
}

bool myMesh::collapse()
{
	myHalfedge* e = findMinimalHalfedge();
	return(collapse(e));
}

myVertex* myMesh::computeFacePoints(myFace* f) {
	// Étape 1 : ajoute un point pour chaque face.
	myVertex* facePt = new myVertex();
	facePt->point = new myPoint3D();
	myPoint3D sum;
	int vertexCount = 0;

	myHalfedge* start = f->adjacent_halfedge;
	myHalfedge* current = start;
	do {
		*(facePt->point) += *(current->source->point);
		vertexCount++;
		current = current->next;
	} while (current != start);

	if (vertexCount > 0) {
		*(facePt->point) /= vertexCount;
	}
	return facePt;
}


void myMesh::subdivisionCatmullClark() { 
	for (myFace* f : faces) {
		f->facePoint = computeFacePoints(f); // Utilise l'attribut facePoint
	}
	//// Étape 2: Pour chaque arête, ajouter un point d'arête.
	//std::map<std::pair<myVertex*, myVertex*>, myPoint3D> edgePoints;
	for (myHalfedge* he : halfedges) {
		if (he->edgePoint == nullptr && he->twin->edgePoint == nullptr) {
			he->edgePoint = new myVertex(); // Utilise l'attribut edgePoint
			he->edgePoint->point = new myPoint3D();

			// Deux extrémités (M, E) de l'arête
			myPoint3D M = *(he->source->point);
			myPoint3D E = *(he->twin->source->point);

			if (he->adjacent_face && he->twin->adjacent_face) {
				// Points de face des faces adjacentes
				myPoint3D A = *(he->adjacent_face->facePoint->point); // Utilise l'attribut facePoint de myFace
				myPoint3D F = *(he->twin->adjacent_face->facePoint->point);

				*(he->edgePoint->point) = (A + F + M + E) / 4;
			}

			he->twin->edgePoint = he->edgePoint; // Duplique le point pour l'arête jumelle
		}
	}

	// Mise à jour de chaque point original (P)
	for (myVertex* v : vertices) {
		myPoint3D* originalPos = new myPoint3D(*(v->point));
		myPoint3D* faceAverage = new myPoint3D(); // Moyenne de tous les points de face pour les faces touchant P
		myPoint3D* edgeAverage = new myPoint3D(); // Moyenne de tous les milieux d'arête pour les arêtes originales touchant P

		int n = 0; // Nombre de faces adjacentes et aussi d'arêtes voisines de P

		myHalfedge* startEdge = v->originof;
		myHalfedge* currentEdge = startEdge;

		do {
			// Ajoute la position du point de face
			*faceAverage += *(currentEdge->adjacent_face->facePoint->point);

			// Ajoute la position du point d'arête
			if (currentEdge->edgePoint != nullptr) {
				*edgeAverage += *(currentEdge->edgePoint->point);
			}

			n++;

			currentEdge = currentEdge->twin->next;
		} while (currentEdge != startEdge);

		if (n > 0) {
			*faceAverage /= n;
			*edgeAverage /= n;
		}

		// Calculer la nouvelle position
		if (n > 3) {
			*(v->point) = ((*faceAverage) + (*edgeAverage) * 2 + (*originalPos) * (n - 3)) / n;
		}
		else if (n > 0) {
			// Gestion des cas où n est inférieur à 4
			*(v->point) = ((*faceAverage) + (*edgeAverage) * 2) / n;
		}

		delete originalPos;
	}

	std::vector<myFace*> newFaces;
	std::vector<myHalfedge*> newHalfedges;
	std::vector<myVertex*> newVertices;

	// Lambda function to add a vertex if it's not already in the vector
	auto addVertexIfNotExists = [&](myVertex* vertex) {
		if (std::find(newVertices.begin(), newVertices.end(), vertex) == newVertices.end()) {
			newVertices.push_back(vertex);
		}
		};

	// Form edges and faces in the new mesh
	for (myFace* oldFace : faces) {
		myHalfedge* startEdge = oldFace->adjacent_halfedge;
		myHalfedge* currentEdge = startEdge;

		do {
			myFace* newFace = new myFace();

			myHalfedge* edge1 = new myHalfedge();
			myHalfedge* edge2 = new myHalfedge();
			myHalfedge* edge3 = new myHalfedge();
			myHalfedge* edge4 = new myHalfedge();

			// Set the new vertices
			myVertex* updatedVertex = currentEdge->source;
			myVertex* edgePoint1 = currentEdge->edgePoint; // Utilise l'attribut edgePoint
			myVertex* facePoint = oldFace->facePoint; // Utilise l'attribut facePoint
			myVertex* edgePoint2 = currentEdge->prev->edgePoint; // Utilise l'attribut edgePoint

			// Connect the new half-edges
			edge1->source = updatedVertex;
			edge1->next = edge2;
			edge1->prev = edge4;
			edge1->adjacent_face = newFace;
			newFace->adjacent_halfedge = edge1;
			edge1->source->originof = edge1;
			
			edge2->source = edgePoint1;
			edge2->next = edge3;
			edge2->prev = edge1;
			edge2->adjacent_face = newFace;

			edge3->source = facePoint;
			edge3->next = edge4;
			edge3->prev = edge2;
			edge3->adjacent_face = newFace;
			edge3->source->originof = edge3;

			edge4->source = edgePoint2;
			edge4->next = edge1;
			edge4->prev = edge3;
			edge4->adjacent_face = newFace;
			edge4->source->originof = edge4;
			edgePoint2->originof = edge4;
			
			
			
			newFaces.push_back(newFace);
			newHalfedges.insert(newHalfedges.end(), { edge1, edge2, edge3, edge4 });

			addVertexIfNotExists(updatedVertex);
			addVertexIfNotExists(edgePoint1);
			addVertexIfNotExists(facePoint);
			addVertexIfNotExists(edgePoint2);

			currentEdge = currentEdge->next;
		} while (currentEdge != startEdge);
	}
	// Assignation du jumeau pour chaque nouvelle demi-arête
	for (myHalfedge* newEdge : newHalfedges) {
		for (myHalfedge* potentialTwin : newHalfedges) {
			if (newEdge->source == potentialTwin->next->source && newEdge->next->source == potentialTwin->source) {
				newEdge->twin = potentialTwin;
				break;
			}
		}
	}


	// Libération de la mémoire des anciens éléments et mise à jour des collections
	for (myFace* f : faces) {
		delete f; // Libère la mémoire allouée pour chaque ancienne face
	}
	for (myHalfedge* e : halfedges) {
		delete e; // Libère la mémoire allouée pour chaque ancienne demi-arête
	}
	//for (myVertex* v : vertices) {
	//	delete v; // Libère la mémoire allouée pour chaque ancien sommet qui n'a pas été déplacé.
	//}

	// Mise à jour des collections avec les nouveaux éléments
	faces = newFaces;
	halfedges = newHalfedges;
	vertices = newVertices;

	// Vérifie l'intégrité du nouveau maillage
	checkMeshAdvanced();
}


