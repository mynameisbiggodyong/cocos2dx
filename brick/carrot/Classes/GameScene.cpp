#include "GameScene.h"
GameScene::GameScene()
{
}
GameScene::GameScene(std::string name)
{
	this->setSceneName(name);
}
GameScene* GameScene::create(std::string name)
{
	auto gameScene = new GameScene(name);
	if (gameScene && gameScene->init())
	{
		gameScene->autorelease();
		return gameScene;
	}
	delete gameScene;
	return nullptr;
}

Scene* GameScene::createScene(std::string name)
{
	auto scene = Scene::create();
	auto layer = create(name);
	scene->addChild(layer);
	return scene;
}

bool GameScene::init()
{
	if (!Layer::init()) {
		return false;
	}
	visible = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();
	tmx = TMXTiledMap::create(sceneName);
	auto bg = tmx->getLayer("background");
	tmx->setAnchorPoint(Vec2(0.5, 0.5));
	tmx->setPosition(origin.x + visible.width / 2, origin.y + visible.height / 2);
	offX = (visible.width - tmx->getContentSize().width) / 2;
	addChild(tmx);

	int val = 1;
	auto obj = tmx->getObjectGroup("points");
	auto point = obj->getObject(std::to_string(val));
	while (point.begin() != point.end())
	{
		auto x = point.at("x").asFloat();
		auto y = point.at("y").asFloat();
		auto node = Node::create();
		node->setPosition(x, y);
		positions.pushBack(node);
		val++;
		point = obj->getObject(std::to_string(val));
	}
	initResource(NULL);

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("element.plist", "element.png");
	monsterInterval = 2;
	flashCount = 0;

	//绑定触摸函数
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	//生成炮台列表
	float layerWidth = 0, layerHeight = 0, marginRate = 1.2;

	towerAvailiable = LayerColor::create(Color4B(178, 95, 27, 120));
	//towerAvailiable->setAnchorPoint(Vec2(0.5, 0.5));

	auto bottleTower = BottleTower::create();
	bottleTower->setPosition(layerWidth  + bottleTower->getContentSize().width * marginRate / 2, 0);
	towerAvailiable->addChild(bottleTower);
	layerWidth += bottleTower->getContentSize().width * marginRate;
	layerHeight = layerHeight > bottleTower->getContentSize().height * marginRate 
		? layerHeight : bottleTower->getContentSize().height * marginRate;

	auto fanBottle = FanTower::create();
	fanBottle->setPosition(layerWidth + fanBottle->getContentSize().width * marginRate / 2, 0);
	towerAvailiable->addChild(fanBottle);
	layerWidth += fanBottle->getContentSize().width * marginRate;
	layerHeight = layerHeight > fanBottle->getContentSize().height * marginRate
		? layerHeight : fanBottle->getContentSize().height * marginRate;

	bottleTower->setPositionY(layerHeight / 2);
	fanBottle->setPositionY(layerHeight / 2);

	towerAvailiable->setContentSize(Size(layerWidth, layerHeight));
	towerAvailiable->setVisible(false);
	towerAvailiable->ignoreAnchorPointForPosition(false);
	tmx->addChild(towerAvailiable);

	//创建重新开始按钮
	auto label = Label::create("Restart", "fonts/Marker Felt.ttf", 30);
	auto menuItem = MenuItemLabel::create(label, CC_CALLBACK_1(GameScene::initResource, this));
	auto menu = Menu::create(menuItem, NULL);
	menu->setContentSize(menuItem->getContentSize());
	menu->setAnchorPoint(Vec2(0, 1));
	menu->setPosition(tmx->getContentSize().width / 2, tmx->getContentSize().height - menu->getContentSize().height);
	menu->alignItemsVertically();
	tmx->addChild(menu);
	gameManager = GameManager::getInstance();
	scheduleUpdate();
	log("Visible x:%f y:%f", visible.width, visible.height);
	log("Win x:%f y:%f", Director::getInstance()->getWinSize().width, Director::getInstance()->getWinSize().height);
	log("Frame x:%f y:%f", Director::getInstance()->getOpenGLView()->getFrameSize().width, Director::getInstance()->getOpenGLView()->getFrameSize().height);
	log("Origin x:%f y:%f", origin.x, origin.y);
	auto sprite = Sprite::create("Fan2.png");
	log("Size1 x:%f y:%f", sprite->getContentSize().width, sprite->getContentSize().height);
	sprite = Sprite::create("Fan2.png", Rect(0, 0, 20, 10));
	log("Size2 x:%f y:%f", sprite->getContentSize().width, sprite->getContentSize().height);
}

