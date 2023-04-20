#pragma once

#include "raaFacarde.h"
#include <osg/Material>
#include <osg/Geode>

class TrafficLightFacarde : public raaFacarde
{
public:TrafficLightFacarde(osg::Node* pPart, osg::Vec3 vTrans, float fRot, float fScale);
	  virtual ~TrafficLightFacarde();
	  void initLights(osg::Node* pPart);
	  void createMaterial(osg::Vec3f vColour, osg::Material* mat);
	  int m_iTrafficLightIndex;
	  int m_iTrafficLightStatus;
	  int step;
	  void setRedTrafficLight();
	  void setAmberTrafficLight();
	  void setGreenTrafficLight();

	  bool isTimeFinished();

	  virtual osg::Vec3f getWorldDetectionPoint();
	  virtual osg::Vec3f getWorldCollisionPoint();
protected:
	osg::Geode* m_pRedTrafficLight;
	osg::Geode* m_pAmberTrafficLight;
	osg::Geode* m_pGreenTrafficLight;

	osg::Material* m_pRedTrafficLightOnMaterial;
	osg::Material* m_pRedTrafficLightOffMaterial;

	osg::Material* m_pAmberTrafficLightOnMaterial;
	osg::Material* m_pAmberTrafficLightOffMaterial;

	osg::Material* m_pGreenTrafficLightOnMaterial;
	osg::Material* m_pGreenTrafficLightOffMaterial;

	osg::Timer_t m_nStartTime;
	double m_dDuration;
};