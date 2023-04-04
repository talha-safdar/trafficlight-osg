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


raaCarFacarde::raaCarFacarde(osg::Node* pWorldRoot, osg::Node* pPart, osg::AnimationPath* ap, double dSpeed): raaAnimatedFacarde(pPart, ap, dSpeed)
{
	raaTrafficSystem::addTarget(this); // adds the car to the traffic system (static class) which holds a reord of all the dynamic parts in the system
	this->dSpeed = dSpeed;
	this->curSpeed = dSpeed;
	this->timeCount = 0;
	this->flag = FALSE;
	this->preDistance = 0.0f;
	this->status =	FAST;
	this->isCollision = FALSE;
	this->isCurve = FALSE;
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
	//osg::Vec3 worldDetectionPoint = getWorldDetectionPoint();
	//std::cout << this->pPart->getName() << ": " << worldCollisionPoint[0] + 800 << ',' << worldCollisionPoint[1] - 530 << std::endl;
	float carX, carY;
	carX = worldCollisionPoint[0] + 800;
	carY = worldCollisionPoint[1] - 530;
	//std::cout << this->pPart->getName() << ": " << carX << ',' << carY << std::endl;
	bool red_status = FALSE, exit_flag = false;
	for (raaFacarde* facarde : sm_lFacardes) {
		// Get name
		std::string name = facarde->pPart->getName();
		if (name.find("traffic") == std::string::npos && name.find("car") == std::string::npos) continue;
		raaBoundCalculator* facardeBounds = new raaBoundCalculator(facarde->pPart);
		osg::Vec3 facardePos;
		facardePos = (facardeBounds->centre()) * facarde->translation()->getWorldMatrices()[0];

		// Calculate distance between car and facarde
		if (name.find("car") != std::string::npos) {
			facardePos[0] = facardePos[0] + 800;
			facardePos[1] = facardePos[1] - 530;
		}

		float dx = facardePos[0] - carX;
		float dy = facardePos[1] - carY;
		float distance = sqrt((dx * dx) + (dy * dy));
		// Get position of facarde
		//std::cout << name << ": " << facardePos[0] << ", " << facardePos[1] << std::endl;
		if (name == "trafficLight5") {
			if (dx > 0 && dx < 150 && dy>0 && dy < 190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight6") {
			if (dy < 0 && dy > -150 && dx>0 && dx < 170) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight9") {
			if (dx < 0 && dx > -170 && dy>0 && dy < 150) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight2") {
			if (dx < 0 && dx > -200 && dy < 0 && dy > -190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight4") {
			if (dx < 0 && dx > -200 && dy < 0 && dy > -190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight12") {
			if (dx < 0 && dx > -190 && dy > 0 && dy < 150) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight13") {
			if (dy < 0 && dy > -150 && dx > 0 && dx < 170) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight7") {
			if (dx < 0 && dx > -170 && dy > 0 && dy < 150) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight3") {
			if (dx < 0 && dx > -250 && dy < 0 && dy > -190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight11") {
			if (dx > 0 && dx < 150 && dy>0 && dy < 190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight10") {
			if (dx > 0 && dx < 150 && dy>0 && dy < 190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight8") {
			if (dy < 0 && dy > -150 && dx > 0 && dx < 170) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight1") {
			if (dx > 0 && dx < 150 && dy>0 && dy < 190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight14") {
			if (dx > 0 && dx < 100 && dy>0 && dy < 190) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (name == "trafficLight15") {
			if (dx < 0 && dx > -170 && dy > 0 && dy < 150) {
				TrafficLightFacarde* trafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
				int trafficLightStatus = trafficLight->m_iTrafficLightStatus;
				if (trafficLightStatus == 1 || trafficLightStatus == 2) this->status = SPEEDDOWN, red_status = TRUE;
				else this->status = SPEEDUP;
				break;
			}
		}
		else if (pPart->getName() != name && name.find("car") != std::string::npos) {
			//std::cout << distance << std::endl;
			//std::cout << name << ":: " << facardePos[0] << ", " << facardePos[1] << std::endl;
			//std::cout << name << ": " << dx << ", " << dy << std::endl;
			if ((distance > 0 && distance < 200 && (abs(dx) == distance || abs(dy) == distance))) {
				this->status = SPEEDDOWN;
				//	std::cout << " collision" << std::endl;
				break;
			}
			else if (distance >= 200 && distance < 400 && (abs(dx) == distance || abs(dy) == distance))
			{
				this->status = SPEEDUP; break;
			}	
	
		}
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
	raaBoundCalculator* bounds = new raaBoundCalculator(pPart);
	//return bounds->centre() * pPart->getWorldMatrices()[0];
	return (bounds->centre() + osg::Vec3(800, -530, 0)) * translation()->getWorldMatrices()[0];
	//return this->m_pRoot->getBound().center();
}

osg::Vec3f raaCarFacarde::getWorldCollisionPoint()
{
	raaBoundCalculator* bounds = new raaBoundCalculator(pPart);
	//return bounds->centre() * pPart->getWorldMatrices()[0];
	//if(pPart->getName() == "car1") return (bounds->centre() + osg::Vec3(-100, -530, 0)) *translation()->getWorldMatrices()[0];
	//else  
	return (bounds->centre() + osg::Vec3(0, 0, 0)) * translation()->getWorldMatrices()[0];

	//return this->m_pRoot->getBound().center();
}

// by controlling button and key, changes status of cars
void raaCarFacarde::toggleStatus()
{
	std::cout << "car clicked" << std::endl;
	if (this->status == SPEEDDOWN || this->status == STOP) this->status = SPEEDUP;
	else this->status = SPEEDDOWN;
}