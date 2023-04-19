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
	for (TrafficLightFacarde* light : m_lTrafficLights)
	{
		if (light->isTimeFinished())
			changeTrafficLight(light);
	}
	traverse(node, nv);
}

void TrafficLightControl::changeTrafficLight(TrafficLightFacarde* pTrafficLight)
{
	if (pTrafficLight->m_iTrafficLightStatus == 1)
		pTrafficLight->step = 1;
	else if (pTrafficLight->m_iTrafficLightStatus == 3)
		pTrafficLight->step = -1;
	pTrafficLight->m_iTrafficLightStatus = pTrafficLight->m_iTrafficLightStatus + pTrafficLight->step;

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

