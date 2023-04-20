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

	bool handleFrame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	bool handleLeftMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	bool handleKeyUp(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	void setAllowClick(bool bAllow);

	void setFirstPerson(bool bFirst);
protected:
	void updateUI(osgViewer::View* pView);
protected:
	raaCarFacarde* m_pCarFacarde = nullptr;
	bool m_bCameraChange = false;
	osg::Vec3 m_vOldEye;
	osg::Vec3 m_vOldCenter;
	osg::Vec3 m_vOldUp;
	bool m_bAllowClick = true;
	bool m_bUpdateUI = false;
	bool m_bFirstPerson = true;
};