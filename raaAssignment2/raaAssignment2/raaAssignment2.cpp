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
#include "PickingHandler.h"
#include "MenuButton.h"
#include "ButtonCommandSet.h"
#include "raaFinder.h"
#include <osgDB/ReadFile>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Label>


// For now this is just an example, but osgWidget::Menu will later be it's own Window.
// I just wanted to get this out there so that people could see it was possible.

const unsigned int MASK_2D = 0xF0000000;
const unsigned int MASK_3D = 0x0F000000;
osgViewer::Viewer viewer;

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
std::vector<raaAnimationPointFinders> lstAnimPointFinders;
osg::Group* g_pRoot = 0; // root of the sg
float g_fTileSize = 472.441f; // width/depth of the standard road tiles
std::string g_sDataPath = "../../Data/";

const float FACE_FORWARD = 0.0f;
const float FACE_LEFT = -90.0f;
const float FACE_UP = 180.0f;
const float FACE_RIGHT = 90.0f;

int lights = 0;

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

osg::Node* buildAnimatedVehicleAsset()
{
	osg::Group* pGroup = new osg::Group();

	osg::MatrixTransform* pCarL = new osg::MatrixTransform();
	osg::Node* pCarN = osgDB::readNodeFile("../models/car-veyron.OSGB");
	raaBoundCalculator* bound = new raaBoundCalculator(pCarN);
	pCarL->preMult(osg::Matrix::scale(10.0f, 10.0f, 10.0f));
	pCarL->preMult(osg::Matrix::rotate(osg::PI_2, osg::Vec3(0.0f, 0.0f, 1.0f)));
	pCarL->postMult(osg::Matrix::translate(osg::Vec3(0.0f, 0.0f, 30.0f)));
	pCarL->addChild(pCarN);
	pGroup->addChild(pCarL);

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

void initAnimationPoints(osg::Group* pRoadGroup)
{
	{
		raaAnimationPointFinders apfs;
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
		lstAnimPointFinders.push_back(apfs);
	}
	{
		raaAnimationPointFinders apfs;
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
		lstAnimPointFinders.push_back(apfs);
	}
	{
		raaAnimationPointFinders apfs;
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
		lstAnimPointFinders.push_back(apfs);
	}
	{
		raaAnimationPointFinders apfs;
		apfs.push_back(raaAnimationPointFinder("tile19", 8, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile19", 9, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile19", 7, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile16", 8, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile16", 6, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile16", 10, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile24", 0, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile24", 1, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile24", 2, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile28", 0, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile28", 1, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile28", 2, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile11", 8, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile11", 9, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile11", 7, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 5, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 15, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 7, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile19", 8, pRoadGroup));
		lstAnimPointFinders.push_back(apfs);
	}
	{
		raaAnimationPointFinders apfs;
		apfs.push_back(raaAnimationPointFinder("tile8", 1, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile8", 3, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile9", 3, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile9", 4, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile9", 5, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile14", 5, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile14", 6, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile14", 7, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile19", 0, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile19", 3, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile19", 4, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile0", 5, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile0", 6, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile0", 7, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 8, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 9, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 7, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile4", 0, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile4", 3, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile4", 4, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile8", 1, pRoadGroup));
		lstAnimPointFinders.push_back(apfs);
	}
	{
		raaAnimationPointFinders apfs;
		apfs.push_back(raaAnimationPointFinder("tile12", 2, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile12", 0, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 5, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 14, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile6", 10, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile24", 0, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile24", 1, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile24", 2, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile28", 0, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile28", 1, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile28", 2, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile11", 8, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile11", 9, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile11", 7, pRoadGroup));
		apfs.push_back(raaAnimationPointFinder("tile12", 2, pRoadGroup));
		lstAnimPointFinders.push_back(apfs);
	}
}

void createCar(osg::Group* pRoadGroup, osg::Group* pCarGroup)
{
	// Rand select three car driving routes
	std::set<int> lstIndexs;
	while (lstIndexs.size() < 3)
	{
		int nIndex = rand() % 6;
		if (lstIndexs.find(nIndex) != lstIndexs.end())
			continue;

		lstIndexs.insert(nIndex);
	}

	// Each route is to place two cars, where the second car travels 2.5 seconds earlier than the first car
	int nIndex = 1;
	for (std::set<int>::iterator iter = lstIndexs.begin(); iter != lstIndexs.end(); ++iter)
	{
		osg::AnimationPath* ap = new osg::AnimationPath();
		ap = createAnimationPath(lstAnimPointFinders[*iter], pRoadGroup);

		raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car" + std::to_string(nIndex)), ap, 30.0);
		pCarGroup->addChild(pCar->root());
		++nIndex;

		osg::AnimationPath* ap2 = new osg::AnimationPath();
		ap2 = createAnimationPath(lstAnimPointFinders[*iter], pRoadGroup);
		raaCarFacarde* pCar2 = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car" + std::to_string(nIndex)), ap, 30.0);
		pCarGroup->addChild(pCar2->root());
		pCar2->setAnimationTime(25);
		++nIndex;
	}
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
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
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
	osg::Matrixd scalingMatrix;
	scalingMatrix.makeScale(0.5, 0.5, 0.5); // Set the scale factors
	osg::ref_ptr<osg::MatrixTransform> scalingTransform = new osg::MatrixTransform(scalingMatrix);
	scalingTransform->addChild(model);
	mt->addChild(scalingTransform);
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
	textNode->setCharacterSize(18);
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

void initFacardeStruct(osg::Group* pGroup)
{
	osg::MatrixTransform* pTranslation = new osg::MatrixTransform();
	pTranslation->setName("Translation");
	osg::MatrixTransform* pRotation = new osg::MatrixTransform();
	pRotation->setName("Rotation");
	osg::MatrixTransform* pScale = new osg::MatrixTransform();
	pScale->setName("Scale");
	pTranslation->addChild(pRotation);
	pRotation->addChild(pScale);
	pGroup->addChild(pTranslation);
}

// Add traffic light to x junction
void addXJunctionTrafficLight(std::string sJunctionName, int xUnit, int yUnit, float fRot, osg::Group* pParent, int& nIndex)
{
	osg::Group* pTrafficLightTile = new osg::Group();
	pTrafficLightTile->setName(sJunctionName);
	initFacardeStruct(pTrafficLightTile);
	pParent->addChild(pTrafficLightTile);
	TrafficLightControl* pLightControl = new TrafficLightControl(pTrafficLightTile, osg::Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot, 1);

	// Each x junction defines 4 traffic lights, of which 1 and 3 are a group, 2 and 4 are a group, and the status of the traffic lights in each group must be consistent.
	TrafficLightFacarde* pTrafficLight1 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "trafficLight" + std::to_string(nIndex)), osg::Vec3(-180, 180, 0.0f), -90, 0.08f);
	pLightControl->scale()->addChild(pTrafficLight1->root());
	pTrafficLight1->setRedTrafficLight();
	pTrafficLight1->m_iTrafficLightIndex = 1;
	pLightControl->addTrafficLight(pTrafficLight1);
	++nIndex;

	TrafficLightFacarde* pTrafficLight2 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "trafficLight" + std::to_string(nIndex)), osg::Vec3(180.0f, 180.0f, 0.0f), 180.0f, 0.08f);
	pLightControl->scale()->addChild(pTrafficLight2->root());
	pTrafficLight2->setGreenTrafficLight();
	pTrafficLight2->m_iTrafficLightIndex = 2;
	pLightControl->addTrafficLight(pTrafficLight2);
	++nIndex;

	TrafficLightFacarde* pTrafficLight3 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "trafficLight" + std::to_string(nIndex)), osg::Vec3(180.0f, -180.0f, 0.0f), 90.0f, 0.08f);
	pLightControl->scale()->addChild(pTrafficLight3->root());
	pTrafficLight3->setRedTrafficLight();
	pTrafficLight3->m_iTrafficLightIndex = 3;
	pLightControl->addTrafficLight(pTrafficLight3);
	++nIndex;


	TrafficLightFacarde* pTrafficLight4 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "trafficLight" + std::to_string(nIndex)), osg::Vec3(-180.0f, -180.0f, 0.0f), 0.0f, 0.08f);
	pLightControl->scale()->addChild(pTrafficLight4->root());
	pTrafficLight4->setGreenTrafficLight();
	pTrafficLight4->m_iTrafficLightIndex = 4;
	pLightControl->addTrafficLight(pTrafficLight4);
	++nIndex;
}
// Add traffic light to T junction
void addTJunctionTrafficLight(std::string sJunctionName, int xUnit, int yUnit, float fRot, osg::Group* pParent, int& nIndex)
{
	osg::Group* pTrafficLightTile = new osg::Group();
	pTrafficLightTile->setName(sJunctionName);
	initFacardeStruct(pTrafficLightTile);
	pParent->addChild(pTrafficLightTile);
	TrafficLightControl* pLightControl = new TrafficLightControl(pTrafficLightTile, osg::Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot, 1);

	// Each T junction defines 3 traffic lights, among which 1 and 3 are a group, and the status of the traffic lights in each group must be consistent.
	TrafficLightFacarde* pTrafficLight1 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "trafficLight" + std::to_string(nIndex)), osg::Vec3(-180.0f, -180.0f, 0.0f), 0.0f, 0.08f);
	pTrafficLight1->setGreenTrafficLight();
	pLightControl->scale()->addChild(pTrafficLight1->root());
	pLightControl->addTrafficLight(pTrafficLight1);
	pTrafficLight1->m_iTrafficLightIndex = 1;
	++nIndex;

	TrafficLightFacarde* pTrafficLight2 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "trafficLight" + std::to_string(nIndex)), osg::Vec3(-180, 180, 0.0f), -90, 0.08f);
	pTrafficLight2->setRedTrafficLight();
	pLightControl->scale()->addChild(pTrafficLight2->root());
	pLightControl->addTrafficLight(pTrafficLight2);
	pTrafficLight2->m_iTrafficLightIndex = 2;
	++nIndex;

	TrafficLightFacarde* pTrafficLight3 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "trafficLight" + std::to_string(nIndex)), osg::Vec3(180.0f, 180.0f, 0.0f), 180.0f, 0.08f);
	pTrafficLight3->setGreenTrafficLight();
	pLightControl->scale()->addChild(pTrafficLight3->root());
	pLightControl->addTrafficLight(pTrafficLight3);
	pTrafficLight3->m_iTrafficLightIndex = 3;
	++nIndex;
}

