#ifndef _MonsterSprite_H_
#define _MonsterSprite_H_

#include"HeroSprite.h"

using namespace cocos2d;

class MonsterSprite :public HeroSprite
{
public:
	MonsterSprite();
	~MonsterSprite();

public:
	Action* runAction(Action* action);
};



#endif // !_MonsterSprite_H_
