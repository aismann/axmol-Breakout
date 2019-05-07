#include "GameoverScene.h"
#include "HelloWorldScene.h"

Scene* Gameover::createScene(bool win)
{
    return Gameover::create(win);
}

Gameover* Gameover::create(bool win)
{
	auto *ret = new (std::nothrow) Gameover();
	if (ret && ret->init(win))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool Gameover::init(bool win)
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();  
	auto label = Label::createWithTTF(win ? "YOU WON!" : "YOU LOSE :( ", "fonts/Marker Felt.ttf", 24);
	if (win) label->setColor(Color3B(0, 255, 255));
	label->setPosition(visibleSize / 2);
	this->addChild(label);
	this->runAction(Sequence::create(
		DelayTime::create(2.0f),
		CallFunc::create([]{
			Director::getInstance()->replaceScene(HelloWorld::createScene());
		})
		, NULL));
    return true;
}

