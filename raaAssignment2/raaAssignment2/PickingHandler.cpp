#include "PickingHandler.h"
#include "raaCarFacarde.h"
#include "raaBoundCalculator.h"

#include <osgViewer/View>

#include <osgGA/KeySwitchMatrixManipulator>

#include <iostream>

#include <osg/LightModel>

PickingHandler::PickingHandler()
{

}
PickingHandler::~PickingHandler()
{

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
	if (!mCameraChange || !mCarFacarde)
		return false;

	osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
	if (!pView)
		return false;

	osgGA::KeySwitchMatrixManipulator* pSwitchSceneManipulator = dynamic_cast<osgGA::KeySwitchMatrixManipulator*>(pView->getCameraManipulator());
	if (!pSwitchSceneManipulator)
		return false;

	raaBoundCalculator* bounds = new raaBoundCalculator(mCarFacarde->root());
	osg::Vec3 vNewEye = (bounds->centre() + osg::Vec3(50, 0, 20)) * mCarFacarde->translation()->getWorldMatrices()[0];
	osg::Vec3 vNewCenter = (bounds->centre() + osg::Vec3(100, 0, 20)) * mCarFacarde->translation()->getWorldMatrices()[0];

	osg::Vec3 vNewUp = osg::Z_AXIS;
	pSwitchSceneManipulator->setByInverseMatrix(osg::Matrix::lookAt(vNewEye, vNewCenter, vNewUp));
	return true;
}

bool PickingHandler::handleLeftMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
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
			if (nodeName.compare("car1") == 0 || nodeName.compare("car2") == 0 || nodeName.compare("car3") == 0)
			{
				osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
				if (!pView)
					return false;

				osgGA::KeySwitchMatrixManipulator* pSwitchSceneManipulator = dynamic_cast<osgGA::KeySwitchMatrixManipulator*>(pView->getCameraManipulator());
				if (!pSwitchSceneManipulator)
					return false;

				pSwitchSceneManipulator->getInverseMatrix().getLookAt(mOldEye, mOldCenter, mOldUp);
				mCarFacarde = pCarFacarde;
				pCarFacarde->toggleStatus();
				mCameraChange = true;

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
	if (!mCarFacarde)
		return false;

	if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Up)
	{
		mCarFacarde->status = 1;
	}
	else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Down)
	{
		mCarFacarde->status = 2;
	}
	else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Z)
	{
		osgViewer::View* pView = dynamic_cast<osgViewer::View*>(&aa);
		if (!pView)
			return false;

		osgGA::KeySwitchMatrixManipulator* pSwitchSceneManipulator = dynamic_cast<osgGA::KeySwitchMatrixManipulator*>(pView->getCameraManipulator());
		if (!pSwitchSceneManipulator)
			return false;

		pSwitchSceneManipulator->setByInverseMatrix(osg::Matrix::lookAt(mOldEye, mOldCenter, mOldUp));

		pView->getSceneData()->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::LIGHTMODEL);;
		mCameraChange = false;
	}
	return true;
}
