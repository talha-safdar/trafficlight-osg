#include <windows.h>
#include <iostream>
#include <math.h>
#include <osgWidget/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osg/AnimationPath>
#include <osg/Matrix>
#include <osg/Material>
#include <osg/ShapeDrawable>
#include <osgText/Text>
#include <chrono>
#include <thread>
#include <windows.h>
#include <osg/ImageStream>
#include <osgDB/Registry>
#include <osg/ComputeBoundsVisitor>
#include <osgGA/GUIEventHandler>
#include "raaInputController.h"
#include "raaAssetLibrary.h"
#include "raaFacarde.h"
#include "raaSwitchActivator.h"
#include "raaRoadTileFacarde.h"
#include "raaAnimationPointFinder.h"
#include "raaAnimatedFacarde.h"
#include "raaCarFacarde.h"
#include "raaTrafficSystem.h"
#include "TrafficLightControl.h"
#include "TrafficLightFacarde.h"
#include "raaBoundCalculator.h"

// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetmenu.cpp 66 2008-07-14 21:54:09Z cubicool $

#include <osgDB/ReadFile>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Label>

// For now this is just an example, but osgWidget::Menu will later be it's own Window.
// I just wanted to get this out there so that people could see it was possible.

const unsigned int MASK_2D = 0xF0000000;
const unsigned int MASK_3D = 0x0F000000;
const osg::Vec4 buttonClickedColour = osg::Vec4(0.0f, 0.4f, 0.4f, 0.8f);
osgViewer::Viewer viewer;

struct ColorLabel : public osgWidget::Label {
	ColorLabel(const char* label) :
		osgWidget::Label("", "") {
		// setFont("../../Data/fonts/Vera.ttf");
		setFontSize(14);
		setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
		setColor(0.3f, 0.3f, 0.3f, 1.0f);
		addHeight(30.0f);
		setCanFill(true);
		setLabel(label);
		setEventMask(osgWidget::EVENT_MOUSE_PUSH | osgWidget::EVENT_MASK_MOUSE_MOVE);
	}

	bool mousePush(double, double, const osgWidget::WindowManager*) {
		std::cout << "O mouse push" << std::endl;
		return true;
	}

	bool mouseEnter(double, double, const osgWidget::WindowManager*) {
		if (getColor() != buttonClickedColour) {
			setColor(0.6f, 0.6f, 0.6f, 1.0f);
		}
		std::cout << "O mouse hover" << std::endl;
		return true;
	}

	bool mouseLeave(double, double, const osgWidget::WindowManager*) {
		setColor(0.3f, 0.3f, 0.3f, 1.0f);
		std::cout << "O mouse leave" << std::endl;
		return true;
	}

	bool mouseRelease(double, double, const osgWidget::WindowManager*) {
		setColor(0.3f, 0.3f, 0.3f, 1.0f);
		std::cout << "O mouse release" << std::endl;
		return true;
	}
};

osg::ref_ptr<osg::MatrixTransform> createCameraView(osgViewer::Viewer& viewer, osg::Vec3d position, osg::Vec3d rotation)
{
	// Create a new camera
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;

	// Set the new camera's GraphicsContext and Viewport to match the current viewer
	camera->setGraphicsContext(viewer.getCamera()->getGraphicsContext());
	camera->setViewport(viewer.getCamera()->getViewport());

	// Set the new camera's position and orientation
	osg::Matrixd cameraMatrix;
	cameraMatrix.makeTranslate(position);
	cameraMatrix *= osg::Matrixd::rotate(rotation.x(), 1.0, 0.0, 0.0);
	cameraMatrix *= osg::Matrixd::rotate(rotation.y(), 0.0, 1.0, 0.0);
	cameraMatrix *= osg::Matrixd::rotate(rotation.z(), 0.0, 0.0, 1.0);
	camera->setViewMatrix(cameraMatrix);

	// Create a new MatrixTransform node to hold the camera
	osg::ref_ptr<osg::MatrixTransform> cameraNode = new osg::MatrixTransform;
	cameraNode->addChild(camera);

	// Add a TrackballManipulator to enable mouse rotation
	osgGA::TrackballManipulator* trackball = new osgGA::TrackballManipulator;
	trackball->setAllowThrow(false);
	viewer.setCameraManipulator(trackball);

	// Return the new MatrixTransform node
	return cameraNode;
}

class CameraEventHandler : public osgGA::GUIEventHandler
{
public:
	CameraEventHandler(osg::Camera* camera) : m_camera(camera) {}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		switch (ea.getEventType())
		{
		case osgGA::GUIEventAdapter::MOVE:
		{
			osg::Vec3d position, direction, up;
			osg::Matrixd matrix = m_camera->getViewMatrix();
			matrix.getLookAt(position, direction, up);

			osg::Vec3d rotation;
			rotation.x() = osg::RadiansToDegrees(atan2(direction.z(), direction.y()));
			rotation.y() = osg::RadiansToDegrees(atan2(direction.x(), direction.z()));
			rotation.z() = osg::RadiansToDegrees(atan2(direction.y(), direction.x()));

			std::cout << "Camera position: " << position.x() << ", " << position.y() << ", " << position.z() << std::endl;
			std::cout << "Camera rotation: " << rotation.x() << ", " << rotation.y() << ", " << rotation.z() << std::endl;

			break;
		}

		default:
			break;
		}

		return false;
	}

private:
	osg::Camera* m_camera;
};
  
#define M_PI       3.14159265358979323846   // pi
#define M_1_PI     0.318309886183790671538  // 1/pi

