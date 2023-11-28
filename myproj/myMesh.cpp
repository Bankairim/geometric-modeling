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
			std::cout << "Vertex " << v->index << ": Demi-ar�te actuelle est NULL." << std::endl;
			break;
		}

		if (!current->twin) {
			std::cout << "Vertex " << v->index << ": Demi-ar�te n'a pas de jumeau." << std::endl;
		}
		else if (current->twin->twin != current) {
			std::cout << "Vertex " << v->index << ": Inconsistance de jumeau detectee." << std::endl;
		}

		if (!current->adjacent_face) {
			std::cout << "Vertex " << v->index << ": Pas de face adjacente � la demi-ar�te actuelle." << std::endl;
		}

		current = current->twin ? current->twin->next : nullptr;
		iteration++;
		if (iteration >= maxIterations) {
			std::cout << "Vertex " << v->index << ": Attention, d�passement du nombre maximum d'it�rations dans checkVertice()." << std::endl;
			break;
		}
	} while (current && current != start);
}

// Fonction pour lire un fichier .obj et stocker ses donn�es dans la structure halfedge.
bool myMesh::readFile(std::string filename)
{
	// D�claration de variables pour stocker les lignes du fichier, les types d'�l�ments (comme "v" pour les sommets ou "f" pour les faces) et les donn�es temporaires.
	string s, t, u;
	vector<int> faceids; // Pour stocker les identifiants des sommets des faces.
	myHalfedge** hedges; // Pointeur vers le tableau de halfedges.

	// Ouvrir le fichier .obj pour la lecture.
	ifstream fin(filename);
	// Si le fichier ne peut pas �tre ouvert, afficher un message d'erreur et retourner false.
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	// Stocker le nom du fichier pour une utilisation ult�rieure.
	name = filename;

	// Cr�er une map pour stocker les halfedges et leurs jumeaux (twins).
	map<pair<int, int>, myHalfedge*> twin_map; 
	map<pair<int, int>, myHalfedge*>::iterator it;
	int verticeNumber = 0;
	// Lire le fichier ligne par ligne.
	while (getline(fin, s))
	{
		// Utiliser un stringstream pour extraire les donn�es de la ligne.
		stringstream myline(s);
		myline >> t; // Extraire le type d'�l�ment (comme "v" ou "f").

		// Si l'�l�ment est un groupe ("g"), ne rien faire.
		if (t == "g") {}
		// Si l'�l�ment est un sommet ("v").
		else if (t == "v")
		{
			float x, y, z;
			// Extraire les coordonn�es x, y et z du sommet.
			myline >> x >> y >> z;
			// Afficher les coordonn�es pour le d�bogage.
			cout << "v " << x << " " << y << " " << z << " index : " << verticeNumber << endl;
			myPoint3D* Pt = new myPoint3D(x, y, z);
			myVertex* v = new myVertex();
			v->point = Pt;

			// 23/11:2023 : Set the index of the vertex
			v->index = vertices.size();
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

				//// search for the twins using twin_map
				//pair<int, int> edgeKey = make_pair(faceids[iplusone], faceids[i]);
				//it = twin_map.find(edgeKey);
				//if (it != twin_map.end()) {
				//	hedges[i]->twin = it->second;
				//	it->second->twin = hedges[i];
				//}
				//else {
				//	twin_map[make_pair(faceids[i], faceids[iplusone])] = hedges[i];
				//}

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
	cout << " Readfile termin�. Vertices size : " << vertices.size() << endl;
	clearTwinRelationships();
	establishTwinRelationships();
	// V�rifier la coh�rence du maillage.
	checkMeshAdvanced();
	// Normaliser les coordonn�es des sommets.
	checkHalfEdgeReferences();
	normalize();

	// Retourner true pour indiquer que la lecture a r�ussi.
	return true;
}

void myMesh::computeNormals()
{
	// Parcourir toutes les  faces et calculer leurs normales
	for (myFace* f : faces)
	{
		f->computeNormal();
	}
	// V�rifier si 'vertices' est non vide avant d'appeler computeNormal
	if (!vertices.empty()) {
		// Parcourir tous les sommets et calculer leurs normales
		cout << "Vertices size : " << vertices.size() << endl;
		for (int i=0;i < vertices.size();i++)
		{
			//A ce stade la, v est cens� avoir un point et un originof du readfile et va obtenir une normal bas�e sur son originof.
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

void myMesh::subdivisionCatmullClark()
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

void myMesh::establishTwinRelationships() {
	std::map<std::pair<int, int>, myHalfedge*> edgeMap;

	for (std::size_t i = 0;i<halfedges.size();i++) {
		if (halfedges[i]) {
			std::pair<int, int> edgeKey(halfedges[i]->source->index, halfedges[i]->next->source->index);
			std::pair<int, int> twinKey(halfedges[i]->next->source->index, halfedges[i]->source->index);

			// Check if the twin is already in the map
			if (edgeMap.count(twinKey)) {
				//cout << "Twin d�j� existant" << endl;
				// Establish the twin relationship
				halfedges[i]->twin = edgeMap[twinKey];
				edgeMap[twinKey]->twin = halfedges[i];
			}
			else {
				// Add the half-edge to the map for future twin finding
				edgeMap[edgeKey] = halfedges[i];
			}
		}
	}

	// search for the twins using twin_map
}

void myMesh::triangulate() {
	
	std::vector<myFace*> toRemove;
	int faceNumber = faces.size();
	cout << "number of faces (face.size()) : " << faces.size() << endl;
	for (int i=0;i<faceNumber;i++)
	{
		//cout << "Starting triangulate " << i << "." << endl;
		myFace* f  = faces[i];
		if (triangulate(f)) {
			toRemove.push_back(f);
		}
		//cout << "End of triangulate " << i << "." << endl;
	}
	for (int i = 0; i < toRemove.size(); i++)
	{
		faces.erase(remove(faces.begin(), faces.end(), toRemove[i]),faces.end());
	}
	std::cout << " Triangulated faces : " << faces.size() << endl;
	
	clearTwinRelationships();
	establishTwinRelationships();
	//checkMeshAdvanced();
	//checkHalfEdgeReferences();
}

void myMesh::testTriangulate()
{
	//triangulate();
	//checkMeshAdvanced();
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

bool myMesh::triangulate(myFace* f) {
	//std::cout << "Starting triangulation function" << std::endl;
	std::vector<myVertex*> faceVertices;
	std::map<std::pair<int, int>, myHalfedge*> edgeMap; // To track edges and their twins

	myHalfedge* currentEdge = f->adjacent_halfedge;
	do {
		faceVertices.push_back(currentEdge->source);
		std::pair<int, int> edgeKey(currentEdge->source->index, currentEdge->next->source->index);
		edgeMap[edgeKey] = currentEdge;
		currentEdge = currentEdge->next;
	} while (currentEdge != f->adjacent_halfedge);
	//std::cout << "Edges correctly mapped " << std::endl;
	if (faceVertices.size() <= 3) return false;

	myVertex* centerVertex = createCenterVertex(faceVertices);
	std::vector<myHalfedge*> newHalfedges;
	createTriangles(centerVertex, faceVertices, newHalfedges);
	
	

	//std::cout << "checkMesh after triangulation of face " << std::endl;
	//checkMeshAdvanced();
	//std::cout << "Leaving the triangulate function "<< endl;
	return true;
}

void myMesh::createTriangles(myVertex* centerVertex, const std::vector<myVertex*>& faceVertices, std::vector<myHalfedge*>& newHalfedges) {
	for (size_t i = 0; i < faceVertices.size(); ++i) {
		myVertex* A = centerVertex;
		myVertex* B = faceVertices[i];
		myVertex* C = faceVertices[(i + 1) % faceVertices.size()];

		myFace* newTriangle = new myFace();
		myHalfedge* AB = new myHalfedge();
		myHalfedge* BC = new myHalfedge();
		myHalfedge* CA = new myHalfedge();

		// Setup half-edges and assign to newTriangle
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

		// Add the new triangle to the face list
		faces.push_back(newTriangle);

		// Add the new half-edges to the half-edge list
		halfedges.push_back(AB);
		halfedges.push_back(BC);
		halfedges.push_back(CA);

		// Set originof for centerVertex
		if (i == 0) {  // Set it only once
			centerVertex->originof = AB;
		}

		// Store BC for twin assignment in a later step
		newHalfedges.push_back(BC);
	}
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
	if (!e || !e->twin) return false; // V�rifier la pr�sence de l'halfedge et de son jumeau
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
	// Effacer v1 si pr�sent
	auto itV1 = std::remove(vertices.begin(), vertices.end(), v1);
	if (itV1 != vertices.end()) {
		vertices.erase(itV1, vertices.end());
	}

	// Effacer v2 si pr�sent
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

void myMesh::allCollapse(double d) {
	bool collapsed = true; // Flag pour v�rifier si au moins un effondrement a eu lieu
	while (collapsed) {
		collapsed = false; // R�initialiser le flag pour la prochaine it�ration
		std::vector<myHalfedge*> edgesToCollapse; // Stocker les halfedges � effondrer

		// Identifier les halfedges �ligibles pour l'effondrement
		for (myHalfedge* he : halfedges) {
			if (distance(he->source->point, he->twin->source->point) <= d) {
				edgesToCollapse.push_back(he);
			}
		}

		// Effondrer les halfedges identifi�s
		for (myHalfedge* he : edgesToCollapse) {
			if (collapse(he)) {
				collapsed = true; // Si collapse a r�ussi, mettre le flag � vrai
				// Note: collapse doit mettre � jour les structures halfedges et vertices
			}
		}

		// Optionnel: recalculer les distances ou la mesure d'erreur apr�s chaque effondrement
		// ...

		// S'arr�ter si aucun effondrement n'a eu lieu durant cette it�ration
		if (!collapsed) {
			break;
		}
	}

	// Recalculer les normales apr�s la simplification
	computeNormals();
}
