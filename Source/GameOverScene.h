#ifndef __Breakout__GameOver__
#define __Breakout__GameOver__

#include "axmol.h"


class GameOver : public ax::Scene
{
public:
    static ax::Scene* createScene(bool win);
	static GameOver* create(bool win);
    virtual bool init(bool win);
};

#endif // __Breakout__GameOver__
