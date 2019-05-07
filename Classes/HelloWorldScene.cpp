
/****************************************************************************
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "HelloWorldScene.h"
#define DRAG_BODYS_TAG 0x80


Scene* HelloWorld::createScene()
{
	auto scene = Scene::createWithPhysics();
	auto layer = HelloWorld::create();
	scene->addChild(layer);
	auto world = scene->getPhysicsWorld();
	auto gravity = Vec2(0.0f,0.0f);
	world->setGravity(gravity);
	// world->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}

	visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	auto closeItem = MenuItemImage::create(
		"CloseNormal.png",
		"CloseSelected.png",
		CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

	if (closeItem == nullptr ||
		closeItem->getContentSize().width <= 0 ||
		closeItem->getContentSize().height <= 0)
	{
		problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
	}
	else
	{
		float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
		float y = origin.y + closeItem->getContentSize().height / 2;
		closeItem->setPosition(Vec2(x, y));
	}

	// create menu, it's an autorelease object
	auto menu = Menu::create(closeItem, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

	/////////////////////////////
	// 3. add your codes below...

	// edge box (walls)
	auto ebNode = Node::create();
	auto ebBody = PhysicsBody::createEdgeBox(visibleSize);
	ebNode->setPhysicsBody(ebBody);
	ebNode->setPosition(visibleSize / 2);
	this->addChild(ebNode);

	// touch listener
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(HelloWorld::onTouchCancelled, this);
	getEventDispatcher()->addEventListenerWithFixedPriority(touchListener, 11);

	return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
	//Close the cocos2d-x game scene and quit the application
	Director::getInstance()->end();

	/*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

	//EventCustom customEndEvent("game_scene_close_event");
	//_eventDispatcher->dispatchEvent(&customEndEvent);
}

void HelloWorld::onEnter()
{
	Scene::onEnter();

	_physicsWorld = Director::getInstance()->getRunningScene()->getPhysicsWorld();

	// ball
	auto ball = Sprite::create("ball.png");
	ball->setPosition(Vec2(300,100));
	float radius = ball->getContentSize().width / 2;
	auto ballBody = PhysicsBody::createCircle(radius,
		PhysicsMaterial(0.0f,1.0f,0.0f));
	ballBody->setDynamic(true);
	ball->setPhysicsBody(ballBody);

	// paddle
	auto paddle = Sprite::create("paddle.png");
	paddle->setPosition(Vec2(visibleSize.width / 2, 50));
	auto paddleBody = PhysicsBody::createBox(paddle->getContentSize(),
		PhysicsMaterial(10.0f, 0.1f, 0.4f));
	paddleBody->setDynamic(true);
	paddle->setPhysicsBody(paddleBody);
	paddleBody->setTag(DRAG_BODYS_TAG);
	paddleBody->setRotationEnable(false);
	
	// Restrict paddle along x axes
	auto ground = Sprite::create();
	ground->setPosition(0, 0);
	auto groundBody = PhysicsBody::create();
	groundBody->setDynamic(false);
	ground->setPhysicsBody(groundBody);
	auto joint = PhysicsJointGroove::construct(
		paddle->getPhysicsBody(),
		ground->getPhysicsBody(),
		Vec2(-visibleSize.width/2, 0),
		Vec2(visibleSize.width/2, 0),
		Vec2(visibleSize.width/2, paddle->getPositionY())
		);
	_physicsWorld->addJoint(joint);

	Vec2 force = Vec2(300, 300);
	ballBody->applyImpulse(force);
	
	this->addChild(ground);
	this->addChild(ball);
	this->addChild(paddle);

	// Adding blcoks
	for (size_t i = 0; i < 4; i++)
	{
		static int padding = 20;
		auto block = Sprite::create("block.png");
		int xOffset = padding +
			block->getContentSize().width / 2 + 
			(block->getContentSize().width + padding)*i;
		block->setPosition(xOffset, 250);
		block->setTag(2);
		auto blockBody = PhysicsBody::createBox(block->getContentSize(),
			PhysicsMaterial(10.0f, 0.1, 0.4f));
		block->setPhysicsBody(blockBody);
		this->addChild(block);
	}
}

bool HelloWorld::onTouchBegan(Touch* touch, Event* event)
{
	auto location = touch->getLocation();
	auto arr = _physicsWorld->getShapes(location);

	PhysicsBody* body = nullptr;
	for (auto& obj : arr)
	{
		if ((obj->getBody()->getTag() & DRAG_BODYS_TAG) != 0)
		{
			body = obj->getBody();
			break;
		}
	}

	if (body != nullptr)
	{
		Node* mouse = Node::create();
		auto physicsBody = PhysicsBody::create(PHYSICS_INFINITY, PHYSICS_INFINITY);
		physicsBody->setDynamic(false);
		mouse->addComponent(physicsBody);
		mouse->setPosition(location);
		this->addChild(mouse);
		PhysicsJointPin* joint = PhysicsJointPin::construct(physicsBody, body, location);
		joint->setMaxForce(5000.0f * body->getMass());
		_physicsWorld->addJoint(joint);
		_mouses.insert(std::make_pair(touch->getID(), mouse));
		return true;
	}

	return false;
}

void HelloWorld::onTouchMoved(Touch* touch, Event* event)
{
	auto it = _mouses.find(touch->getID());

	if (it != _mouses.end())
	{
		it->second->setPosition(touch->getLocation());
	}
}

void HelloWorld::onTouchEnded(Touch* touch, Event* event)
{
	auto it = _mouses.find(touch->getID());

	if (it != _mouses.end())
	{
		this->removeChild(it->second);
		_mouses.erase(it);
	}
}

void HelloWorld::onTouchCancelled(Touch* touch, Event* event)
{
	auto it = _mouses.find(touch->getID());

	if (it != _mouses.end())
	{
		this->removeChild(it->second);
		_mouses.erase(it);
	}
}

