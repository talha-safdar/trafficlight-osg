#pragma once

#include <osgGA/GUIEventHandler>

class raaCarFacarde;
class PickingHandler : public osgGA::GUIEventHandler
{
public:
	PickingHandler();
	~PickingHandler();

public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	bool handleFrame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	bool handleLeftMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	bool handleKeyUp(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
protected:
	raaCarFacarde* mCarFacarde = nullptr;
	bool mCameraChange = false;
	osg::Vec3 mOldEye;
	osg::Vec3 mOldCenter;
	osg::Vec3 mOldUp;
};

