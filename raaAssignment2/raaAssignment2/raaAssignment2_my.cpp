#include <windows.h>
#include <iostream>
#include <math.h>
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



typedef std::vector<raaAnimationPointFinder>raaAnimationPointFinders;
osg::Group* g_pRoot = 0; // root of the sg
float g_fTileSize = 472.441f; // width/depth of the standard road tiles
std::string g_sDataPath = "../../Data/";

const float FACE_FORWARD = 0.0f;
const float FACE_LEFT = -90.0f;
const float FACE_UP = 180.0f;
const float FACE_RIGHT = 90.0f;

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
	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car1"), ap, 50.0);
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

	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car2"), ap, 50.0);
	pCarGroup->addChild(pCar->root());
}

void createCarThree(osg::Group* pRoadGroup, osg::Group* pCarGroup) {
	raaAnimationPointFinders apfs;
	osg::AnimationPath* ap = new osg::AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile24", 2, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile26", 1, pRoadGroup));
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

	apfs.push_back(raaAnimationPointFinder("tile18", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile18", 0, pRoadGroup));

	apfs.push_back(raaAnimationPointFinder("tile19", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile19", 1, pRoadGroup));
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

	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle", "car3"), ap, 40.0);
	pCarGroup->addChild(pCar->root());
}

void createTrafficLights(osg::Group* pTrafficLightGroup) {
	
#pragma region Tile 0 T Junction
	osg::Group* tile0TJunction = new osg::Group();
	pTrafficLightGroup->addChild(tile0TJunction);

	TrafficLightFacarde* tlFacarde0 = // Horizontal
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 0"),
			osg::Vec3(-175.0f, 200.0f, 0.0f), FACE_LEFT, 0.08f);

	TrafficLightFacarde* tlFacarde1 = // Horizontal
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 1"),
			osg::Vec3(-175.0f, -175.0f, 0.0f), FACE_LEFT, 0.08f);

	TrafficLightFacarde* tlFacarde2 = // Vertical
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 2"),
			osg::Vec3(200.0f, 200.0f, 0.0f), FACE_UP, 0.08f);

	tile0TJunction->addChild(tlFacarde0->root());
	tile0TJunction->addChild(tlFacarde1->root());
	tile0TJunction->addChild(tlFacarde2->root());

	TrafficLightControl* pTile0TJunctionController =
		new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadTJunction", "TJ0"),
			osg::Vec3(0.0f, 0.0f, 0.0f), -90.0f, 1.0f);
	pTile0TJunctionController->setName("Tile 0 T Junction Controller");
	g_pRoot->addChild(pTile0TJunctionController->root());

	pTile0TJunctionController->addTrafficLight(tlFacarde0);
	pTile0TJunctionController->addTrafficLight(tlFacarde1);
	pTile0TJunctionController->addTrafficLight(tlFacarde2);
#pragma endregion

#pragma region Tile 6 X Junction
	osg::Group* tile6XJunction = new osg::Group();
	pTrafficLightGroup->addChild(tile6XJunction);

	TrafficLightFacarde* tlFacarde3 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 3"),
			osg::Vec3(-175.0f, 1125.0f, 0.0f), FACE_LEFT, 0.08f);

	TrafficLightFacarde* tlFacarde4 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 4"),
			osg::Vec3(200.0f, 750.0f, 0.0f), FACE_RIGHT, 0.08f);


	TrafficLightFacarde* tlFacarde5 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 5"),
			osg::Vec3(-175.0f, 750.0f, 0.0f), FACE_FORWARD, 0.08f);

	TrafficLightFacarde* tlFacarde6 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 6"),
			osg::Vec3(200.0f, 1125.0f, 0.0f), FACE_UP, 0.08f);


	tile6XJunction->addChild(tlFacarde3->root());
	tile6XJunction->addChild(tlFacarde4->root());
	tile6XJunction->addChild(tlFacarde5->root());
	tile6XJunction->addChild(tlFacarde6->root());


	TrafficLightControl* Tile6XJunctionController = new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadXJunction", "XJ6"),
		osg::Vec3(0.0f, 945.0f, 0.0f), 0.0f, 1.0f);
	Tile6XJunctionController->setName("Tile 6 X Junction Controller");
	std::cout << Tile6XJunctionController->getCompoundClassName() << std::endl;

	g_pRoot->addChild(Tile6XJunctionController->root());

	Tile6XJunctionController->addTrafficLight(tlFacarde3);
	Tile6XJunctionController->addTrafficLight(tlFacarde4);
	Tile6XJunctionController->addTrafficLight(tlFacarde5);
	Tile6XJunctionController->addTrafficLight(tlFacarde6);
