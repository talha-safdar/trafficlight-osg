#include "TrafficLightControl.h"
#include <iostream>

TrafficLightControl::TrafficLightControl(osg::Node* pPart, osg::Vec3 vTrans, float fRot, float fScale) : raaNodeCallbackFacarde(pPart, vTrans, fRot, fScale)
{
	timeCount = 0;
	activeIndex = 0;
}

TrafficLightControl::~TrafficLightControl()
{

}

int XJunction1_trafficLights[] = { 9, 2, 6, 5 }, TJunction1_trafficLights[] = { 4, 13, 12 }, TJunction2_trafficLights[] = { 8, 10, 1 }, TJunction3_trafficLights[] = { 7, 3, 11 };

void TrafficLightControl::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
	// Xjunction 1 traffic lights control part
	if (timeCount == 0)
	{
		int noOfLightsUpdate = m_lTrafficLights.size();
		for (trafficLightList::iterator it = m_lTrafficLights.begin(); it != m_lTrafficLights.end(); it++)
		{
			//std::cout << (*it)->pPart->getName()
			if ((*it)->pPart->getName() == ("trafficLight" + std::to_string(XJunction1_trafficLights[activeIndex % 4]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction1_trafficLights[activeIndex % 3]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction2_trafficLights[activeIndex % 3]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction3_trafficLights[activeIndex % 3]))) //XJunction1_trafficLights[activeIndex % 4]) 
			{
				//std::cout << (*it)->pPart->getName();
				(*it)->setGreenTrafficLight();
			}
		}
	}
	
	if (timeCount == 50)
	{
		int noOfLightsUpdate = m_lTrafficLights.size();
		for (trafficLightList::iterator it = m_lTrafficLights.begin(); it != m_lTrafficLights.end(); it++)
		{
			if ((*it)->pPart->getName() == ("trafficLight" + std::to_string(XJunction1_trafficLights[activeIndex % 4])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(XJunction1_trafficLights[(activeIndex+1) % 4]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction1_trafficLights[activeIndex % 3])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction1_trafficLights[(activeIndex + 1) % 3]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction2_trafficLights[activeIndex % 3])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction2_trafficLights[(activeIndex + 1) % 3]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction3_trafficLights[activeIndex % 3])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction3_trafficLights[(activeIndex + 1) % 3])))
				changeTrafficLight(*it);		
		}
	}
	if (timeCount == 100)
	{
		int noOfLightsUpdate = m_lTrafficLights.size();
		for (trafficLightList::iterator it = m_lTrafficLights.begin(); it != m_lTrafficLights.end(); it++)
		{
			if ((*it)->pPart->getName() == ("trafficLight" + std::to_string(XJunction1_trafficLights[activeIndex % 4])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(XJunction1_trafficLights[(activeIndex + 1) % 4]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction1_trafficLights[activeIndex % 3])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction1_trafficLights[(activeIndex + 1) % 3]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction2_trafficLights[activeIndex % 3])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction2_trafficLights[(activeIndex + 1) % 3]))
				|| (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction3_trafficLights[activeIndex % 3])) || (*it)->pPart->getName() == ("trafficLight" + std::to_string(TJunction3_trafficLights[(activeIndex + 1) % 3])))
				changeTrafficLight(*it);
		}
		timeCount = 1;
		activeIndex++;
	}

	timeCount++;
}

void TrafficLightControl::changeTrafficLight(TrafficLightFacarde* pTrafficLight)
{
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

void TrafficLightControl::addTrafficLight(TrafficLightFacarde* pTrafficLight)
{
	m_lTrafficLights.push_back(pTrafficLight);
}

trafficLightList TrafficLightControl::getTrafficLights()
{
	return m_lTrafficLights;
}

