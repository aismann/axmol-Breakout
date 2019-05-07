#ifndef __Breakout__Gameover__
#define __Breakout__Gameover__

#include "cocos2d.h"
USING_NS_CC;

class Gameover : public Scene
{
public:
    static Scene* createScene(bool win);
	static Gameover* create(bool win);
    virtual bool init(bool win);
};

#endif // __Breakout__Gameover__
