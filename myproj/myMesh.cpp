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


// Fonction pour lire un fichier .obj et stocker ses donn�es dans la structure halfedge.
bool myMesh::readFile(std::string filename)
{
    // D�claration de variables pour stocker les lignes du fichier, les types d'�l�ments (comme "v" pour les sommets ou "f" pour les faces) et les donn�es temporaires.
    string s, t, u;
    vector<int> faceids; // Pour stocker les identifiants des sommets des faces.
    myHalfedge** hedges; // Pointeur vers un tableau de halfedges.

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
            cout << "v " << x << " " << y << " " << z << endl;
            myPoint3D * Pt = new myPoint3D(x, y, z);
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

            myFace * f = new myFace(); // allocate the new face
            f->adjacent_halfedge = hedges[0]; // connect the face with incident edge

            for (unsigned int i = 0; i < faceids.size(); i++)
            {
                int iplusone = (i + 1) % faceids.size();
                int iminusone = (i - 1 + faceids.size()) % faceids.size();

                // YOUR CODE COMES HERE!

                // connect prevs, and next
                hedges[i]->next = hedges[iplusone];
                hedges[i]->prev = hedges[iminusone];

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
                if (vertices[faceids[i]]) {
                    vertices[faceids[i]]->originof = hedges[i];
                }
                else {
                    std::cout << "Error: vertices[" << faceids[i] << "] is nullptr!" << std::endl;
                }

                hedges[i]->source = vertices[faceids[i]];


                //std::cout << "vertices (coord X) : " << i << vertices[faceids[i]]->originof->source->point->X << std::endl;
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

    // V�rifier la coh�rence du maillage.
    checkMesh();
    // Normaliser les coordonn�es des sommets.
    normalize();

    // Retourner true pour indiquer que la lecture a r�ussi.
    return true;
}



void myMesh::computeNormals()
{
    // Parcourir toutes les faces et calculer leurs normales
    for (myFace* f : faces)
    {
        f->computeNormal();
    }
    // V�rifier si 'vertices' est non vide avant d'appeler computeNormal
    if (!vertices.empty()) {
        // Parcourir tous les sommets et calculer leurs normales
        for (myVertex* v : vertices)
        {
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


void myMesh::triangulate()
{

}


bool myMesh::triangulate(myFace* f)
{
    int counter = 0;
    myHalfedge* adj = f->adjacent_halfedge;
    myHalfedge* start = adj;

    do
    {
        adj = adj->next;
        ++counter;
    } while (adj != start);

    return counter > 3;
}


