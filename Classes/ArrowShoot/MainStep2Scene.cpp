#include"MainStep2Scene.h"
#include"MainStep3Scene.h"
#include"ArrowSpriteStep2Layer.h"

float MainStep2Scene::Scores = 0;

Scene* MainStep2Scene::CreateScene(){
	auto scene = Scene::createWithPhysics();

	scene->getPhysicsWorld()->setGravity(Vect(0, -980));
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

	auto layer = MainStep2Scene::create();
	scene->addChild(layer);

	return scene;
}

bool MainStep2Scene::init(){
	if (!Layer::create()){
		return false;
	}

	//Size visibleSize = Director::getInstance()->getVisibleSize();

	_arrowNumber = STEP_TWO_ARROW;
	_monsterNumber = MONSTER_NUM;

	_arrowNumber = STEP_TWO_ARROW;
	_monsterNumber = MONSTER_NUM;

	/*���ص�ͼ*/
	setMap(MapLayer::create(), 2);

	/*�����˳���ť*/

	setCommonPart();

	/*���ؼ�ͷ*/

	setArrowLayer(ArrowSpriteStep2Layer::create());


	/*���ع���*/
	setMonsterLayer(MonsterSpriteLayer::create(), 2);

	/*����Ӣ��*/
	setHero(Sprite::createWithSpriteFrameName("B_littlestar.png"));/*


	/*��ײ������*/

	setListener();
	

	/*���̼�����*/
	auto listenerKeypad = EventListenerKeyboard::create();
	listenerKeypad->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event){
		/*�����ESC��������ͣ��*/
		if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE){
			if (this->_flagPressed == false){
				this->Pause();
				this->_flagPressed = true;
			}

		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listenerKeypad, this);

	this->scheduleUpdate();

	//this->schedule(schedule_selector(MainStep2Scene::judge), 3.0f, kRepeatForever, 0.0f);


	return true;
}

void MainStep2Scene::onEventHappen(Layer * object, MyEvent e)
{
	switch (e)
	{
	case NoArrow: {
		if (_monsterNumber > 0)
		{
			Size visibleSize = Director::getInstance()->getVisibleSize();
			float score = MAX_SCORES - 20 * this->_monsterLayer->getMonsterNumber() - 0.1*this->scores;
			int scorefinal = (int)(0.3*score + this->Scores);
			char finalscore[20];
			sprintf(finalscore, "%d", scorefinal);
			auto scorelabel = LabelTTF::create(finalscore, "Brush Script MT", 32);
			scorelabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 100);
			MessageBox("There are no arrows left", "Poi!");
			Director::getInstance()->pause();
			this->faillayer = FailLayer::create();
			this->faillayer->mainStep2Layer = this;
			this->addChild(faillayer, 20);
			this->addChild(scorelabel, 21);
		}
		break;
	}
	case NoMonster: {

		//this->_arrowLayer->step = 3;
		this->_mapLayer->step = 3;
		this->_monsterLayer->step = 3;
		//auto step3 = MainStep3Scene::create();
		//step3->Scores = 0.3*score;
		Director::getInstance()->replaceScene(
			TransitionSplitRows::create(3.0f, MainStep3Scene::CreateScene()));

		break;

	}
	case ArrowRotate: {
		if (((ArrowSpriteLayer*)object)->getArrowSprite())
		{
			float angle = ((ArrowSpriteLayer*)object)->getArrowSprite()->getRotation();

			this->_arch->setRotation(angle);

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

void MainStep2Scene::update(float dt)
{
	this->scores += 0.1;
	/*�жϼ��Ƿ���ײ�����ϰ�*/
	Sprite* arrowSprite = this->_arrowLayer->getArrowSprite();
	if (arrowSprite)
	{

		Point arrowPoint = arrowSprite->getPosition();
		Size visibleSize = Director::getInstance()->getVisibleSize();
		/*�ɳ���Ļ�Ϸ������*/
		if (arrowPoint.y < visibleSize.height) {
			/*������ת��Ϊ��Ƭ��ͼ�������*/
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
	}

}

Vec2 MainStep2Scene::getSpeed(){
	return this->speed;
}


void MainStep2Scene::Pause(){
	Director::getInstance()->pause();
	if (this->_arrowLayer->isflying == true){
		speed = this->_arrowLayer->getArrowSprite()->getPhysicsBody()->getVelocity();
		this->_arrowLayer->getArrowSprite()->getPhysicsBody()->setVelocity(Vec2::ZERO);
		this->_arrowLayer->getArrowSprite()->getPhysicsBody()->setGravityEnable(FALSE);
	}
	//this->pauseSchedulerAndActions();
	this->pauselayer = PauseLayer::create();
	this->pauselayer->mainStep2Layer = this;
	this->pauselayer->mainStep3Layer = NULL;
	this->pauselayer->mainPlayLayer = NULL;
	this->addChild(pauselayer, 20);
}
