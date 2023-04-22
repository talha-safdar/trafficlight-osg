#pragma once

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>

class raaCarFacarde;
class PickingHandler : public osgGA::GUIEventHandler
{
public:
	PickingHandler();
	~PickingHandler();

	static PickingHandler* instance();
public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	// Handle frame event
	bool handleFrame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	// Handle left mouse release event
	bool handleLeftMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	// Handle keyboard key up event
	bool handleKeyUp(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	// Set whether to allow click scene
	void setAllowClick(bool bAllow);

	// Set whether to use the first person to drive
	void setFirstPerson(bool bFirst);
protected:
	void updateUI(osgViewer::View* pView);
protected:
	raaCarFacarde* m_pCarFacarde = nullptr;
	bool m_bCameraChange = false;	// Is the current driving mode
	// Save the viewpoint information before driving, and restore the state when exiting the driving mode.
	osg::Vec3 m_vOldEye;
	osg::Vec3 m_vOldCenter;
	osg::Vec3 m_vOldUp;
	bool m_bAllowClick = true;// Whether to allow click scene
	bool m_bUpdateUI = false;// Do you need to update the ui
	bool m_bFirstPerson = true;// In camera mode, whether to use the first person to drive.
};