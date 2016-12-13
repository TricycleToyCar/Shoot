#include"MainStep3Scene.h"
#include"ArrowSpriteStep3Layer.h"
#include"MonsterSpriteStep3Layer.h"

float MainStep3Scene::Scores = 0;

Scene* MainStep3Scene::CreateScene(){
	auto scene = Scene::createWithPhysics();

	scene->getPhysicsWorld()->setGravity(Vect(0, -980));
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

	auto layer = MainStep3Scene::create();
	scene->addChild(layer);

	return scene;
}

bool MainStep3Scene::init(){
	if (!Layer::create()){
		return false;
	}

	_monsterNumber = MONSTER_NUM;
	_arrowNumber = STEP_THREE_ARROW;

	Size visibleSize = Director::getInstance()->getVisibleSize();

	/*加载地图*/
	this->_mapLayer = MapLayer::create();
	this->addChild(_mapLayer, 1);


	/*创建退出按钮*/
	auto exitButton = MenuItemSprite::create(
		Sprite::createWithSpriteFrameName("exit.png"),
		Sprite::createWithSpriteFrameName("exitOn.png"),
		CC_CALLBACK_1(MainStep3Scene::menuExitCallBack, this));
	exitButton->setScale(0.4);

	auto menu = Menu::create(exitButton, NULL);
	menu->setPosition(visibleSize.width*0.9, 32);
	this->addChild(menu, 2);

	/*加载箭头*/
	this->_arrowLayer = ArrowSpriteStep3Layer::create();
	this->_arrowLayer->addObserver(this);
	_arrowLayer->setArrowPosition(this->_mapLayer->getObjectGroup());
	this->addChild(_arrowLayer, 2);

	/*加载弓*/
	arch = Sprite::createWithSpriteFrameName("arch.png");
	ValueMap archPointMap = this->_mapLayer->getObjectGroup()->getObject("Heros");
	float archX = archPointMap.at("x").asFloat();
	float archY = archPointMap.at("y").asFloat();
	arch->setPosition(archX + 20, archY + 30);
	arch->setScale(0.7f);
	this->addChild(arch, 1);

	/*加载怪物*/
	this->_monsterLayer = MonsterSpriteStep3Layer::create();
	this->_monsterLayer->addObserver(this);
	_monsterLayer->setMonstersPosition(this->_mapLayer->getObjectGroup());
	this->addChild(_monsterLayer, 2);

	/*创建英雄*/
	myHero = Sprite::createWithSpriteFrameName("B_sensen.png");
	ValueMap heroPointMap = this->_mapLayer->getObjectGroup()->getObject("Heros");
	float heroX = heroPointMap.at("x").asFloat();
	float heroY = heroPointMap.at("y").asFloat();
	myHero->setPosition(heroX + 25-64, heroY + 50);
	myHero->setScale(0.1f);
	this->addChild(myHero, 1);

	/*创建火焰效果*/

	ValueMap burningfirePointMap = this->_mapLayer->getObjectGroup()->getObject("BurningFire");
	float burningfireX = burningfirePointMap.at("x").asFloat();
	float burningfireY = burningfirePointMap.at("y").asFloat();
	ParticleSystemQuad* burningfire = ParticleSystemQuad::create("burning.plist");
	burningfire->retain();
	this->burningbatch = ParticleBatchNode::createWithTexture(burningfire->getTexture());
	burningbatch->addChild(burningfire);
	burningfire->setPosition(burningfireX, burningfireY);
	burningfire->release();
	burningbatch->setVisible(false);
	this->addChild(burningbatch, 10);

	/*碰撞监听器*/
	_contactListener = EventListenerPhysicsContact::create();
	_contactListener->onContactBegin = [=](PhysicsContact &contact){
		auto nodeA = contact.getShapeA()->getBody()->getNode();
		auto nodeB = contact.getShapeB()->getBody()->getNode();
		if (nodeA&&nodeB){
			if (nodeA->getTag() == 10){

				nodeB->getPhysicsBody()->removeFromWorld();
				nodeB->removeFromParentAndCleanup(true);
			}
			else if (nodeB->getTag() == 10){
				nodeA->getPhysicsBody()->removeFromWorld();
				nodeA->removeFromParentAndCleanup(true);
			}
			//_monsterLayer->monsterNumberDecrease();
			_monsterLayer->onContact();
		}

		//arrow->getArrowSprite()->getPhysicsBody()->removeFromWorld();

		//arrow->getArrowSprite()->removeFromParent();//removeFromPhysicsWorld();
		//arrow->getArrowSprite()->setVisible(false);
		//if (monster->getMonsterNumber() > 0){
		//	arrow->changeArrowSpriteReferTo();
		//}
		return true;
	};

	_contactListener->onContactSeparate = [=](PhysicsContact &contact) {
		this->_arrowLayer->getArrowSprite()->getPhysicsBody()->removeFromWorld();

		this->_arrowLayer->getArrowSprite()->setVisible(false);
		if (_monsterLayer->getMonsterNumber() > 0) {
			this->_arrowLayer->changeArrowSpriteReferTo();
			this->_arrowLayer->onContact();
			this->_arrowLayer->updateLabel();
		}
	};

	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_contactListener, this);

	/*键盘监听器*/
	auto listenerKeypad = EventListenerKeyboard::create();
	listenerKeypad->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event){
		/*如果按ESC键创建暂停层*/
		if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE){
			if (this->_flagPressed == false){
				this->Pause();
				this->_flagPressed = true;
			}

		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listenerKeypad, this);

	this->scheduleUpdate();

	//this->schedule(schedule_selector(MainStep3Scene::judge), 3.0f, kRepeatForever, 0.0f);
	this->schedule(schedule_selector(MainStep3Scene::setBurning), 6.0f, kRepeatForever, 0.0f);

	return true;

}

