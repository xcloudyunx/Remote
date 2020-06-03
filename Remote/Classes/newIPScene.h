#ifndef __NEWIP_SCENE_H__
#define __NEWIP_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "HomeScene.h"

using namespace cocos2d;
using namespace ui;

class newIP : public Scene, public EditBoxDelegate {
public:
	static Scene* createScene();

	virtual bool init();

    CREATE_FUNC(newIP);
	
	virtual void editBoxReturn(EditBox* editbox);
	
private:
	const char *IP = UserDefault::getInstance()->getStringForKey("ip").c_str();
	const int PORT = 1235;
};

#endif // __NEWIP_SCENE_H__