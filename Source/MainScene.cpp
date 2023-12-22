/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

 https://axmolengine.github.io/

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

#include "MainScene.h"
#include "GameOverScene.h"

#define DRAG_BODYS_TAG 0x80
#define BALL_BODY_TAG 1
#define BLOCK_BODYS_TAG 2
#define GROUND_BODY_TAG 3
//#define PADDLE_BODY_TAG 4

USING_NS_AX;

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf(
		"Depending on how you compiled you might have to add 'Content/' in front of filenames in "
		"MainScene.cpp\n");
}

Scene* MainScene::createScene()
{
	auto scene = Scene::createWithPhysics();
	auto layer = MainScene::create();
	scene->addChild(layer);
	auto world = scene->getPhysicsWorld();
	auto gravity = Vec2(0.0f, 0.0f);
	world->setGravity(gravity);
	return scene;
}

void MainScene::onEnter()
{
	Scene::onEnter();
	auto visibleSize = _director->getVisibleSize();
	_physicsWorld = Director::getInstance()->getRunningScene()->getPhysicsWorld();

	// ball
	auto ball = Sprite::create("ball.png");
	ball->setPosition(Vec2(300, 100));
	float radius = ball->getContentSize().width / 2;
	ballBody = PhysicsBody::createCircle(radius,
		PhysicsMaterial(1.0f, 1.0f, 0.0f));
	ballBody->setDynamic(true);
	ballBody->setContactTestBitmask(true);
	ball->setPhysicsBody(ballBody);
	//ball->addComponent(ballBody);
	ball->setTag(BALL_BODY_TAG);

	// paddle
	auto paddle = Sprite::create("paddle.png");
	paddle->setPosition(Vec2(visibleSize.width / 2, 50));
	auto paddleBody = PhysicsBody::createBox(paddle->getContentSize(),
		PhysicsMaterial(10.0f, 0.1f, 0.4f));
	paddleBody->setDynamic(true);
	paddle->setPhysicsBody(paddleBody);
	//paddle->addComponent(paddleBody);
	paddleBody->setTag(DRAG_BODYS_TAG);
	paddleBody->setRotationEnable(false);

	// Restrict paddle along x axes
	auto ground = Sprite::create();
	ground->setPosition(0, 0);
	auto groundBody = PhysicsBody::createEdgeSegment(Vec2(0, 0), Vec2(visibleSize.width, 0));
	groundBody->setDynamic(false);
	ground->setTag(GROUND_BODY_TAG);
	groundBody->setContactTestBitmask(true);
	ground->setPhysicsBody(groundBody);
	//ground->addComponent(groundBody);
	auto joint = PhysicsJointGroove::construct(
		paddle->getPhysicsBody(),
		ground->getPhysicsBody(),
		Vec2(-visibleSize.width / 2, 0),
		Vec2(visibleSize.width / 2, 0),
		Vec2(visibleSize.width / 2, paddle->getPositionY())
	);
//	_physicsWorld->addJoint(joint);
	getPhysicsWorld()->addJoint(joint);

	Vec2 force = Vec2(300, 300);
	ballBody->applyImpulse(force);

	this->addChild(ground);
	this->addChild(ball);
	this->addChild(paddle);


	// Adding blocks
	for (size_t n = 0; n < 5; n++)
	{
		for (size_t i = 0; i < 9; i++)
		{
			static int padding = 20;
			auto block = Sprite::create("block.png");
			int xOffset = padding +
				block->getContentSize().width / 2 +
				(block->getContentSize().width + padding) * i;
			block->setPosition(xOffset, visibleSize.y - 50 - 50 * n);
			block->setTag(BLOCK_BODYS_TAG);
			auto blockBody = PhysicsBody::createBox(block->getContentSize(),
				PhysicsMaterial(10.0f, 0.1, 0.4f));
			blockBody->setContactTestBitmask(true);
			blockBody->setDynamic(false);
			block->setPhysicsBody(blockBody);
			//	block->addComponent(blockBody);
			this->addChild(block);
			paddles++;
		}
	}

	mouse = Node::create();
	mouseBody = PhysicsBody::create(PHYSICS_INFINITY, PHYSICS_INFINITY);
	mouseBody->setDynamic(false);
	mouse->setPhysicsBody(mouseBody);
	//mouse->setPosition(paddle->getPosition());
	this->addChild(mouse);

}