void MainStep3Scene::update(float dt){
	this->scores += 0.1;
	/*判断箭是否碰撞到了障碍*/
	Sprite* arrowSprite = this->_arrowLayer->getArrowSprite();
	if (arrowSprite)
	{
		Point arrowPoint = arrowSprite->getPosition();
		Size visibleSize = Director::getInstance()->getVisibleSize();
		/*飞出屏幕上方则忽略*/
		if (arrowPoint.y < visibleSize.height) {
			/*将坐标转化为瓦片地图里的坐标*/
			Size arrowSize = this->_arrowLayer->getArrowSprite()->getContentSize();
			Size mapTiledNum = this->_mapLayer->getMap()->getMapSize();
			Size tiledSize = this->_mapLayer->getMap()->getTileSize();
			if (arrowPoint.x + arrowSize.width / 2 < visibleSize.width) {
				int tiledPosX = (arrowPoint.x + arrowSize.width / 2) / tiledSize.width;
				int tiledPosY = (640 - arrowPoint.y) / tiledSize.height;

				Point tiledPos = Point(tiledPosX, tiledPosY);

				TMXLayer* meta = this->_mapLayer->getMap()->getLayer("obscatle");
				int tiledGid = meta->getTileGIDAt(tiledPos);

				if (tiledGid != 0) {
					arrowSprite->getPhysicsBody()->removeFromWorld();
					//arrowSprite->removeFromParent();//removeFromPhysicsWorld();
					arrowSprite->stopAllActions();

					this->_arrowLayer->changeArrowSpriteReferTo();

					this->_arrowLayer->updateLabel();
				}
			}
		}
		if (this->_flagBurning == true) {
			float arrowX = arrowPoint.x;
			float arrowY = arrowPoint.y;
			float burningX = this->burningbatch->getPosition().x;
			float burningY = this->burningbatch->getPosition().y;
			if ((arrowX + arrowSprite->getContentSize().width / 2) >= 8 * 32 && (arrowX - arrowSprite->getContentSize().width / 2) <= 11 * 32) {
				//arrowSprite->getPhysicsBody()->removeFromWorld();
				arrowSprite->removeFromParent();//removeFromPhysicsWorld();
				//arrowSprite->stopAllActions();
				//arrowSprite->setVisible(false);

				this->_arrowLayer->changeArrowSpriteReferTo();

				this->_arrowLayer->updateLabel();
			}
		}
	}

}