// create the traffic lights in the scene
void createTrafficLightGroup()
{
	int nIndex = 1;
	osg::Group* pTrafficLightGroup = new osg::Group();
	g_pRoot->addChild(pTrafficLightGroup);
	addXJunctionTrafficLight("XJunction1Light", 2, 2, 0.0f, pTrafficLightGroup, nIndex);
	addXJunctionTrafficLight("XJunction2Light", 0, 2, 0.0f, pTrafficLightGroup, nIndex);
	addTJunctionTrafficLight("TJunction1Light", 0, 0, -90.0f, pTrafficLightGroup, nIndex);
	addTJunctionTrafficLight("TJunction2Light", 2, 0, -90.0f, pTrafficLightGroup, nIndex);
	addTJunctionTrafficLight("TJunction3Light", 0, 4, 90.0f, pTrafficLightGroup, nIndex);
	addTJunctionTrafficLight("TJunction4Light", 2, 4, 90.0f, pTrafficLightGroup, nIndex);
	addTJunctionTrafficLight("TJunction5Light", -2, 2, 180.0f, pTrafficLightGroup, nIndex);
	addTJunctionTrafficLight("TJunction6Light", 4, 2, 0.0f, pTrafficLightGroup, nIndex);
}

class VehicleFollowingCommand : public CommandBase
{
public:
	VehicleFollowingCommand(MenuButton* pMenu)
	{
		m_pMenu = pMenu;
	}
	bool operator()()
	{
		if (!m_pMenu)
			return false;

		if (m_pMenu->getLabel().compare("Enable Follow") == 0)
		{
			m_pMenu->setLabel("Disable Follow");
			PickingHandler::instance()->setAllowClick(false);
		}
		else
		{
			m_pMenu->setLabel("Enable Follow");
			PickingHandler::instance()->setAllowClick(true);
		}
		return true;
	}

protected:
	MenuButton* m_pMenu;
};