typedef std::vector<raaAnimationPointFinder>raaAnimationPointFinders;
osg::Group* g_pRoot = 0; // root of the sg
float g_fTileSize = 472.441f; // width/depth of the standard road tiles
std::string g_sDataPath = "../../Data/";

const float FACE_FORWARD = 0.0f;
const float FACE_LEFT = -90.0f;
const float FACE_UP = 180.0f;
const float FACE_RIGHT = 90.0f;

int lights = 0;

class CarControlButton : public ColorLabel {

private:
	int carID;
public:
	CarControlButton(const char* label) : ColorLabel(label) {

		if (!strcmp(label, "car1")) carID = 1;
		else if (!strcmp(label, "car2")) carID = 2;
		else if (!strcmp(label, "car3")) carID = 3;


		setColor(0.8f, 0.8f, 0.8f, 0.8f);
	}

	bool mousePush(double, double, const osgWidget::WindowManager*) {
		// If the button is already orange, set it back to the original color
		if (getColor() == osg::Vec4(buttonClickedColour)) {
			std::cout << "original colour" << std::endl;
			setColor(0.8f, 0.8f, 0.8f, 0.8f); // set color back to original color
		}
		// If the button is not orange, set it to orange
		else {
			std::cout << "orange colour" << std::endl;
			setColor(buttonClickedColour); // set color to orange
		}

		switch (carID) {
		case 1:
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "car1") {
					raaCarFacarde* pCar = dynamic_cast<raaCarFacarde*>(facarde);
					pCar->toggleStatus();

					// to be fixed
					//osg::ref_ptr<osg::MatrixTransform> cameraTransform = createCameraView(viewer, osg::Vec3d(192.062, 892.065, 195.52), osg::Vec3d(0.0, 0.0, 0.0));
					//osg::ref_ptr<osg::MatrixTransform> carTransform = new osg::MatrixTransform();
					//carTransform->addChild(pCar->root());
					//carTransform->addChild(cameraTransform);
					//g_pRoot->addChild(carTransform);
				}
				// setColor(1.0f, 0.5f, 0.5f, 0.8f); // orange
				// std::cout << "mouse leave " << getLabel() << std::endl; // print button label
			}
			return true;
		case 2:
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "car2") {
					raaCarFacarde* pCar = dynamic_cast<raaCarFacarde*>(facarde);
					pCar->toggleStatus();
				}

			}
			return true;
		case 3:
			for (raaFacarde* facarde : raaFacarde::facardes()) {
				std::string name = facarde->pPart->getName();
				if (name == "car3") {
					raaCarFacarde* pCar = dynamic_cast<raaCarFacarde*>(facarde);
					pCar->toggleStatus();
				}

			}
			return true;
		default:
			return false;
		}
	}

	//bool mouseRelease(double, double, const osgWidget::WindowManager*) {
	//	setColor(1.0f, 0.5f, 0.5f, 0.8f); // orange
	//	std::cout << "mouse release 2" << std::endl;
	//	return true;
	//}

	bool mouseLeave(double, double, const osgWidget::WindowManager*) {
		if (getColor() != buttonClickedColour) {
			setColor(0.8f, 0.8f, 0.8f, 0.8f);
		}
		std::cout << "mouse leave 2" << std::endl;
		osg::Camera* camera = viewer.getCamera();
		// viewer.addEventHandler(new CameraEventHandler(camera));
		return true;
	}
};

enum raaRoadTileType
{
	Normal,
	LitTJunction,
	LitXJunction,
};

void addRoadTile(std::string sAssetName, std::string sPartName, int xUnit, int yUnit, float fRot, osg::Group* pParent)
{
	raaFacarde* pFacarde = new raaRoadTileFacarde(raaAssetLibrary::getNamedAsset(sAssetName, sPartName), osg::Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot);
	pParent->addChild(pFacarde->root());
}