void MainStep3Scene::judge(float dt){
	Size visibleSize = Director::getInstance()->getVisibleSize();
	if (this->_arrowLayer->getArrowSpriteNumber() >= 0 && this->_monsterLayer->getMonsterNumber() == 0){
		float score = MAX_SCORES + 4 * this->_arrowLayer->getArrowSpriteNumber() - 8 * this->_monsterLayer->getMonsterNumber() - 0.01*this->scores;
		int scorefinal =(int) (0.5*score + this->Scores);
		char finalscore[20];
		sprintf(finalscore, "%d", scorefinal);
		auto scorelabel = LabelTTF::create(finalscore, "Brush Script MT", 32);
		scorelabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 100);
		Director::getInstance()->pause();
		this->winlayer = WinLayer::create();
		this->addChild(winlayer, 20);
		this->addChild(scorelabel, 21);
	}
	if (this->_arrowLayer->getArrowSpriteNumber() <= 0 && this->_monsterLayer->getMonsterNumber() > 0){
		float score = MAX_SCORES - 20 * this->_monsterLayer->getMonsterNumber() - 0.1*this->scores;
		int scorefinal = (int)(0.5*score + this->Scores);
		char finalscore[20];
		sprintf(finalscore, "%d", scorefinal);
		auto scorelabel = LabelTTF::create(finalscore, "Brush Script MT", 32);
		scorelabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 100);
		MessageBox("There are no arrows left", "Poi!");
		Director::getInstance()->pause();
		this->faillayer = FailLayer::create();
		this->faillayer->mainStep3Layer = this;
		this->addChild(faillayer, 20);
		this->addChild(scorelabel, 21);
	}
}

void MainStep3Scene::setBurning(float dt)
{
	this->burningbatch->setVisible(true);
	this->_flagBurning = true;
	this->scheduleOnce(schedule_selector(MainStep3Scene::deleteBurning), 1.5f);
}

void MainStep3Scene::onEventHappen(Layer * object, MyEvent e)
{
	switch (e)
	{
	case NoArrow: {
		if (_monsterNumber > 0)
		{
			Size visibleSize = Director::getInstance()->getVisibleSize();
			float score = MAX_SCORES - 20 * this->_monsterLayer->getMonsterNumber() - 0.1*this->scores;
			int scorefinal = (int)(0.5*score + this->Scores);
			char finalscore[20];
			sprintf(finalscore, "%d", scorefinal);
			auto scorelabel = LabelTTF::create(finalscore, "Brush Script MT", 32);
			scorelabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 100);
			MessageBox("There are no arrows left", "Poi!");
			Director::getInstance()->pause();
			this->faillayer = FailLayer::create();
			this->faillayer->mainStep3Layer = this;
			this->addChild(faillayer, 20);
			this->addChild(scorelabel, 21);
			break;
		}
	}
	case NoMonster: {

		Director::getInstance()->pause();
		this->winlayer = WinLayer::create();
		this->addChild(winlayer, 20);

		break;

	}
	case ArrowRotate: {
		if (((ArrowSpriteLayer*)object)->getArrowSprite())
		{
			float angle = ((ArrowSpriteLayer*)object)->getArrowSprite()->getRotation();
			this->arch->setRotation(angle);
			break;
		}
	}
	case Contact: {
		if (MONSTER_LAYER_TAG == object->getTag())
		{
			this->_monsterNumber--;
		}
		else
		{
			this->_arrowNumber--;
		}
		break;
	}
	case ArrowOut: {
		this->_arrowNumber--;
		this->_arrowLayer->updateLabel();
		break;
	}
	default:
		break;
	}
}

void MainStep3Scene::deleteBurning(float dt)
{
	this->burningbatch->setVisible(false);
	this->_flagBurning = false;
}

Vec2 MainStep3Scene::getSpeed(){
	return this->speed;
}

void MainStep3Scene::Pause(){
	Director::getInstance()->pause();
	if (this->_arrowLayer->isflying == true){
		speed = this->_arrowLayer->getArrowSprite()->getPhysicsBody()->getVelocity();
		this->_arrowLayer->getArrowSprite()->getPhysicsBody()->setVelocity(Vec2::ZERO);
		this->_arrowLayer->getArrowSprite()->getPhysicsBody()->setGravityEnable(FALSE);
	}
	//this->pauseSchedulerAndActions();
	this->pauselayer = PauseLayer::create();
	this->pauselayer->mainStep3Layer = this;
	this->pauselayer->mainPlayLayer = NULL;
	this->pauselayer->mainStep2Layer = NULL;
	this->addChild(pauselayer, 20);
}


void MainStep3Scene::menuExitCallBack(Ref* pSender){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.", "Alert");
	return;
#endif

	Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif

}