class VehicleControlCommand : public CommandBase
{
public:
	VehicleControlCommand(const std::string& strName)
	{
		mstrName = strName;
	}
	bool operator()()
	{
		// Find the car with the specified name from the scene
		raaFinder<osg::Node> car(mstrName, g_pRoot);
		if (car.node() && car.node()->getNumParents() > 0)
		{
			osg::Group* pParent = car.node()->getParent(0);
			if (pParent)
			{
				// Set whether the car is stopped or running according to the state of the car
				raaCarFacarde* pCarFacarde = dynamic_cast<raaCarFacarde*>(pParent->getUpdateCallback());
				if (pCarFacarde)
					pCarFacarde->setManualStop(!pCarFacarde->getManualStop());
			}
		}
		return true;
	}

protected:
	std::string mstrName;
};

class VehicleDriveCommand : public CommandBase
{
public:
	VehicleDriveCommand(MenuButton* pMenu)
	{
		m_pMenu = pMenu;
	}
	bool operator()()
	{
		if (!m_pMenu)
			return false;

		// If the current is the first person, then switch directly to the third person, otherwise switch to the first person
		if (m_pMenu->getLabel().compare("First Person") == 0)
		{
			m_pMenu->setLabel("Third Person");
			PickingHandler::instance()->setFirstPerson(false);
		}
		else
		{
			m_pMenu->setLabel("First Person");
			PickingHandler::instance()->setFirstPerson(true);
		}
		return true;
	}

protected:
	MenuButton* m_pMenu;
};

