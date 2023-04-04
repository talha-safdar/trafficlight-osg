#include <iostream>
#include "raaFacarde.h"
#include "raaSwitchActivator.h"
#include "raaAssetLibrary.h"
#include "raaInputController.h"
#include "TrafficLightFacarde.h"
#include "TrafficLightControl.h"
#include "raaCarFacarde.h"
#include "raaBoundCalculator.h"
/*
pTileIDS->setName("IDSwitch");
pAnimPointS->setName("AnimationPointSwitch");
pAnimIDS->setName("AnimationIDSwitch");
*/

bool raaInputController::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	//osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
	if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
	{
		switch(ea.getKey())
		{
		case 't':
		case 'T':
			raaFacarde::toggleNames(); // shows names of the facade objects			
			return true;
		case 'a':
		case 'A':
		{
			raaSwitchActivator a("AnimationPointSwitch", raaAssetLibrary::getAssetsRoot(), m_bShowAnimationPoints = !m_bShowAnimationPoints);
		}
			return true;
		case 'n':
		case 'N':
		{
			raaSwitchActivator n("AnimationIDSwitch", raaAssetLibrary::getAssetsRoot(), m_bShowAnimationNames = !m_bShowAnimationNames);
		}
			return true;
		case 'i':
		case 'I':
		{
			raaSwitchActivator i("IDSwitch", raaAssetLibrary::getAssetsRoot(), m_bShowAssetName = !m_bShowAssetName);
		}
			return true;
		case 'q':
		case 'Q':
		{
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "trafficLight14") {
					TrafficLightFacarde* pTrafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
					if (pTrafficLight->m_iTrafficLightStatus == 1) pTrafficLight->step = 1;
					else if (pTrafficLight->m_iTrafficLightStatus == 3) pTrafficLight->step = -1;
					pTrafficLight->m_iTrafficLightStatus = pTrafficLight->m_iTrafficLightStatus + pTrafficLight->step;
					//std::cout << step << std::endl;
					if (pTrafficLight->m_iTrafficLightStatus == 1) // set red	
					{
						pTrafficLight->setRedTrafficLight();
					}

					if (pTrafficLight->m_iTrafficLightStatus == 2) // set amber
					{
						pTrafficLight->setAmberTrafficLight();
					}

					if (pTrafficLight->m_iTrafficLightStatus == 3) // set green
					{
						pTrafficLight->setGreenTrafficLight();
					}
				}

			}
		}
			return true;
		case 'w':
		case 'W':
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "trafficLight15") {
					TrafficLightFacarde* pTrafficLight = dynamic_cast<TrafficLightFacarde*>(facarde);
					if (pTrafficLight->m_iTrafficLightStatus == 1) pTrafficLight->step = 1;
					else if (pTrafficLight->m_iTrafficLightStatus == 3) pTrafficLight->step = -1;
					pTrafficLight->m_iTrafficLightStatus = pTrafficLight->m_iTrafficLightStatus + pTrafficLight->step;
					//std::cout << step << std::endl;
					if (pTrafficLight->m_iTrafficLightStatus == 1) // set red	
					{
						pTrafficLight->setRedTrafficLight();
					}

					if (pTrafficLight->m_iTrafficLightStatus == 2) // set amber
					{
						pTrafficLight->setAmberTrafficLight();
					}

					if (pTrafficLight->m_iTrafficLightStatus == 3) // set green
					{
						pTrafficLight->setGreenTrafficLight();
					}
				}

			}
			return true;
		case '1':
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "car1") {
					raaCarFacarde* pCar = dynamic_cast<raaCarFacarde*>(facarde);
					pCar->toggleStatus();
				}
			}
			return true;
		case '2':
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "car2") {
					raaCarFacarde* pCar = dynamic_cast<raaCarFacarde*>(facarde);
					pCar->toggleStatus();
				}

			}
			return true;
		case '3':
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "car3") {
					raaCarFacarde* pCar = dynamic_cast<raaCarFacarde*>(facarde);
					pCar->toggleStatus();
				}

			}
			return true;

		}

	}
	return false;
}

raaInputController::raaInputController(osg::Node* pWorldRoot) : m_pWorldRoot(pWorldRoot), m_bShowAnimationPoints(false), m_bShowAnimationNames(false), m_bShowCollisionObjects(false), m_bShowAssetName(false)
{
}


raaInputController::~raaInputController()
{
}