#pragma endregion


#pragma region Tile 4 T Junction

	osg::Group* Tile4TJunction = new osg::Group();
	pTrafficLightGroup->addChild(Tile4TJunction);

	TrafficLightFacarde* tlFacarde7 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 8"),
			osg::Vec3(-750.0f, 750.0f, 0.0f), FACE_RIGHT, 0.08f);

	TrafficLightFacarde* tlFacarde8 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 9"),
			osg::Vec3(-1125.0f, 750.0f, 0.0f), FACE_FORWARD, 0.08f);

	TrafficLightFacarde* tlFacarde9 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 10"),
			osg::Vec3(-750.0f, 1125.0f, 0.0f), FACE_UP, 0.08f);

	Tile4TJunction->addChild(tlFacarde7->root());
	Tile4TJunction->addChild(tlFacarde8->root());
	Tile4TJunction->addChild(tlFacarde9->root());

	TrafficLightControl* Tile4TJunctionController =
		new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadTJunction", "TJ4"),
			osg::Vec3(-945.0f, 945.0f, 0.0f), -180.0f, 1.0f);

	Tile4TJunctionController->setName("Tile 4 T Junction Controller");
	
	g_pRoot->addChild(Tile4TJunctionController->root());

	Tile4TJunctionController->addTrafficLight(tlFacarde7);
	Tile4TJunctionController->addTrafficLight(tlFacarde8);
	Tile4TJunctionController->addTrafficLight(tlFacarde9);
#pragma endregion

#pragma region Tile 11 T Junction

	osg::Group* Tile11TJunction = new osg::Group();
	pTrafficLightGroup->addChild(Tile11TJunction);

	TrafficLightFacarde* tlFacarde11 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 11"),
			osg::Vec3(-175.0f, 1700.0f, 0.0f), FACE_FORWARD, 0.08f);

	TrafficLightFacarde* tlFacarde12 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 12"),
			osg::Vec3(200.0f, 1700.0f, 0.0f), FACE_RIGHT, 0.08f);

	TrafficLightFacarde* tlFacarde13 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 13"),
			osg::Vec3(-175, 2050.0f, 0.0f), FACE_LEFT, 0.08f);

	Tile11TJunction->addChild(tlFacarde11->root());
	Tile11TJunction->addChild(tlFacarde12->root());
	Tile11TJunction->addChild(tlFacarde13->root());

	TrafficLightControl* Tile11TJunctionController =
		new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadTJunction", "TJ11"),
			osg::Vec3(0.0f, 1890.0f, 0.0f), 90.0f, 1.0f);

	Tile11TJunctionController->setName("Tile 11 T Junction Controller");

	g_pRoot->addChild(Tile11TJunctionController->root());

	Tile11TJunctionController->addTrafficLight(tlFacarde11);
	Tile11TJunctionController->addTrafficLight(tlFacarde12);
	Tile11TJunctionController->addTrafficLight(tlFacarde13);
#pragma endregion

