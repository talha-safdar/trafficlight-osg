#include <windows.h>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/Material>
#include <osg/Switch>

#include "raaTrafficSystem.h"
#include "raaCarFacarde.h"
#include <iostream>
#include "raaBoundCalculator.h"
#include "raaFacarde.h"
#include "raaCollisionTarget.h"
#include "TrafficLightFacarde.h"
#include "TrafficLightControl.h"

#define SPEEDUP	1
#define SPEEDDOWN	2
#define FAST	3
#define SLOW	4
#define STOP	5


raaCarFacarde::raaCarFacarde(osg::Node* pWorldRoot, osg::Node* pPart, osg::AnimationPath* ap, double dSpeed) : raaAnimatedFacarde(pPart, ap, dSpeed)
{
	raaTrafficSystem::addTarget(this); // adds the car to the traffic system (static class) which holds a reord of all the dynamic parts in the system
	this->dSpeed = dSpeed;
	this->curSpeed = dSpeed;
	this->timeCount = 0;
	this->flag = FALSE;
	this->preDistance = 0.0f;
	this->status = FAST;
	this->isCollision = FALSE;
	this->isCurve = FALSE;
	this->bManualDriving = false;
	this->nManualSpeed = SPEEDUP;
	this->bManualStop = false;
}

raaCarFacarde::~raaCarFacarde()
{
	raaTrafficSystem::removeTarget(this); // removes the car from the traffic system (static class) which holds a reord of all the dynamic parts in the system
}

// speed down car
void raaCarFacarde::speedDown()
{
	if (this->curSpeed > 0) {
		this->curSpeed -= 10;
		setCurrentSpeed(this->curSpeed);
	}
	else {
		this->curSpeed = 0;
		setCurrentSpeed(this->curSpeed);
		//this->status = STOP;
	}
}

void raaCarFacarde::setSpeed(double dSpeed)
{
	this->dSpeed = dSpeed;
}

double raaCarFacarde::getSpeed()
{
	return this->dSpeed;
}

void raaCarFacarde::setManualDriving(bool bManual)
{
	bManualDriving = bManual;
}

void raaCarFacarde::setManualSpeed(int nStatus)
{
	nManualSpeed = nStatus;
}

void raaCarFacarde::setManualStop(bool bStop)
{
	bManualStop = bStop;
}

bool raaCarFacarde::getManualStop()
{
	return bManualStop;
}

// speed up car
void raaCarFacarde::speedUp()
{
	if (this->curSpeed < this->dSpeed) {
		this->curSpeed += 10;
		setCurrentSpeed(this->curSpeed);
	}
	else {
		this->curSpeed = this->dSpeed;
		setCurrentSpeed(this->curSpeed);
		//this->status = FAST;
	}
}

// callback function for rendering of cars
void raaCarFacarde::operator()(osg::Node* node, osg::NodeVisitor* nv)
{

	osg::Vec3 worldCollisionPoint = getWorldCollisionPoint();
	osg::Vec3 worldDetectionPoint = getWorldDetectionPoint();
	osg::Vec3 carDirection = worldDetectionPoint - worldCollisionPoint;
	carDirection.normalize();
	status = SPEEDUP;
	if (bManualDriving)
		status = nManualSpeed;

	if (!bManualStop)
	{
		for (raaFacarde* facarde : sm_lFacardes)
		{
			if (facarde == this)
				continue;

			if (TrafficLightFacarde* pTrafficLight = dynamic_cast<TrafficLightFacarde*>(facarde))
			{
				osg::Vec3 lightDetectionPoint = pTrafficLight->getWorldDetectionPoint();
				osg::Vec3 lightCollisionPoint = pTrafficLight->getWorldCollisionPoint();
				osg::Vec3 lightDirection = lightCollisionPoint - lightDetectionPoint;
				lightDirection.normalize();

				// Determine whether the traffic light and direction are consistent with the direction of the vehicle
				if (!isSameDirection(carDirection, lightDirection))
					continue;

				// Detect the distance between the vehicle and the traffic light, 
				// if it is less than 130, set the speed according to the color of the traffic light
				float distance = (worldDetectionPoint - lightCollisionPoint).length();
				if (distance < 130)
				{
					// Stop at red light
					if (pTrafficLight->m_iTrafficLightStatus == 1)
					{
						status = STOP;
					}
					// slow down at yellow light
					else if (pTrafficLight->m_iTrafficLightStatus == 2)
					{
						status = SPEEDDOWN;
					}
					// speed up at green light
					else if (pTrafficLight->m_iTrafficLightStatus == 3)
					{
						status = SPEEDUP;
						if (nManualSpeed)
							status = nManualSpeed;
					}
					break;
				}
			}

			if (raaCarFacarde* pCarFacarde = dynamic_cast<raaCarFacarde*>(facarde))
			{
				// Calculate the distance between two vehicles, if less than 50, the vehicle stops running
				osg::Vec3 vPoint = pCarFacarde->getWorldCollisionPoint();
				float distance = (worldDetectionPoint - vPoint).length();
				if (distance < 100)
				{
					status = STOP;
					break;
				}
			}
		}
	}
	else
	{
		status = STOP;
	}
	if (this->status == SPEEDUP) speedUp();
	else if (this->status == SPEEDDOWN) speedDown();
	else if (this->status == FAST) setCurrentSpeed(30);
	else if (this->status == SLOW) setCurrentSpeed(10);
	else if (this->status == STOP) setCurrentSpeed(0);

	raaAnimationPathCallback::operator()(node, nv);
}

osg::Vec3f raaCarFacarde::getWorldDetectionPoint()
{
	raaBoundCalculator bounds(pPart);
	return (bounds.centre() + osg::Vec3(100, 0, 0)) * root()->getWorldMatrices()[0];
}

osg::Vec3f raaCarFacarde::getWorldCollisionPoint()
{
	raaBoundCalculator bounds(pPart);
	return (bounds.centre() - osg::Vec3(100, 0, 0)) * root()->getWorldMatrices()[0];

}

bool raaCarFacarde::isSameDirection(osg::Vec3 v1, osg::Vec3 v2)
{
	float epsinon = 0.0001f;
	return abs(v1.x() - v2.x()) < epsinon && abs(v1.y() - v2.y()) < epsinon && abs(v1.z() - v2.z()) < epsinon;
}

// by controlling button and key, changes status of cars
void raaCarFacarde::toggleStatus()
{
	std::cout << "car clicked" << std::endl;
	if (this->status == SPEEDDOWN || this->status == STOP) this->status = SPEEDUP;
	else this->status = SPEEDDOWN;
}