void addLightsTile(std::string sAssetName, std::string sPartName, int xUnit, int yUnit, float fRot, osg::Group* pParent, osg::Group* trafficLights)
{
	// Create traffic control
	std::string name;
	TrafficLightControl* pFacarde = new TrafficLightControl(raaAssetLibrary::getNamedAsset(sAssetName, sPartName), osg::Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot, 1.0f);
	pParent->addChild(pFacarde->root());

	// Dynamically give traffic light names based on the amount in the group
	int length = trafficLights->getNumChildren();

	float angleInRadians = fRot * (M_PI / 180);
	float pavementRatio = 0.8f;

	if (sin(fRot) > cos(fRot) && sAssetName != "roadCurve") {
		angleInRadians += M_PI;
	}


	if (sAssetName == "roadCurve") {
		angleInRadians += M_PI / 4;
		pavementRatio = 0.3f;
	}
	if (sPartName == "tile20" || sPartName == "tile17" || sPartName == "tile13" || sPartName == "tile5" || sPartName == "tile25") {
		// Create left light
		lights++;
		name = "trafficLight" + std::to_string(lights);
		TrafficLightFacarde* leftLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + sin(angleInRadians) * (g_fTileSize / 2) * pavementRatio - 285.0f, g_fTileSize * yUnit + (cos(angleInRadians) * (g_fTileSize / 2) - g_fTileSize) * pavementRatio, 0.0f), fRot + 90.0f, 0.08f);
		// Add lights to controller
		pFacarde->addTrafficLight(leftLight);
		// Add to trafficLights OSG::Group
		trafficLights->addChild(leftLight->root());

		if (sPartName == "tile5") {
			// Create right light
			lights++;
			name = "trafficLight" + std::to_string(lights);
			TrafficLightFacarde* rightLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + sin(angleInRadians + M_1_PI) * (g_fTileSize / 2) * pavementRatio + 225.0f, g_fTileSize * yUnit + cos(angleInRadians + M_1_PI) * (g_fTileSize / 2) * pavementRatio, 0.0f), fRot - 90.0f, 0.08f);
			pFacarde->addTrafficLight(rightLight);
			trafficLights->addChild(rightLight->root());
			
		}
	}

	if (sPartName == "tile1" || sPartName == "tile10") {
			// Create right light
			lights++;
			name = "trafficLight" + std::to_string(lights);
			TrafficLightFacarde* rightLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + sin(angleInRadians + M_1_PI) * (g_fTileSize / 2) * pavementRatio + 225.0f, g_fTileSize * yUnit + cos(angleInRadians + M_1_PI) * (g_fTileSize / 2) * pavementRatio, 0.0f), fRot - 90.0f, 0.08f);
			pFacarde->addTrafficLight(rightLight);
			trafficLights->addChild(rightLight->root());
		
	}

	if (sPartName == "tile12" || sPartName == "tile15" || sPartName == "tile7" || sPartName == "tile18") {
		// Create left light
		lights++;
		name = "trafficLight" + std::to_string(lights);
		TrafficLightFacarde* leftLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + (sin(angleInRadians) * (g_fTileSize / 2) + g_fTileSize  )* pavementRatio, g_fTileSize * yUnit + (cos(angleInRadians) * (g_fTileSize / 2)) * pavementRatio - 285.0f , 0.0f), fRot - 90.0f, 0.08f);
		// Add lights to controller
		pFacarde->addTrafficLight(leftLight);
		
		// Add to trafficLights OSG::Group
		trafficLights->addChild(leftLight->root());


		// Create right light
		lights++;
		name = "trafficLight" + std::to_string(lights);
		TrafficLightFacarde* rightLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + (sin(angleInRadians + M_1_PI) * (g_fTileSize / 2) ) * pavementRatio , g_fTileSize * yUnit + (cos(angleInRadians + M_1_PI) * (g_fTileSize / 2) )* pavementRatio + 225.0f , 0.0f), 0.0f, 0.08f);
		pFacarde->addTrafficLight(rightLight);
		trafficLights->addChild(rightLight->root());
	}

	if (sPartName == "tile3" || sPartName == "tile8" ) {
		
		if (sPartName == "tile8") {
			// Create left light
			lights++;
			name = "trafficLight" + std::to_string(lights);
			TrafficLightFacarde* leftLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + (sin(angleInRadians) * (g_fTileSize / 2) + g_fTileSize) * pavementRatio, g_fTileSize * yUnit + (cos(angleInRadians) * (g_fTileSize / 2)) * pavementRatio - 285.0f, 0.0f), fRot - 90.0f, 0.08f);
			// Add lights to controller
			pFacarde->addTrafficLight(leftLight);

			// Add to trafficLights OSG::Group
			trafficLights->addChild(leftLight->root());

		}

		if (sPartName == "tile3") {
			// Create right light
			lights++;
			name = "trafficLight" + std::to_string(lights);
			TrafficLightFacarde* rightLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + (sin(angleInRadians + M_1_PI) * (g_fTileSize / 2)) * pavementRatio, g_fTileSize * yUnit + (cos(angleInRadians + M_1_PI) * (g_fTileSize / 2)) * pavementRatio + 225.0f, 0.0f), 0.0f, 0.08f);
			pFacarde->addTrafficLight(rightLight);
			trafficLights->addChild(rightLight->root());
		}
	}

	if (sPartName == "tile21") {
		
			lights++;
			name = "trafficLight" + std::to_string(lights);
			TrafficLightFacarde* rightLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + sin(angleInRadians + M_1_PI) * (g_fTileSize / 2) * pavementRatio + 225.0f, g_fTileSize * yUnit + cos(angleInRadians + M_1_PI) * (g_fTileSize / 2) * pavementRatio, 0.0f), fRot - 90.0f, 0.08f);
			pFacarde->addTrafficLight(rightLight);
			trafficLights->addChild(rightLight->root());

	}
	if (sPartName == "tile26") {
		// Create left light
		lights++;
		name = "trafficLight" + std::to_string(lights);
	    TrafficLightFacarde* rightLight = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", name), osg::Vec3(g_fTileSize * xUnit + (sin(angleInRadians + M_1_PI) * (g_fTileSize / 2)) * pavementRatio, g_fTileSize * yUnit + (cos(angleInRadians + M_1_PI) * (g_fTileSize / 2)) * pavementRatio + 225.0f, 0.0f), 0.0f, 0.08f);
		pFacarde->addTrafficLight(rightLight);
		trafficLights->addChild(rightLight->root());
	}

}

osg::Node* buildAnimatedVehicleAsset()
{
	osg::Group* pGroup = new osg::Group();

	osg::Geode* pGB = new osg::Geode();
	osg::ShapeDrawable* pGeomB = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f, 0.0f, 0.0f), 100.0f, 60.0f, 40.0f));
	osg::Material* pMat = new osg::Material();
	pMat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.3f, 0.3f, 0.1f, 1.0f));
	pMat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.8f, 0.8f, 0.3f, 1.0f));
	pMat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 0.6f, 1.0f));

	pGroup->addChild(pGB);
	pGB->addDrawable(pGeomB);

	pGB->getOrCreateStateSet()->setAttribute(pMat, osg::StateAttribute::ON || osg::StateAttribute::OVERRIDE);
	pGB->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE), osg::StateAttribute::ON || osg::StateAttribute::OVERRIDE);

	return pGroup;
}

