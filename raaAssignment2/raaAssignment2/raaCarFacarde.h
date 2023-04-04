#pragma once

#include <windows.h>
#include <osg/switch>
#include <list>

#include "raaAnimatedFacarde.h"
#include "raaCollisionTarget.h"

// a facarde for the cars in the scene - note this also inherets from collision target to provide support for collision management

class raaCarFacarde: public raaAnimatedFacarde, public raaCollisionTarget
{
public:
	raaCarFacarde(osg::Node* pWorldRoot, osg::Node* pPart, osg::AnimationPath* ap, double dSpeed);
	virtual ~raaCarFacarde();
	void operator()(osg::Node* node, osg::NodeVisitor* nv) override;

	double dSpeed;
	int timeCount;
	bool flag;
	double preDistance;
	osg::AnimationPath* ap;

	virtual osg::Vec3f getWorldDetectionPoint(); // from raaCollisionTarget
	virtual osg::Vec3f getWorldCollisionPoint(); // from raaCollisionTarget

	void toggleStatus();
	void speedUp();
	void speedDown();
	double curSpeed;
	int status;
	bool isCollision;
	bool isCurve;
};