#pragma region Tile 16 T Junction
	osg::Group* Tile16TJunction = new osg::Group();
	pTrafficLightGroup->addChild(Tile16TJunction);

	TrafficLightFacarde* tlFacarde14 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 14"),
			osg::Vec3(750.0f, 750.0f, 0.0f), FACE_FORWARD, 0.08f);

	TrafficLightFacarde* tlFacarde15 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 15"),
			osg::Vec3(750.0f, 1125.0f, 0.0f), FACE_LEFT, 0.08f);

	TrafficLightFacarde* tlFacarde16 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 16"),
			osg::Vec3(1125.0f, 1125.0f, 0.0f), FACE_UP, 0.08f);

	TrafficLightFacarde* tlFacarde17 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 17"),
			osg::Vec3(1125.0f, 750.0f, 0.0f), FACE_RIGHT, 0.08f);

	Tile16TJunction->addChild(tlFacarde14->root());
	Tile16TJunction->addChild(tlFacarde15->root());
	Tile16TJunction->addChild(tlFacarde16->root());
	Tile16TJunction->addChild(tlFacarde17->root());

	TrafficLightControl* Tile16TJunctionController =
		new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadTJunction", "XJ16"),
			osg::Vec3(-945.0f, 945.0f, 0.0f), 0.0f, 1.0f);


	Tile16TJunctionController->setName("Tile 16 T Junction Controller");

	g_pRoot->addChild(Tile16TJunctionController->root());

	Tile16TJunctionController->addTrafficLight(tlFacarde14);
	Tile16TJunctionController->addTrafficLight(tlFacarde15);
	Tile16TJunctionController->addTrafficLight(tlFacarde16);
	Tile16TJunctionController->addTrafficLight(tlFacarde17);
#pragma endregion

#pragma region Tile 18 T Junction
	osg::Group* Tile18TJunction = new osg::Group();
	pTrafficLightGroup->addChild(Tile18TJunction);

	TrafficLightFacarde* tlFacarde18 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 18"),
			osg::Vec3(750.0f, 200.0f, 0.0f), FACE_FORWARD, 0.08f);

	TrafficLightFacarde* tlFacarde19 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 19"),
			osg::Vec3(1125.0f, 200.0f, 0.0f), FACE_RIGHT, 0.08f);

	TrafficLightFacarde* tlFacarde20 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 20"),
			osg::Vec3(750.0f, -175.0f, 0.0f), FACE_RIGHT, 0.08f);

	Tile18TJunction->addChild(tlFacarde18->root());
	Tile18TJunction->addChild(tlFacarde19->root());
	Tile18TJunction->addChild(tlFacarde20->root());

	TrafficLightControl* Tile18TJunctionController =
		new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadTJunction", "TJ18"),
			osg::Vec3(0.0f, 1890.0f, 0.0f), 90.0f, 1.0f);

	Tile18TJunctionController->setName("Tile 18 T Junction Controller");

	g_pRoot->addChild(Tile18TJunctionController->root());

	Tile18TJunctionController->addTrafficLight(tlFacarde18);
	Tile18TJunctionController->addTrafficLight(tlFacarde19);
	Tile18TJunctionController->addTrafficLight(tlFacarde20);
#pragma endregion 

#pragma region Tile 21 T Junction
	osg::Group* Tile21TJunction = new osg::Group();
	pTrafficLightGroup->addChild(Tile21TJunction);

	TrafficLightFacarde* tlFacarde21 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 21"),
			osg::Vec3(750.0f, 1700.0f, 0.0f), FACE_FORWARD, 0.08f);

	TrafficLightFacarde* tlFacarde22 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 22"),
			osg::Vec3(1125.0f, 1700.0f, 0.0f), FACE_RIGHT, 0.08f);

	TrafficLightFacarde* tlFacarde23 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 23"),
			osg::Vec3(750.0f, 2050, 0.0f), FACE_RIGHT, 0.08f);

	Tile21TJunction->addChild(tlFacarde21->root());
	Tile21TJunction->addChild(tlFacarde22->root());
	Tile21TJunction->addChild(tlFacarde23->root());

	TrafficLightControl* Tile21TJunctionController =
		new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadTJunction", "TJ21"),
			osg::Vec3(0.0f, 1890.0f, 0.0f), 90.0f, 1.0f);

	Tile21TJunctionController->setName("Tile 21 T Junction Controller");

	g_pRoot->addChild(Tile21TJunctionController->root());

	Tile21TJunctionController->addTrafficLight(tlFacarde21);
	Tile21TJunctionController->addTrafficLight(tlFacarde22);
	Tile21TJunctionController->addTrafficLight(tlFacarde23);