osg::AnimationPath* createAnimationPath(raaAnimationPointFinders apfs, osg::Group* pRoadGroup)
{
	float fAnimTime = 0.0f;
	osg::AnimationPath* ap = new osg::AnimationPath();

	for (int i = 0; i < apfs.size(); i++)
	{
		float fDistance = 0.0f;
		osg::Vec3 vs;
		osg::Vec3 ve;

		vs.set(apfs[i].translation().x(), apfs[i].translation().y(), apfs[i].translation().z());

		if (i == apfs.size() - 1)
			ve.set(apfs[0].translation().x(), apfs[0].translation().y(), apfs[0].translation().z());
		else
			ve.set(apfs[i + 1].translation().x(), apfs[i + 1].translation().y(), apfs[i + 1].translation().z());

		float fXSqr = pow((ve.x() - vs.x()), 2);
		float fYSqr = pow((ve.y() - vs.y()), 2);
		float fZSqr = pow((ve.z() - vs.z()), 2);

		fDistance = sqrt(fXSqr + fYSqr);
		ap->insert(fAnimTime, osg::AnimationPath::ControlPoint(apfs[i].translation(), apfs[i].rotation()));
		fAnimTime += (fDistance / 10.0f);
	}

	return ap;
}

void buildRoad(osg::Group* pRoadGroup)
{
	addRoadTile("roadTJunction", "tile0", 0, 0, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile1", -1, 0, 0.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile2", -2, 0, 0.0f, pRoadGroup);	
	addRoadTile("roadStraight", "tile3", -2, 1, -90.0f, pRoadGroup);
	addRoadTile("roadTJunction", "tile4", -2, 2, -180.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile5", -1, 2, 0.0f, pRoadGroup);
	addRoadTile("roadXJunction", "tile6", 0, 2, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile7", 0, 1, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile8", -2, 3, -90.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile9", -2, 4, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile10", -1, 4, 0.0f, pRoadGroup);
	addRoadTile("roadTJunction", "tile11", 0, 4, 90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile12", 0, 3, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile13", 1, 4, 0.0f, pRoadGroup);
	addRoadTile("roadTJunction", "tile14", 2, 4, 90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile15", 2, 3, -90.0f, pRoadGroup);
	addRoadTile("roadXJunction", "tile16", 2, 2, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile17", 1, 2, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile18", 2, 1, -90.0f, pRoadGroup);
	addRoadTile("roadTJunction", "tile19", 2, 0, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile20", 1, 0, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile21", 3, 0, 0.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile22", 4, 0, 90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile23", 4, 1, -90.0f, pRoadGroup);
	addRoadTile("roadTJunction", "tile24", 4, 2, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile25", 3, 2, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile26", 4, 3, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile27", 3, 4, 0.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile28", 4, 4, -180.0f, pRoadGroup);
}

void createCarOne(osg::Group* pRoadGroup, osg::Group* pCarGroup)
{
	raaAnimationPointFinders apfs;
	osg::AnimationPath* ap = new osg::AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile0", 4, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile1", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile1", 3, pRoadGroup));
	
	apfs.push_back(raaAnimationPointFinder("tile2", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile2", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile2", 5, pRoadGroup));
	
	apfs.push_back(raaAnimationPointFinder("tile3", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile3", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile4", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile4", 6, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile4", 7, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile5", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile5", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile6", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 2, pRoadGroup));
	
	apfs.push_back(raaAnimationPointFinder("tile12", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile12", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile11", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 4, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile13", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile14", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile14", 6, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile15", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile15", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile16", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile16", 6, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile16", 7, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile17", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile17", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile6", 11, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 12, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 4, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile7", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile7", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile0", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 4, pRoadGroup));


	ap = createAnimationPath(apfs, pRoadGroup);
	
	// NOTE: you will need to extend or develop the car facarde to manage the animmation speed and events
	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car1"), ap, 30.0);
	pCarGroup->addChild(pCar->root());
}

void createCarTwo(osg::Group* pRoadGroup, osg::Group* pCarGroup) {
	raaAnimationPointFinders apfs;
	osg::AnimationPath* ap = new osg::AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile20", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile19", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile19", 9, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile19", 7, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile18", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile18", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile16", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile16", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile15", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile15", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile14", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile14", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile14", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile13", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile11", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 9, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 7, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile12", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile12", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile6", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 6, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 7, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile5", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile5", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile4", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile4", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile4", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile3", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile3", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile2", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile2", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile2", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile1", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile1", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile0", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile20", 2, pRoadGroup));
	ap = createAnimationPath(apfs, pRoadGroup);

	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car2"), ap, 30.0);
	pCarGroup->addChild(pCar->root());
}

void createCarThree(osg::Group* pRoadGroup, osg::Group* pCarGroup) {
	raaAnimationPointFinders apfs;
	osg::AnimationPath* ap = new osg::AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile24", 2, pRoadGroup));
	
	apfs.push_back(raaAnimationPointFinder("tile26", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 3, pRoadGroup));
	
	apfs.push_back(raaAnimationPointFinder("tile28", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile28", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile28", 2, pRoadGroup));
	
	apfs.push_back(raaAnimationPointFinder("tile27", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile14", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile14", 9, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile14", 7, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile15", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile15", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile16", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile16", 6, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile16", 7, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile17", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile17", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile6", 11, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 12, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 4, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile7", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile7", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile0", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile20", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile19", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile19", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile21", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile21", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile22", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile22", 1, pRoadGroup));	
	apfs.push_back(raaAnimationPointFinder("tile22", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile23", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile23", 3, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile24", 2, pRoadGroup));


	

	ap = createAnimationPath(apfs, pRoadGroup);

	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car3"), ap, 30.0);
	pCarGroup->addChild(pCar->root());
}

void createTrafficLights(osg::Group* pRoadGroup, osg::Group* pTrafficLightGroup)
{	
	
	addLightsTile("roadStraight", "tile20", 1, 0, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile17", 1, 2, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile13", 1, 4, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile5", -1, 2, 0.0f, pRoadGroup, pTrafficLightGroup);
	//addLightsTile("roadStraight", "tile25", 3, 2, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile12", 0, 3, -90.0f, pRoadGroup, pTrafficLightGroup);
	//addLightsTile("roadStraight", "tile15", 2, 3, -90.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile7", 0, 1, -90.0f, pRoadGroup, pTrafficLightGroup);
	//addLightsTile("roadStraight", "tile18",2, 1, -90.0f, pRoadGroup, pTrafficLightGroup);
	//addLightsTile("roadStraight", "tile21", 3, 0, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile1", -1, 0, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile10", -1, 4, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile3", -2, 1, -90.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile8", -2, 3, -90.0f, pRoadGroup, pTrafficLightGroup);
	// user-controlled traffic lights
	addLightsTile("roadStraight", "tile21", 3, 0, 0.0f, pRoadGroup, pTrafficLightGroup);
	addLightsTile("roadStraight", "tile26", 4, 3, -90.0f, pRoadGroup, pTrafficLightGroup);
}

osg::ref_ptr<osg::Group> create2DText(const std::string& fontFile, const std::string& textString,
	float characterSize, const osg::Vec3& position) {
	// Create a text node
	osg::ref_ptr<osgText::Text> text = new osgText::Text;

	// Set the text properties
	text->setFont(fontFile);
	text->setCharacterSize(characterSize);
	text->setAxisAlignment(osgText::Text::SCREEN);
	text->setPosition(position);
	text->setText(textString);

	// Create a geode to hold the text
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(text);

	// Create a group to hold the geode
	osg::ref_ptr<osg::Group> group = new osg::Group;
	group->addChild(geode);

	return group;
}

// sun model 
osg::Geode* SunAndMoon() {
	// std::cout << "sunnning.." << std::endl;
	osg::Geode* geodeSunAndMoon = new osg::Geode();
	geodeSunAndMoon->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(2000.0f, 2500.0f, 1000.0f), 200.0f)));
	osg::Texture2D* pTextBall = new osg::Texture2D;
	osg::StateSet* pStateSetBall = geodeSunAndMoon->getOrCreateStateSet();
	osg::ref_ptr<osg::Image>
		pImageBall(osgDB::readImageFile("../images/sun.tga"));
	if (!pImageBall) {
		std::cout << "Error: Couldn't find texture!" << std::endl;
	}
	pTextBall->setImage(pImageBall.get());
	pStateSetBall->setTextureAttributeAndModes(0, pTextBall, osg::StateAttribute::ON);

	return geodeSunAndMoon;
}

// day night simulation
class DayNightSimulation : public osg::NodeCallback
{
public:
	osg::Geode* geodeSunAndMoon = NULL;

	DayNightSimulation(osg::Camera* camera, const osg::Vec4& startColor, const osg::Vec4& endColor, float duration, osg::Geode* pSunAndMoon)
		: _camera(camera), _startColor(startColor), _endColor(endColor), _duration(duration), _timeElapsed(0.0f), _forward(true)
	{
		geodeSunAndMoon = pSunAndMoon;
		std::cout << "class geodeSunAndMoon: " << geodeSunAndMoon << std::endl;
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		//std::cout << "geodeSunAndMoon: " << geodeSunAndMoon << std::endl;
		// Calculate the interpolation factor based on the time elapsed and the total duration
		float t = _timeElapsed / _duration;

		// If we're going in the backward direction, flip the interpolation factor
		if (!_forward)
			t = 1.0f - t;

		// Interpolate the current color with the target color using the interpolation factor
		osg::Vec4 currentColor = _startColor * (1.0f - t) + _endColor * t;
		_camera->setClearColor(currentColor);

		// Increment the time elapsed
		double currentFrameTime = nv->getFrameStamp()->getReferenceTime();
		double deltaTime = currentFrameTime - _lastFrameTime;
		_timeElapsed += deltaTime;
		_lastFrameTime = currentFrameTime;

		// If the animation is complete in either direction, flip the direction flag and reset the time elapsed
		if (_timeElapsed >= _duration)
		{
			// here for end point
			if (changeTexture == true) {
				std::cout << "chaning texture.." << std::endl;
				// Assume "geode" is a pointer to the Geode you want to change the texture of
				osg::StateSet* stateSet = geodeSunAndMoon->getOrCreateStateSet();
				osg::Texture2D* pTextBall = new osg::Texture2D;
				osg::StateSet* pStateSetBall = geodeSunAndMoon->getOrCreateStateSet();
				osg::ref_ptr<osg::Image>
					pImageBall(osgDB::readImageFile("../images/moon.tga"));
				if (!pImageBall) {
					std::cout << "Error: Couldn't find texture!" << std::endl;
				}
				pTextBall->setImage(pImageBall.get());
				pStateSetBall->setTextureAttributeAndModes(0, pTextBall, osg::StateAttribute::ON);
				changeTexture = false;
			}
			else {
				std::cout << "chaning texture.." << std::endl;
				// Assume "geode" is a pointer to the Geode you want to change the texture of
				osg::StateSet* stateSet = geodeSunAndMoon->getOrCreateStateSet();
				osg::Texture2D* pTextBall = new osg::Texture2D;
				osg::StateSet* pStateSetBall = geodeSunAndMoon->getOrCreateStateSet();
				osg::ref_ptr<osg::Image>
					pImageBall(osgDB::readImageFile("../images/sun.tga"));
				if (!pImageBall) {
					std::cout << "Error: Couldn't find texture!" << std::endl;
				}
				pTextBall->setImage(pImageBall.get());
				pStateSetBall->setTextureAttributeAndModes(0, pTextBall, osg::StateAttribute::ON);
				changeTexture = true;
			}

			_forward = !_forward;
			_timeElapsed = 0.0f;
		}

		// Continue traversing the scene graph
		traverse(node, nv);
	}

private:
	osg::Camera* _camera;
	osg::Vec4 _startColor;
	osg::Vec4 _endColor;
	float _duration;
	float _timeElapsed;
	bool _forward;
	double _lastFrameTime;
	bool changeTexture = true; // true=sun fasle=moon
	osg::Geode* geodeSunAndMoonA;
};

osg::MatrixTransform* createEnvironmentStructure()
{
	osg::ref_ptr<osg::Box> northBox = new osg::Box(osg::Vec3(470, 2130, 250), 3320, 20, 500);
	osg::ref_ptr<osg::Box> southBox = new osg::Box(osg::Vec3(470, -240, 250), 3320, 20, 500);
	osg::ref_ptr<osg::Box> eastBox = new osg::Box(osg::Vec3(2130, 945, 250), 10, 2390, 500);
	osg::ref_ptr<osg::Box> westBox = new osg::Box(osg::Vec3(-1190, 945, 250), 10, 2390, 500);
	osg::ref_ptr<osg::Box> floorBox = new osg::Box(osg::Vec3(470, 945, -1), 3320, 2390, 0.1);

	osg::ref_ptr<osg::Geode> wallGeode = new osg::Geode();

	// Create shape drawables for each side
	osg::ref_ptr<osg::ShapeDrawable> northDrawable = new osg::ShapeDrawable(northBox);
	osg::ref_ptr<osg::ShapeDrawable> southDrawable = new osg::ShapeDrawable(southBox);
	osg::ref_ptr<osg::ShapeDrawable> eastDrawable = new osg::ShapeDrawable(eastBox);
	osg::ref_ptr<osg::ShapeDrawable> westDrawable = new osg::ShapeDrawable(westBox);
	osg::ref_ptr<osg::ShapeDrawable> floorDrawable = new osg::ShapeDrawable(floorBox);

	wallGeode->addDrawable(northDrawable.get());
	wallGeode->addDrawable(southDrawable.get());
	wallGeode->addDrawable(eastDrawable.get());
	wallGeode->addDrawable(westDrawable.get());
	wallGeode->addDrawable(floorDrawable.get());

	osg::MatrixTransform* mtWall = new osg::MatrixTransform;
	osg::Matrixf locationWall;
	locationWall.setTrans(osg::Vec3d(0, 0, 0));
	mtWall->setMatrix(locationWall);
	mtWall->addChild(wallGeode.get());

	// Create texture objects for each side
	osg::Texture2D* northTexture = new osg::Texture2D(osgDB::readImageFile("../images/city.jpg"));
	osg::Texture2D* southTexture = new osg::Texture2D(osgDB::readImageFile("../images/city.jpg"));
	osg::Texture2D* eastTexture = new osg::Texture2D(osgDB::readImageFile("../images/city.jpg"));
	osg::Texture2D* westTexture = new osg::Texture2D(osgDB::readImageFile("../images/city.jpg"));
	osg::Texture2D* floorTexture = new osg::Texture2D(osgDB::readImageFile("../images/grass.jpg"));

	// Set the texture objects for each side
	osg::StateSet* northStateSet = northDrawable->getOrCreateStateSet();
	osg::StateSet* southStateSet = southDrawable->getOrCreateStateSet();
	osg::StateSet* eastStateSet = eastDrawable->getOrCreateStateSet();
	osg::StateSet* westStateSet = westDrawable->getOrCreateStateSet();
	osg::StateSet* floorStateSet = floorDrawable->getOrCreateStateSet();

	northStateSet->setTextureAttributeAndModes(0, northTexture, osg::StateAttribute::ON);
	southStateSet->setTextureAttributeAndModes(0, southTexture, osg::StateAttribute::ON);
	eastStateSet->setTextureAttributeAndModes(0, eastTexture, osg::StateAttribute::ON);
	westStateSet->setTextureAttributeAndModes(0, westTexture, osg::StateAttribute::ON);
	floorStateSet->setTextureAttributeAndModes(0, floorTexture, osg::StateAttribute::ON);

	osg::Material* material = new osg::Material;
	material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	wallGeode->getOrCreateStateSet()->setAttribute(material);


	return mtWall;
}

osg::MatrixTransform* createAnimatedModel() {
	// Load the model
	osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("../models/boy.obj");

	// Create the animation path
	osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
	path->setLoopMode(osg::AnimationPath::LOOP);
	float radius = 500.0;
	float angle = osg::DegreesToRadians(90.0f);
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);
	// define control points
	osg::AnimationPath::ControlPoint CP0(osg::Vec3(-200.f, 210.f, 1.f), osg::Quat(osg::DegreesToRadians(-180.0f), osg::Vec3(0.f, 0.f, 1.f)));
	osg::AnimationPath::ControlPoint CP1(osg::Vec3(-200.f, 750.f, 1.f), osg::Quat(osg::DegreesToRadians(-90.0f), osg::Vec3(0.f, 0.f, 1.f)));
	osg::AnimationPath::ControlPoint CP2(osg::Vec3(-750.f, 750.f, 1.f), osg::Quat(osg::DegreesToRadians(0.0f), osg::Vec3(0.f, 0.f, 1.f)));
	osg::AnimationPath::ControlPoint CP3(osg::Vec3(-750.f, 210.f, 1.f), osg::Quat(osg::DegreesToRadians(90.0f), osg::Vec3(0.f, 0.f, 1.f)));
	osg::AnimationPath::ControlPoint CP4(osg::Vec3(-200.f, 210.f, 1.f));

	// Insert them to the path
	path->insert(2.0f, CP0); // time, point
	path->insert(4.0f, CP1);
	path->insert(6.0f, CP2);
	path->insert(8.0f, CP3);
	path->insert(10.0f, CP4);

	// Define animation path callback
	osg::ref_ptr<osg::AnimationPathCallback> APCallback = new osg::AnimationPathCallback(path.get());

	osg::MatrixTransform* mt = new osg::MatrixTransform;
	mt->addChild(model);
	g_pRoot->addChild(mt);

	// Update the matrix transform with the animation path
	mt->setUpdateCallback(APCallback.get());

	osg::ref_ptr<osg::Group> lightGroup(new osg::Group);
	osg::ref_ptr<osg::StateSet>lightSS(g_pRoot->getOrCreateStateSet());

	return mt;
}

osg::ref_ptr<osg::Geode> createText(const std::string& text, const osg::Vec4& color, float x, float y)
{
	// Create a text node
	osg::ref_ptr<osgText::Text> textNode = new osgText::Text;
	textNode->setText(text);
	textNode->setColor(color);
	textNode->setCharacterSize(32);
	textNode->setFont("arial.ttf");

	// Create a geode to hold the text node
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(textNode);

	// Position the text node
	osg::Vec3 pos(x, y, 0);
	textNode->setPosition(pos);

	// Set up the projection matrix for 2D rendering
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, 800, 0, 600));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);

	// Add the geode to the camera
	camera->addChild(geode);

	// Add the camera to the root node
	g_pRoot->addChild(camera);

	return geode;
}

