﻿#pragma once

#include <windows.h>
#include <osg/switch>
#include <list>
#include "raaAnimatedFacarde.h"
#include "raaCollisionTarget.h"

// a facarde for the cars in the scene - note this also inherets from collision target to provide support for collision management

class raaCarFacarde : public raaAnimatedFacarde, public raaCollisionTarget
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

	bool isSameDirection(osg::Vec3 v1, osg::Vec3 v2);
	void toggleStatus();
	void speedUp();
	void speedDown();
	void setSpeed(double dSpeed);
	double getSpeed();
	void setManualDriving(bool bAuto);
	void setManualSpeed(int nStatus);
	void setManualStop(bool bStop);
	bool getManualStop();
	double curSpeed;
	int status;
	bool isCollision;
	bool isCurve;
	bool bManualDriving;
	int nManualSpeed;
	bool bManualStop;
};