// on "init" you need to initialize your instance
bool MainScene::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::initWithPhysics())
	{
		return false;
	}

	auto visibleSize = _director->getVisibleSize();
	auto origin = _director->getVisibleOrigin();
	auto safeArea = _director->getSafeAreaRect();
	auto safeOrigin = safeArea.origin;

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
		AX_CALLBACK_1(MainScene::menuCloseCallback, this));

	if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0)
	{
		problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
	}
	else
	{
		float x = safeOrigin.x + safeArea.size.width - closeItem->getContentSize().width / 2;
		float y = safeOrigin.y + closeItem->getContentSize().height / 2;
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
	//ebNode->setPhysicsBody(ebBody);
	ebNode->addComponent(ebBody);
	ebNode->setPosition(visibleSize / 2);
	this->addChild(ebNode);

	// Some templates (uncomment what you  need)
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = AX_CALLBACK_2(MainScene::onTouchBegan, this);
	touchListener->onTouchMoved = AX_CALLBACK_2(MainScene::onTouchMoved, this);
	touchListener->onTouchEnded = AX_CALLBACK_2(MainScene::onTouchEnded, this);
	touchListener->onTouchCancelled = AX_CALLBACK_2(MainScene::onTouchCancelled, this);
	_eventDispatcher->addEventListenerWithFixedPriority(touchListener, -128);

	//auto mouseListener           = EventListenerMouse::create();
	//mouseListener->onMouseMove   = AX_CALLBACK_1(MainScene::onMouseMove, this);
	//mouseListener->onMouseUp     = AX_CALLBACK_1(MainScene::onMouseUp, this);
	//mouseListener->onMouseDown   = AX_CALLBACK_1(MainScene::onMouseDown, this);
	//mouseListener->onMouseScroll = AX_CALLBACK_1(MainScene::onMouseScroll, this);
	//_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

	//auto keyboardListener           = EventListenerKeyboard::create();
	//keyboardListener->onKeyPressed  = AX_CALLBACK_2(MainScene::onKeyPressed, this);
	//keyboardListener->onKeyReleased = AX_CALLBACK_2(MainScene::onKeyReleased, this);
	//_eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 11);


	// contact listener
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = AX_CALLBACK_1(MainScene::onContactBegin, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);

	// scheduleUpdate() is required to ensure update(float) is called on every loop
	scheduleUpdate();

	return true;
}

bool MainScene::onTouchBegan(Touch* touch, Event* event)
{
	auto location = touch->getLocation();
	auto arr = _physicsWorld->getShapes(location);

	PhysicsBody* body = nullptr;
	for (auto&& obj : arr)
	{
		if ((obj->getBody()->getTag() & DRAG_BODYS_TAG) != 0)
		{
			body = obj->getBody();
			break;
		}
	}

	if (body != nullptr)
	{
		//Node* mouse = Node::create();
		//auto physicsBody = PhysicsBody::create(PHYSICS_INFINITY, PHYSICS_INFINITY);
		//physicsBody->setDynamic(false);
		//mouse->addComponent(physicsBody);
		mouse->setPosition(location);
		//this->addChild(mouse);
		PhysicsJointPin* joint = PhysicsJointPin::construct(mouseBody, body, location);
		joint->setMaxForce(5000.0f * body->getMass());
		_physicsWorld->addJoint(joint);
		_mouses.insert(std::make_pair(touch->getID(), mouse));
		return true;
	}

	return false;
}

void MainScene::onTouchMoved(Touch* touch, Event* event)
{
	auto it = _mouses.find(touch->getID());

	if (it != _mouses.end())
	{
		it->second->setPosition(touch->getLocation());
	}
}

void MainScene::onTouchEnded(Touch* touch, Event* event)
{
	auto it = _mouses.find(touch->getID());

	if (it != _mouses.end())
	{
		this->removeChild(it->second);
		_mouses.erase(it);
	}
}

void MainScene::onTouchCancelled(Touch* touch, Event* event)
{
	auto it = _mouses.find(touch->getID());

	if (it != _mouses.end())
	{
		this->removeChild(it->second);
		_mouses.erase(it);
	}
}


void MainScene::onMouseDown(Event* event)
{
	EventMouse* e = static_cast<EventMouse*>(event);
	AXLOG("onMouseDown detected, Key: %d", static_cast<int>(e->getMouseButton()));
}

void MainScene::onMouseUp(Event* event)
{
	EventMouse* e = static_cast<EventMouse*>(event);
	AXLOG("onMouseUp detected, Key: %d", static_cast<int>(e->getMouseButton()));
}

void MainScene::onMouseMove(Event* event)
{
	EventMouse* e = static_cast<EventMouse*>(event);
	AXLOG("onMouseMove detected, X:%f  Y:%f", e->getCursorX(), e->getCursorY());
}

