#include"MainScene.h"
#include"FirstScene.h"
#include"MainStep2Scene.h"


Scene* MainScene::CreateScene(){
	auto scene = Scene::createWithPhysics();

	scene->getPhysicsWorld()->setGravity(Vect(0, -980));
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

	auto layer = MainScene::create();
	scene->addChild(layer);
	
	return scene;
}

bool MainScene::init(){
	if (!Layer::create()){
		return false;
	}

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("AS_Zombie.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("B_Figure.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Pictures.plist");


	Director* dir = Director::getInstance();
	Size visibleSize = dir->getVisibleSize();
	
	/*¼ÓÔØµØÍ¼*/
	this->map = MapScene::create();

	//this->map->setPosition(10, 10);

	this->map->step = 1;
	this->addChild(map, 1);
	
	log("map.position = (%f , %f)", map->getPositionX(), map->getPositionY());


	/*´´½¨ÍË³ö°´Å¥*/
	auto exitButton = MenuItemSprite::create(
		Sprite::createWithSpriteFrameName("exit.png"),
		Sprite::createWithSpriteFrameName("exitOn.png"),
		CC_CALLBACK_1(MainScene::menuExitCallBack, this));
	exitButton->setScale(0.4);
	
	auto menu = Menu::create(exitButton, NULL);
	menu->setPosition(visibleSize.width*0.9, 32);
	this->addChild(menu, 2);

	/*¼ÓÔØ¼ýÍ·*/
	this->arrow = ArrowSpriteLayer::create();
	this->arrow->step = 1;
	this->arrow->layer1 = this;
	arrow->setArrowPosition(this->map->getObjectGroup());
	this->addChild(arrow,2);
	
	/*¼ÓÔØ¹­*/
	arch = Sprite::createWithSpriteFrameName("arch.png");
	ValueMap archPointMap = this->map->getObjectGroup()->getObject("Heros");
	float archX = archPointMap.at("x").asFloat();
	float archY = archPointMap.at("y").asFloat();
	arch->setPosition(archX + 20 , archY + 30);
	arch->setScale(0.7f);
	this->addChild(arch, 1);

	/*¼ÓÔØ¹ÖÎï*/
	this-> monster = MonsterSprite::create();
	monster->setMonstersPosition(this->map->getObjectGroup());
	this->addChild(monster, 2);
	
	/*´´½¨Ó¢ÐÛ*/
	myHero = Sprite::createWithSpriteFrameName("B_huolong.png");
	ValueMap heroPointMap = this->map->getObjectGroup()->getObject("Heros");
	float heroX = heroPointMap.at("x").asFloat();
	float heroY = heroPointMap.at("y").asFloat();
	myHero->setPosition(heroX + 25-64, heroY + 50);
	myHero->setScale(0.1f);
	this->addChild(myHero, 1);
	
	/*´´½¨»ðÑæÐ§¹û*/
	ValueMap firePointMap = this->map->getObjectGroup()->getObject("Fire");
	float fireX = firePointMap.at("x").asFloat();
	float fireY = firePointMap.at("y").asFloat();
	ParticleSystemQuad* fire = ParticleSystemQuad::create("fire1.plist");
	fire->retain();
	ParticleBatchNode* batch = ParticleBatchNode::createWithTexture(fire->getTexture());
	batch->addChild(fire);
	fire->setPosition(fireX,fireY);
	fire->release();
	this->addChild(batch, 10);

	ValueMap burningfirePointMap = this->map->getObjectGroup()->getObject("BurningFire");
	float burningfireX = burningfirePointMap.at("x").asFloat();
	float burningfireY = burningfirePointMap.at("y").asFloat();
	ParticleSystemQuad* burningfire = ParticleSystemQuad::create("burning.plist");
	burningfire->retain();
	this-> burningbatch = ParticleBatchNode::createWithTexture(burningfire->getTexture());
	burningbatch->addChild(burningfire);
	burningfire->setPosition(burningfireX, burningfireY);
	burningfire->release();
	burningbatch->setVisible(false);
	this->addChild(burningbatch, 10);

	/*Åö×²¼àÌýÆ÷*/
	auto contactListener = EventListenerPhysicsContact::create();
	
	contactListener->onContactBegin = [=](PhysicsContact &contact){
		auto nodeA = contact.getShapeA()->getBody()->getNode();
		auto nodeB = contact.getShapeB()->getBody()->getNode();
		if (nodeA&&nodeB){
			if (nodeA->getTag() == 10){
				//nodeB->getPhysicsBody()->removeFromWorld();
				//nodeB->removeFromParent();
				nodeB->getPhysicsBody()->removeFromWorld();
				nodeB->removeFromParentAndCleanup(true);
			}
			else if (nodeB->getTag() == 10){
				//nodeA->getPhysicsBody()->removeFromWorld();
				//nodeA->removeFromParent();
				//nodeA->getPhysicsBody()->removeFromWorld();
				//nodeA->removeFromParentAndCleanup(true);
				nodeA->getPhysicsBody()->removeFromWorld();
				nodeA->setVisible(false);
			}
			monster->monsterNumberDecrease();
		}


		return true;
	};

	contactListener->onContactSeparate = [=](PhysicsContact &contact) {

		this->arrow->getArrowSprite()->getPhysicsBody()->removeFromWorld();
		
		this->arrow->getArrowSprite()->setVisible(false);
		if (monster->getMonsterNumber() > 0) {
			this->arrow->changeArrowSpriteReferTo();
		}	
	};

	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
	
	/*¼üÅÌ¼àÌýÆ÷*/
	auto listenerKeypad = EventListenerKeyboard::create();
	listenerKeypad->onKeyPressed = [=](EventKeyboard::KeyCode keyCode,Event* event){
		/*Èç¹û°´ESC¼ü´´½¨ÔÝÍ£²ã*/
		if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE){
			if (this->flagPressed == false){
				this->Pause();
				this->flagPressed = true;
			}
			
		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listenerKeypad, this);
	
	this->scheduleUpdate();

	this->schedule(schedule_selector(MainScene::judge), 3.0f, kRepeatForever,0.0f);
	this->schedule(schedule_selector(MainScene::setBurning), 10.0f, kRepeatForever,0.0f);
	return true;
}

void MainScene::update(float dt){
	this->scores += 0.1;
	/*ÅÐ¶Ï¼ýÊÇ·ñÅö×²µ½ÁËÕÏ°­*/
	Sprite* arrowSprite = this->arrow->getArrowSprite();
	Point arrowPoint = arrowSprite->getPosition();


	Size visibleSize = Director::getInstance()->getVisibleSize();
	/*·É³öÆÁÄ»ÉÏ·½ÔòºöÂÔ*/
	if (arrowPoint.y < visibleSize.height&&arrowPoint.x < visibleSize.width){
		/*½«×ø±ê×ª»¯ÎªÍßÆ¬µØÍ¼ÀïµÄ×ø±ê*/
		Size arrowSize = this->arrow->getArrowSprite()->getContentSize();
		//Size mapTiledNum = this->map->getMap()->getMapSize();
		Size tiledSize = this->map->getMap()->getTileSize();
		if (arrowPoint.x + arrowSize.width / 2 < visibleSize.width){
			int tiledPosX = (arrowPoint.x + arrowSize.width /2) / tiledSize.width;
			int tiledPosY = (639 - arrowPoint.y) / tiledSize.height;

			//log("arrwPoint.y = %f", arrowPoint.y);

			Point tiledPos = Point(tiledPosX, tiledPosY);

			TMXLayer* meta = this->map->getMap()->getLayer("obscatle");
			int tiledGid = meta->getTileGIDAt(tiledPos);

			if (tiledGid != 0){
				arrowSprite->getPhysicsBody()->removeFromWorld();
				//arrowSprite->removeFromParent();//removeFromPhysicsWorld();
				arrowSprite->stopAllActions();
				this->arrow->changeArrowSpriteReferTo();
			}
		}
	}
	if (this->flagBurning == true){
		float arrowX = arrowPoint.x;
		float arrowY = arrowPoint.y;
		float burningX = this->burningbatch->getPosition().x;
		float burningY = this->burningbatch->getPosition().y;
		if ((arrowX+arrowSprite->getContentSize().width/2)>=8*32&&(arrowX-arrowSprite->getContentSize().width/2)<=11*32){
			//arrowSprite->getPhysicsBody()->removeFromWorld();
			    arrowSprite->removeFromParent();//removeFromPhysicsWorld();
				arrowSprite->stopAllActions();
				arrowSprite->setVisible(false);
				this->arrow->changeArrowSpriteReferTo();
		}
	}
}

void MainScene::judge(float dt){
	Size visibleSize = Director::getInstance()->getVisibleSize();
	if (this->arrow->getArrowSpriteNumber() >= 0 && this->monster->getMonsterNumber() == 0){		
		float score = MAX_SCORES+4 * this->arrow->getArrowSpriteNumber() - 8 * this->monster->getMonsterNumber()-0.01*this->scores;
		this->arrow->step = 2;
		this->map->step = 2;
		this->monster->step = 2;
		auto step2 = MainStep2Scene::create();
		step2->Scores = 0.2*score;
		Director::getInstance()->replaceScene(
			TransitionSplitRows::create(3.0f, MainStep2Scene::CreateScene()));
	}
	if (this->arrow->getArrowSpriteNumber() <= 0 && this->monster->getMonsterNumber() > 0){
		float score = MAX_SCORES-20 * this->monster->getMonsterNumber()-0.1*this->scores;
		int scorefinal = (int)(0.2*score);
		char finalscore[20];
		sprintf(finalscore, "%d", scorefinal);
		auto scorelabel = LabelTTF::create(finalscore, "Brush Script MT", 32);
		scorelabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 100);
		MessageBox("There are no arrows left", "Poi!");
		Director::getInstance()->pause();
		this->faillayer = FailLayer::create();
		this->faillayer->mainPlayLayer = this;
		this->addChild(faillayer, 20);
		this->addChild(scorelabel, 21);
	}
}

void MainScene::setBurning(float dt){
	this->burningbatch->setVisible(true);
	this->flagBurning = true;
	this->scheduleOnce(schedule_selector(MainScene::deleteBurning), 3.0f);
}

void MainScene::deleteBurning(float dt){
	this->burningbatch->setVisible(false);
	this->flagBurning = false;
}

void MainScene::Pause(){
	Director::getInstance()->pause();
	if (this->arrow->isflying == true){
		speed = this->arrow->getArrowSprite()->getPhysicsBody()->getVelocity();
		this->arrow->getArrowSprite()->getPhysicsBody()->setVelocity(Vec2::ZERO);
		this->arrow->getArrowSprite()->getPhysicsBody()->setGravityEnable(FALSE);
	}
	//this->pauseSchedulerAndActions();
	this->pauselayer = PauseLayer::create();
	this->pauselayer->mainPlayLayer = this;
	this->pauselayer->mainStep2Layer = NULL;
	this->pauselayer->mainStep3Layer = NULL;
	this->addChild(pauselayer, 20);
}

Vec2 MainScene::getSpeed(){
	return this->speed;
}

void MainScene::menuExitCallBack(Ref* pSender){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.", "Alert");
	return;
#endif

	Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif

}