#pragma endregion 

#pragma region Tile 24 T Junction
	osg::Group* Tile24TJunction = new osg::Group();
	pTrafficLightGroup->addChild(Tile24TJunction);

	TrafficLightFacarde* tlFacarde24 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 24"),
			osg::Vec3(1700.0f, 750.0f, 0.0f), FACE_RIGHT, 0.08f);

	TrafficLightFacarde* tlFacarde25 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 25"),
			osg::Vec3(2050.0f, 750.0f, 0.0f), FACE_FORWARD, 0.08f);

	TrafficLightFacarde* tlFacarde26 =
		new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", "Traffic Light 26"),
			osg::Vec3(1700.0f, 1125.0f, 0.0f), FACE_UP, 0.08f);

	Tile24TJunction->addChild(tlFacarde24->root());
	Tile24TJunction->addChild(tlFacarde25->root());
	Tile24TJunction->addChild(tlFacarde26->root());

	TrafficLightControl* Tile24TJunctionController =
		new TrafficLightControl(raaAssetLibrary::getNamedAsset("roadTJunction", "TJ24"),
			osg::Vec3(-945.0f, 945.0f, 0.0f), 180.0f, 1.0f);

	Tile24TJunctionController->setName("Tile 24 T Junction Controller");

	g_pRoot->addChild(Tile24TJunctionController->root());

	Tile24TJunctionController->addTrafficLight(tlFacarde24);
	Tile24TJunctionController->addTrafficLight(tlFacarde25);
	Tile24TJunctionController->addTrafficLight(tlFacarde26);
#pragma endregion 
}

int main(int argc, char** argv)
{
	raaAssetLibrary::start();
	raaTrafficSystem::start();

	osgViewer::Viewer viewer;

	for (int i = 0; i < argc; i++)
	{
		if (std::string(argv[i]) == "-d") g_sDataPath = argv[++i];
	}

	// the root of the scene - use for rendering
	g_pRoot = new osg::Group();
	g_pRoot->ref();

	// build asset library - instances or clones of parts can be created from this
	raaAssetLibrary::loadAsset("roadStraight", g_sDataPath + "roadStraight.osgb");
	raaAssetLibrary::loadAsset("roadCurve", g_sDataPath + "roadCurve.osgb");
	raaAssetLibrary::loadAsset("roadTJunction", g_sDataPath + "roadTJunction.osgb");
	raaAssetLibrary::loadAsset("roadXJunction", g_sDataPath + "roadXJunction.osgb");
	raaAssetLibrary::loadAsset("trafficLight", g_sDataPath + "raaTrafficLight.osgb");
	raaAssetLibrary::insertAsset("vehicle", buildAnimatedVehicleAsset());

	// add a group node to the scene to hold the road sub-tree
	osg::Group* pRoadGroup = new osg::Group();
	pRoadGroup->setName("Roads Group");
	g_pRoot->addChild(pRoadGroup);

	// Traffic Lights
	osg::Group* pTrafficLightGroup = new osg::Group();
	pTrafficLightGroup->setName("Traffic Lights Group");
	g_pRoot->addChild(pTrafficLightGroup);
	createTrafficLights(pTrafficLightGroup);

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

	// Add car three
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

	// set the scene to render
	viewer.setSceneData(g_pRoot);

	viewer.realize();

	return viewer.run();
}