void MainScene::onMouseScroll(Event* event)
{
	EventMouse* e = static_cast<EventMouse*>(event);
	AXLOG("onMouseScroll detected, X:%f  Y:%f", e->getScrollX(), e->getScrollY());
}

void MainScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event)
{
	AXLOG("onKeyPressed, keycode: %d", static_cast<int>(code));
}

void MainScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event)
{
	AXLOG("onKeyReleased, keycode: %d", static_cast<int>(code));
}

void MainScene::update(float delta)
{
	switch (_gameState)
	{
	case GameState::init:
	{
		_gameState = GameState::update;
		break;
	}

	case GameState::update:
	{
		/////////////////////////////
		// Add your codes below...like....
		// 
		// UpdateJoyStick();
		// UpdatePlayer();
		// UpdatePhysics();
		// ...
		AXLOG("ballBody->getVelocity().length():  %f", ballBody->getVelocity().length());
		if (ballBody->getVelocity().length() <= 100)
		{
			//Vec2 force = Vec2(300, 300);
			//ballBody->applyImpulse(force);
		}
		break;
	}

	case GameState::pause:
	{
		/////////////////////////////
		// Add your codes below...like....
		//
		// anyPauseStuff()

		break;
	}

	case GameState::menu1:
	{    /////////////////////////////
		// Add your codes below...like....
		// 
		// UpdateMenu1();
		break;
	}

	case GameState::menu2:
	{    /////////////////////////////
		// Add your codes below...like....
		// 
		// UpdateMenu2();
		break;
	}

	case GameState::end:
	{    /////////////////////////////
		// Add your codes below...like....
		// 
		// CleanUpMyCrap();
		menuCloseCallback(this);
		break;
	}

	} //switch
}

void MainScene::menuCloseCallback(Ref* sender)
{
	// Close the axmol game scene and quit the application
	_director->end();

	/*To navigate back to native iOS screen(if present) without quitting the application  ,do not use
	 * _director->end() as given above,instead trigger a custom event created in RootViewController.mm
	 * as below*/

	 // EventCustom customEndEvent("game_scene_close_event");
	 //_eventDispatcher->dispatchEvent(&customEndEvent);
}

bool MainScene::onContactBegin(PhysicsContact& contact)
{
	bool paddleFound = false;
	Vector<Node*> toDestroy;
	auto shapeA = contact.getShapeA();
	auto bodyA = shapeA->getBody();
	auto spriteA = bodyA->getNode();
	auto bodyB = contact.getShapeB()->getBody();
	auto spriteB = bodyB->getNode();
	if (spriteA->getTag() == BALL_BODY_TAG && spriteB->getTag() == GROUND_BODY_TAG
		|| spriteA->getTag() == GROUND_BODY_TAG && spriteB->getTag() == BALL_BODY_TAG)
	{
		// YOU LOSE
		Director::getInstance()->replaceScene(GameOver::createScene(false));
		this->getEventDispatcher()->removeAllEventListeners();
	}

	if (spriteA->getTag() == BALL_BODY_TAG && spriteB->getTag() == BLOCK_BODYS_TAG)
	{
		//Vec2 force = Vec2(3000, 3000);
		//bodyA->applyImpulse(force);
		if (find(toDestroy.begin(), toDestroy.end(), spriteB) == toDestroy.end())
			toDestroy.pushBack(spriteB);

	}
	else if (spriteA->getTag() == BLOCK_BODYS_TAG && spriteB->getTag() == BALL_BODY_TAG)
	{
		//Vec2 force = Vec2(300, 300);
		//bodyB->applyImpulse(force);
		if (find(toDestroy.begin(), toDestroy.end(), spriteA) == toDestroy.end())
			toDestroy.pushBack(spriteA);
	}
	else if (spriteA->getTag() == BALL_BODY_TAG && spriteB->getTag() == DRAG_BODYS_TAG)
	{

		Vec2 force = Vec2(300, 300);
		bodyA->applyImpulse(force);
	/*	if (find(toDestroy.begin(), toDestroy.end(), spriteA) == toDestroy.end())
			toDestroy.pushBack(spriteA);*/
	}

	Vector<Node*>::iterator pos;
	for (pos = toDestroy.begin(); pos != toDestroy.end(); pos++)
	{
		auto sprite = *pos;
		auto body = sprite->getPhysicsBody();
		this->removeChild(sprite, true);
		_physicsWorld->removeBody(body);
		paddles--;
	}

	if (paddles > 0)
		paddleFound = true;

	if (!paddleFound)
	{
		// YOU WON
		Director::getInstance()->replaceScene(GameOver::createScene(true));
		this->getEventDispatcher()->removeAllEventListeners();
	}
	return true;
}
