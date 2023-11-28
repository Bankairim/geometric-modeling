#pragma once

class myHalfedge;
class myVector3D;

class myFace
{
public:
	myHalfedge *adjacent_halfedge;

	myVector3D *normal;

	int index; //use this variable as you wish.
	myFace& myFace::operator=(const myFace& other);
	void computeNormal();
	bool myFace::isDegenerate();
	myFace(void);
	~myFace(void);
};