//osg::Node* loadModel(const std::string& filename)
//{
//	// Load the model
//	osg::Node* model = osgDB::readNodeFile(filename);
//
//	// Check if the model loaded successfully
//	if (!model)
//	{
//		std::cout << "Error loading model from file: " << filename << std::endl;
//		return nullptr;
//	}
//
//	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
//	texture->setImage(osgDB::readImageFile("../models/boyT.jpg"));
//
//	osg::StateSet* state = model->getOrCreateStateSet();
//	state->setTextureAttributeAndModes(0, texture.get());
//
//	// Scale the model to an appropriate size
//	osg::ComputeBoundsVisitor boundsVisitor;
//	model->accept(boundsVisitor);
//	osg::BoundingBox bounds = boundsVisitor.getBoundingBox();
//	float scaleFactor = 100.0f / bounds.radius();
//	osg::MatrixTransform* scaleTransform = new osg::MatrixTransform;
//	scaleTransform->setMatrix(osg::Matrix::scale(scaleFactor, scaleFactor, scaleFactor));
//	scaleTransform->addChild(model);
//	return scaleTransform;
//}

// class to click on 3D nodes (bugs)
class PickingHandler : public osgGA::GUIEventHandler {
public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
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
				std::string nodeName = nodePath.back()->getName();
				std::cout << "Clicked on node: " << nodeName << std::endl;
			}
		}
		return false;
	}
};