void GameScene::initResource(Ref *ref)
{

	for (auto array :mapFlag)
	{
		array.clear();
	}
	mapFlag.clear();

	Size mapSize = tmx->getMapSize();
	for (int i = 0; i < mapSize.width; i++)
	{
		std::vector<int> tmp;
		for (int j = 0; j < mapSize.height; j++)
		{
			int gid = tmx->getLayer("background")->getTileGIDAt(Point(i, j));
			auto map = tmx->getPropertiesForGID(gid).asValueMap();
			if (map.at("canTouch").asInt() == 1)
			{
				tmp.push_back(1);
			}
			else {
				tmp.push_back(0);
			}
		}
		mapFlag.push_back(tmp);
	}
	auto monsters = GameManager::getInstance()->monsters;
	for (int i = 0; i < monsters.size(); i++)
	{
		monsters.at(i)->stopAllActions();
		monsters.at(i)->removeFromParentAndCleanup(true);
	}
	monsters.clear();
	for (int i = 0; i < towers.size(); i++)
	{
		towers.at(i)->stopAllActions();
		towers.at(i)->destroyTower();
		towers.at(i)->removeFromParentAndCleanup(true);
	}
	towers.clear();
}
void GameScene::update(float dt)
{
	//判断是否要新加怪物
	flashCount++;
	if (Director::getInstance()->getAnimationInterval() * flashCount >= monsterInterval)
	{
		flashCount = 0;
		auto monster = Monster::create("monster.png", 200, 80, positions);
		gameManager->monsters.pushBack(monster);
		monster->forwad();
		tmx->addChild(monster, 1);
	}

	for (int j = 0; j < towers.size(); j++)
	{
		//旋转炮台
		auto tower = towers.at(j);
		auto degree = tower->getRotation();
			degree = degree + 2 > 180 ? degree - 360 : degree + 2;
			//tower->setRotation(degree);
	}

	//检查怪兽是否出界或是死亡
	for (int i = 0; i < gameManager->monsters.size(); i++)
	{
		auto monster = (gameManager->monsters).at(i);
		if (monster->getCurrentIndex() == positions.size() || monster->getOpacity() <= 1)
		{
			gameManager->monsters.erase(i);
			i--;
			monster->stopAllActions();
			monster->removeFromParentAndCleanup(true);
			continue;
		}
		if (monster->getBlood() <= 0)
		{
			monster->runAction(Sequence::create(FadeOut::create(0.3f), NULL));
		}
	}

}

bool GameScene::onTouchBegan(Touch *touch, Event *unused_event)
{
	Point coorPoint = touch->getLocation();
	coorPoint.x = coorPoint.x - offX;
	if (coorPoint.x <= 0 || coorPoint.x >= tmx->getContentSize().width)
	{
		//log("point:%f width:%f", coorPoint.x, tmx->getContentSize().width);
		return false;
	}
	Point tiledPos = openglToTiledCoord(coorPoint);
	//log("x:%f y:%f", tiledPos.x, tiledPos.y);
	if (towerAvailiable->isVisible())
	{
		coorPoint.x = coorPoint.x + offX;
		tryAddTower(coorPoint, tiledPos);
		towerAvailiable->setVisible(false);
		return false;
	}

	if (tiledPos.x >= 0 && tiledPos.x < mapFlag.size() 
		&& tiledPos.y >= 0 && tiledPos.y < mapFlag[0].size() 
		&& mapFlag[tiledPos.x][tiledPos.y] == 1)
	{
		//默认在触摸点的上方显示
		towerAvailiable->setPosition(coorPoint);
		towerAvailiable->setVisible(true);
		prePoint = tiledPos;
	}
	return true;
}

void GameScene::onTouchMoved(Touch *touch, Event *unused_event)
{
}

void GameScene::onTouchEnded(Touch *touch, Event *unused_event)
{
}

Point GameScene::openglToTiledCoord(Point position)
{
	Size mapSize = tmx->getMapSize();
	Size tiledSize = tmx->getTileSize();
	float scaleFactorX = (visible.width - 2 * offX) / mapSize.width / tiledSize.width;
	float scaleFactorY = visible.height / mapSize.height / tiledSize.height;
	//log("Touch x:%f, y:%f", position.x, position.y);
	//log("map x:%f y:%f", mapSize.width, mapSize.height);
	//log("tile x:%f y:%f", tiledSize.width, tiledSize.height);
	//log("scale x:%f y:%f", scaleFactorX, scaleFactorY);

	int x = position.x / scaleFactorX / tiledSize.width;
	int y = (mapSize.height * tiledSize.height - position.y / scaleFactorY) / tiledSize.height;

	//log("x:%d y:%d", x, y);
	return Point(x, y);
}

/************************************************************************/
/* 根据不同的触摸位置增加对应的炮台                                     */
/************************************************************************/
void GameScene::tryAddTower(Point coorPoint, Point tiledPoint)
{
	if (!towerAvailiable->isVisible())
	{
		return;
	}
	log("touch1 x:%f y:%f", coorPoint.x, coorPoint.y);
	Point localPoint = towerAvailiable->convertToNodeSpace(coorPoint);
	log("touch2 x:%f y:%f", localPoint.x, localPoint.y);
	for (Node *node : towerAvailiable->getChildren())
	{
		AbstractTower* tower = (AbstractTower*)node;
		//Rect rect = tower->getBoundingBox();
		Rect rect(0, 0, tower->getContentSize().width, tower->getContentSize().height);
		localPoint = tower->convertToNodeSpace(coorPoint);
		//log("minX:%f minY:%f maxX:%f maxY:%f", rect.getMinX(), rect.getMinY(), rect.getMaxX(), rect.getMaxY());
		//if (tower->getBoundingBox().containsPoint(localPoint))
		if (rect.containsPoint(localPoint))
		{
			auto newTower = AbstractTower::createTowerByString(tower->getTowerName());
			Point location = tiledToOpenglCoord(prePoint);
			log("local x:%f y:%f", location.x, location.y);
			newTower->setPosition(location);
			newTower->origin = Point(-location.x, -location.y);
			newTower->visible = tmx->getContentSize();
			tmx->addChild(newTower);
			mapFlag[prePoint.x][prePoint.y] = 0;
			towers.pushBack(newTower);
			newTower->scheduleUpdate();
			break;
		}
	}
}
Point GameScene::tiledToOpenglCoord(Point position)
{
	Size mapSize = tmx->getMapSize();
	Size tiledSize = tmx->getTileSize();
	float scaleFactorX = (visible.width - 2 * offX) / mapSize.width / tiledSize.width;
	float scaleFactorY = visible.height / mapSize.height / tiledSize.height;
	auto x = (position.x * tiledSize.width + tiledSize.width / 2) * scaleFactorX;
	auto y = (mapSize.height * tiledSize.height - (tiledSize.height * position.y + tiledSize.height / 2)) * scaleFactorY;
	return Point(x, y);
}