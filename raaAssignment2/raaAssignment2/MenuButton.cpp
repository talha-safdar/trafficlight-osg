#include "MenuButton.h"
#include "ButtonCommandSet.h"

const osg::Vec4 buttonClickedColour = osg::Vec4(0.0f, 0.4f, 0.4f, 0.8f);

ColorLabel::ColorLabel(const std::string& label) :
	osgWidget::Label("", "")
{
	setFontSize(14);
	setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
	setColor(0.3f, 0.3f, 0.3f, 1.0f);
	addHeight(30.0f);
	setCanFill(true);
	setLabel(label);
	setEventMask(osgWidget::EVENT_MOUSE_PUSH | osgWidget::EVENT_MASK_MOUSE_MOVE);
}

bool ColorLabel::mousePush(double, double, const osgWidget::WindowManager*)
{
	return true;
}

bool ColorLabel::mouseEnter(double, double, const osgWidget::WindowManager*)
{
	if (getColor() != buttonClickedColour)
	{
		setColor(0.6f, 0.6f, 0.6f, 1.0f);
	}
	return true;
}

bool ColorLabel::mouseLeave(double, double, const osgWidget::WindowManager*)
{
	setColor(0.3f, 0.3f, 0.3f, 1.0f);
	return true;
}

bool ColorLabel::mouseRelease(double, double, const osgWidget::WindowManager*)
{
	setColor(0.3f, 0.3f, 0.3f, 1.0f);
	return true;
}


/// <summary>
/// ////////////////////////////////////////////////////////////////////////////
/// </summary>
MenuButton::MenuButton(const std::string& label, int nId) :
	ColorLabel(label)
{
	m_nMenuId = nId;
}

bool MenuButton::mousePush(double, double, const osgWidget::WindowManager*)
{
	if (getColor() == osg::Vec4(buttonClickedColour))
	{
		setColor(0.8f, 0.8f, 0.8f, 0.8f); // set color back to original color
	}
	// If the button is not orange, set it to orange
	else
	{
		setColor(buttonClickedColour); // set color to orange
	}

	CommandBase* pCommand = ButtonCommandSet::instance()->getCommandById(m_nMenuId);
	if (pCommand != nullptr)
	{
		(*pCommand)();
	}
	return true;
}

bool MenuButton::mouseLeave(double, double, const osgWidget::WindowManager*)
{
	if (getColor() != buttonClickedColour)
	{
		setColor(0.3f, 0.3f, 0.3f, 1.0f);
	}
	return true;
}
