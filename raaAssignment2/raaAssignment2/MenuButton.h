#pragma once

#include <osgWidget/Label>

class ColorLabel : public osgWidget::Label
{
public:
	ColorLabel(const std::string& label);

	virtual bool mousePush(double, double, const osgWidget::WindowManager*);

	virtual bool mouseEnter(double, double, const osgWidget::WindowManager*);

	virtual bool mouseLeave(double, double, const osgWidget::WindowManager*);

	virtual bool mouseRelease(double, double, const osgWidget::WindowManager*);
};

class MenuButton : public ColorLabel
{
public:
	MenuButton(const std::string& label, int nId);

	virtual bool mousePush(double, double, const osgWidget::WindowManager*);

	virtual bool mouseLeave(double, double, const osgWidget::WindowManager*);
protected:
	int m_nMenuId;		// The command ID associated with the menu
};