int main(int argc, char** argv)
{
	raaAssetLibrary::start();
	raaTrafficSystem::start();

	for (int i = 0; i < argc; i++)
	{
		if (std::string(argv[i]) == "-d") g_sDataPath = argv[++i];
	}

	// the root of the scene - use for rendering
	g_pRoot = new osg::Group();
	g_pRoot->ref();

	osg::Geode* g_pSun = SunAndMoon(); // call SunAndMoon function
	g_pRoot->addChild(g_pSun);

	osg::MatrixTransform* g_pWall = createEnvironmentStructure();
	g_pRoot->addChild(g_pWall);

	osg::MatrixTransform* g_pWalking = createAnimatedModel();
	g_pRoot->addChild(g_pWalking);

	osg::Geode* text = createText("Use \"q\" to control the TrafficLight5\nUse \"w\" to control the TrafficLight2\nUse \"x\" to toggle user interaction and 2D elements\nUse the UI buttonts from first to sixth to toggle car movemenet\nClick with cursor on a car to take control\nUse arrow up to accelerate and arrow down to stop\nClick seventth UI button to tggole view mode while in car view mode\nUse \"z\" to exit car view mode", osg::Vec4(1.0, 1.0, 1.0, 1.0), 5, 150);

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
		osgWidget::WindowManager::WM_USE_RENDERBINS
	);

	osgWidget::Window* menu = new osgWidget::Box("menu", osgWidget::Box::HORIZONTAL);

	menu->addWidget(new MenuButton("car1", 1));
	menu->addWidget(new MenuButton("car2", 2));
	menu->addWidget(new MenuButton("car3", 3));
	menu->addWidget(new MenuButton("car4", 4));
	menu->addWidget(new MenuButton("car5", 5));
	menu->addWidget(new MenuButton("car6", 6));

	ButtonCommandSet::instance()->addCommand(1, new VehicleControlCommand("car1"));
	ButtonCommandSet::instance()->addCommand(2, new VehicleControlCommand("car2"));
	ButtonCommandSet::instance()->addCommand(3, new VehicleControlCommand("car3"));
	ButtonCommandSet::instance()->addCommand(4, new VehicleControlCommand("car4"));
	ButtonCommandSet::instance()->addCommand(5, new VehicleControlCommand("car5"));
	ButtonCommandSet::instance()->addCommand(6, new VehicleControlCommand("car6"));
	MenuButton* pDriveMenu = new MenuButton("First Person", 7);
	menu->addWidget(pDriveMenu);
	ButtonCommandSet::instance()->addCommand(7, new VehicleDriveCommand(pDriveMenu));

	wm->addChild(menu);

	menu->getBackground()->setColor(1.0f, 1.0f, 1.0f, 0.0f);
	menu->resizePercent(100.0f);

	// add a group node to the scene to hold the road sub-tree
	osg::Group* pRoadGroup = new osg::Group();
	pRoadGroup->setName("Roads Group");
	g_pRoot->addChild(pRoadGroup);

	// Traffic Lights
	createTrafficLightGroup();

	// Add a group node to the scene to hold the cars sub-tree
	osg::Group* pCarGroup = new osg::Group();
	pCarGroup->setName("Traffic Lights Group");
	g_pRoot->addChild(pCarGroup);

	// Create road
	buildRoad(pRoadGroup);
	initAnimationPoints(pRoadGroup);
	createCar(pRoadGroup, pCarGroup);

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
	viewer.addEventHandler(PickingHandler::instance());

	//	osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile("../../Data/osgcool.osgt");
	osg::ref_ptr<osg::Node> model = dynamic_cast<osg::Node*>(g_pRoot);

	model->setNodeMask(MASK_3D);

	osg::ref_ptr<osg::Image> image = osgDB::readImageFile("../images/sun.tga");
	if (!image)
	{
		std::cerr << "Failed to load image file." << std::endl;
		return 1;
	}

	return osgWidget::createExample(viewer, wm, model.get());
}