int main(int argc, char** argv)
{
	raaAssetLibrary::start();
	raaTrafficSystem::start();

	// osgViewer::Viewer viewer;

	for (int i = 0; i < argc; i++)
	{
		if (std::string(argv[i]) == "-d") g_sDataPath = argv[++i];
	}

	// the root of the scene - use for rendering
	g_pRoot = new osg::Group();
	g_pRoot->ref();

	osg::Geode* g_pSun = SunAndMoon(); // call SunAndMoon function
	g_pRoot->addChild(g_pSun);
	// std::cout << "main geodeSunAndMoon: " << g_pSun << std::endl;

	osg::MatrixTransform* g_pWall = createEnvironmentStructure();
	g_pRoot->addChild(g_pWall);

	 osg::MatrixTransform* g_pWalking = createAnimatedModel();
	 g_pRoot->addChild(g_pWalking);

	 osg::Geode* text = createText("Use \"q\" to control the TrafficLight14\nUse \"w\" to control the TrafficLight15", osg::Vec4(1.0, 1.0, 1.0, 1.0), 100, 100);
	 // g_pRoot->addChild(text);

	 // Create and add the model to the root node
	 // osg::Node* boy = loadModel("../models/boy.obj");
	 // g_pRoot->addChild(boy);

	// build asset library - instances or clones of parts can be created from this
	raaAssetLibrary::loadAsset("roadStraight", g_sDataPath + "roadStraight.osgb");
	raaAssetLibrary::loadAsset("roadCurve", g_sDataPath + "roadCurve.osgb");
	raaAssetLibrary::loadAsset("roadTJunction", g_sDataPath + "roadTJunction.osgb");
	raaAssetLibrary::loadAsset("roadXJunction", g_sDataPath + "roadXJunction.osgb");
	raaAssetLibrary::loadAsset("trafficLight", g_sDataPath + "raaTrafficLight.osgb");
	raaAssetLibrary::insertAsset("vehicle", buildAnimatedVehicleAsset());

	// add menu to control cars
	osgWidget::WindowManager* wm = new osgWidget::WindowManager(
		&viewer,
		640.0f,
		480.0f,
		MASK_2D,
		osgWidget::WindowManager::WM_PICK_DEBUG
	);

	osgWidget::Window* menu = new osgWidget::Box("menu", osgWidget::Box::HORIZONTAL);

	menu->addWidget(new CarControlButton("car1"));
	menu->addWidget(new CarControlButton("car2"));
	menu->addWidget(new CarControlButton("car3"));
	
	wm->addChild(menu);

	menu->getBackground()->setColor(1.0f, 1.0f, 1.0f, 0.0f);
	menu->resizePercent(100.0f);

	// Create a group for the text
	// osg::ref_ptr<osg::Group> textGroup = create2DText("arial.ttf", "Hello, world!", 20.0f, osg::Vec3(50, 50, 0));
	// g_pRoot->addChild(textGroup);

	// add a group node to the scene to hold the road sub-tree
	osg::Group* pRoadGroup = new osg::Group();
	pRoadGroup->setName("Roads Group");
	g_pRoot->addChild(pRoadGroup);

	// Traffic Lights
	osg::Group* pTrafficLightGroup = new osg::Group();
	pTrafficLightGroup->setName("Traffic Lights Group");
	g_pRoot->addChild(pTrafficLightGroup);
	createTrafficLights(pRoadGroup, pTrafficLightGroup);

	// Add a group node to the scene to hold the cars sub-tree
	osg::Group* pCarGroup = new osg::Group();
	pCarGroup->setName("Traffic Lights Group");
	g_pRoot->addChild(pCarGroup);

	// Create road
	buildRoad(pRoadGroup);

	// Add car one
	createCarOne(pRoadGroup, pCarGroup);

	// Add car two
	createCarTwo(pRoadGroup, pCarGroup);

	//// Add car three
	createCarThree(pRoadGroup, pCarGroup);
	
	// osg setup stuff
	osg::GraphicsContext::Traits* pTraits = new osg::GraphicsContext::Traits();
	pTraits->x = 20;
	pTraits->y = 20;
	pTraits->width = 600;
	pTraits->height = 480;
	pTraits->windowDecoration = true;
	pTraits->doubleBuffer = true;
	pTraits->sharedContext = 0;

	osg::GraphicsContext* pGC = osg::GraphicsContext::createGraphicsContext(pTraits);
	osgGA::KeySwitchMatrixManipulator* pKeyswitchManipulator = new osgGA::KeySwitchMatrixManipulator();
	pKeyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
	pKeyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
	pKeyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
	viewer.setCameraManipulator(pKeyswitchManipulator);
	osg::Camera* pCamera = viewer.getCamera();
	pCamera->setGraphicsContext(pGC);
	pCamera->setViewport(new osg::Viewport(0, 0, pTraits->width, pTraits->height));
	osg::Vec4 startColor(0.5f, 0.8f, 1.0f, 1.0f);
	osg::Vec4 endColor(0.0f, 0.1f, 0.3f, 1.0f);
	float duration = 15.0f; // in seconds
	pCamera->setUpdateCallback(new DayNightSimulation(pCamera, startColor, endColor, duration, g_pSun));

	// add own event handler - this currently switches on an off the animation points
	viewer.addEventHandler(new raaInputController(g_pRoot));

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	// add the thread model handler
	viewer.addEventHandler(new osgViewer::ThreadingHandler);

	// add the window size toggle handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the record camera path handler
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

	// add the LOD Scale handler
	viewer.addEventHandler(new osgViewer::LODScaleHandler);

	// add the screen capture handler
	viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

	// add the clicking on 3D nodes
	// viewer.addEventHandler(new PickingHandler);

	// set the scene to render
	//viewer.setSceneData(g_pRoot);

	//viewer.realize();
	//return osgWidget::createExample(viewer, wm, g_pRoot);

	//return viewer.run();
	

//	osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile("../../Data/osgcool.osgt");
	osg::ref_ptr<osg::Node> model = dynamic_cast<osg::Node*>(g_pRoot);

	model->setNodeMask(MASK_3D);

	// Set the clear color to white
	// viewer.getCamera()->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	//dayNightSimulation(g_pRoot);
	//viewer.getCamera()->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// call the dayNightSimulation function with the viewer
	std::cout << "starting.." << std::endl;
	//viewer.realize();

	//// Set up the viewer
	//viewer.setSceneData(g_pRoot);
	//viewer.realize();

	//// Run the viewer
	//while (!viewer.done())
	//{
	//	viewer.frame();
	//}
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile("../images/sun.tga");
	if (!image)
	{
		std::cerr << "Failed to load image file." << std::endl;
		return 1;
	}

	return osgWidget::createExample(viewer, wm, model.get());
}
