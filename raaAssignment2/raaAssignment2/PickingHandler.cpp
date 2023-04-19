﻿#include "PickingHandler.h"
#include "raaCarFacarde.h"
#include "raaBoundCalculator.h"

#include <osgViewer/View>

#include <osgGA/KeySwitchMatrixManipulator>

#include <iostream>

#include <osg/LightModel>

PickingHandler::PickingHandler()
{
	m_bAllowClick = true;
	m_bUpdateUI = true;
}
PickingHandler::~PickingHandler()
{

}

PickingHandler* PickingHandler::instance()
{
	static osg::ref_ptr<PickingHandler> s_ptrPickingHandler;
	if (!s_ptrPickingHandler)
		s_ptrPickingHandler = new PickingHandler();
	return s_ptrPickingHandler;
}

bool PickingHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	switch (ea.getEventType())
	{
	case osgGA::GUIEventAdapter::FRAME:
		return handleFrame(ea, aa);

	case osgGA::GUIEventAdapter::RELEASE:
	{
		if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			return handleLeftMouseRelease(ea, aa);

		return false;
	}
	case osgGA::GUIEventAdapter::KEYUP:
	{
		return handleKeyUp(ea, aa);
	}
	default:
		break;
	}
	return false;
}

bool PickingHandler::handleFrame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	if (m_bUpdateUI)
	{
		updateUI(dynamic_cast<osgViewer::View*>(&aa));
		m_bUpdateUI = false;
	}

	if (!m_bCameraChange || !m_pCarFacarde)
		return false;

	osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
	if (!pView)
		return false;

	osgGA::KeySwitchMatrixManipulator* pSwitchSceneManipulator = dynamic_cast<osgGA::KeySwitchMatrixManipulator*>(pView->getCameraManipulator());
	if (!pSwitchSceneManipulator)
		return false;

	raaBoundCalculator* bounds = new raaBoundCalculator(m_pCarFacarde->root());
	osg::Vec3 vNewEye = (bounds->centre() + osg::Vec3(50, 0, 30)) * m_pCarFacarde->translation()->getWorldMatrices()[0];
	osg::Vec3 vNewCenter = (bounds->centre() + osg::Vec3(100, 0, 30)) * m_pCarFacarde->translation()->getWorldMatrices()[0];

	osg::Vec3 vNewUp = osg::Z_AXIS;
	pSwitchSceneManipulator->setByInverseMatrix(osg::Matrix::lookAt(vNewEye, vNewCenter, vNewUp));
	return true;
}

bool PickingHandler::handleLeftMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	if (!m_bAllowClick)
		return false;

	// Get the mouse click position
	float x = ea.getX();
	float y = ea.getY();

	// Create a new intersector for the camera
	osgUtil::LineSegmentIntersector* intersector = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);
	osgUtil::IntersectionVisitor iv(intersector);

	// Traverse the scene graph and compute the intersections
	aa.asView()->getCamera()->accept(iv);
	if (intersector->containsIntersections())
	{
		// Get the first intersection from the list
		osgUtil::LineSegmentIntersector::Intersection intersection = *(intersector->getIntersections().begin());
		osg::NodePath nodePath = intersection.nodePath;

		// Print the node name
		for (int i = 0; i < nodePath.size(); ++i)
		{
			raaCarFacarde* pCarFacarde = dynamic_cast<raaCarFacarde*>(nodePath[i]->getUpdateCallback());
			if (!pCarFacarde || !pCarFacarde->root())
				continue;

			std::string nodeName = nodePath[i + 1]->getName();
			std::cout << "Clicked on node: " << nodeName << std::endl;
			if (nodeName.find("car") != std::string::npos)
			{
				osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
				if (!pView)
					return false;

				osgGA::KeySwitchMatrixManipulator* pSwitchSceneManipulator = dynamic_cast<osgGA::KeySwitchMatrixManipulator*>(pView->getCameraManipulator());
				if (!pSwitchSceneManipulator)
					return false;

				pSwitchSceneManipulator->getInverseMatrix().getLookAt(m_vOldEye, m_vOldCenter, m_vOldUp);
				m_pCarFacarde = pCarFacarde;
				pCarFacarde->toggleStatus();
				m_bCameraChange = true;

				osg::ref_ptr<osg::LightModel> ptrLightModel = new osg::LightModel();
				ptrLightModel->setAmbientIntensity(osg::Vec4(1.0, 1.0, 1.0, 1.0));
				pView->getSceneData()->getOrCreateStateSet()->setAttributeAndModes(ptrLightModel);
				return true;
			}
		}
	}
	return false;
}

bool PickingHandler::handleKeyUp(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	if (ea.getKey() == osgGA::GUIEventAdapter::KEY_X)
	{
		m_bAllowClick = !m_bAllowClick;
		m_bUpdateUI = true;
	}
	if (!m_pCarFacarde)
		return false;
	if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Up)
	{
		m_pCarFacarde->status = 1;
	}
	else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Down)
	{
		m_pCarFacarde->status = 2;
	}
	else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Z)
	{
		osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
		if (!pView)
			return false;

		osgGA::KeySwitchMatrixManipulator* pSwitchSceneManipulator = dynamic_cast<osgGA::KeySwitchMatrixManipulator*>(pView->getCameraManipulator());
		if (!pSwitchSceneManipulator)
			return false;

		pSwitchSceneManipulator->setByInverseMatrix(osg::Matrix::lookAt(m_vOldEye, m_vOldCenter, m_vOldUp));

		pView->getSceneData()->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::LIGHTMODEL);;
		m_bCameraChange = false;
	}
	return true;
}

void PickingHandler::setAllowClick(bool bAllow)
{
	m_bAllowClick = bAllow;
	m_bUpdateUI = true;
}

void PickingHandler::updateUI(osgViewer::View* pView)
{
	if (!pView)
		return;

	osg::Group* pSceneGroup = dynamic_cast<osg::Group*>(pView->getSceneData());
	if (pSceneGroup)
	{
		for (unsigned int i = 0; i < pSceneGroup->getNumChildren(); ++i)
		{
			osg::Camera* pCamera = dynamic_cast<osg::Camera*>(pSceneGroup->getChild(i));
			if (pCamera)
			{
				pCamera->setNodeMask(m_bAllowClick ? 0xFFFFFFFF : 0);
			}
			else
			{
				osg::Group* pRoot = dynamic_cast<osg::Group*>(pSceneGroup->getChild(i));
				if (pRoot)
				{
					for (unsigned int i = 0; i < pRoot->getNumChildren(); ++i)
					{
						osg::Camera* pTextCamera = dynamic_cast<osg::Camera*>(pRoot->getChild(i));
						if (pTextCamera)
						{
							pTextCamera->setNodeMask(m_bAllowClick ? 0xFFFFFFFF : 0);
						}
					}
				}
			}
		}
	}
}
