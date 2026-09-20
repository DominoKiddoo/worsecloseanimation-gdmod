#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <prevter.imageplus/include/api.hpp>

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {



	
    void onQuit(CCObject*) {
        geode::createQuickPopup(
            "Quit Game",
            "Are you sure you want to <cr>quit</c>?",
            "Cancel", "Yes",
            [this](auto, bool btn2) {
                if (btn2) {
                    customQuit();
                }
            }
        );
    }
	

    void customQuit() {
        auto app = AppDelegate::get();
        app->trySaveGame(true);
        app->showLoadingCircle(false, true, false);

        std::string type = Mod::get()->getSettingValue<std::string>("type");
        if (type == "Random") {
            if (geode::utils::random::generate<bool>()) {
                nuclearBomb();
            } else {
                grenade();
            }
        } else if (type == "Grenade") {
            grenade();
        } else {
            nuclearBomb();
        }
    }

    void nuclearBomb() {
        auto sprite = CCSprite::create("bomb.png"_spr);
        if (!sprite) return;

        auto director = CCDirector::get();
        auto winSize = director->getWinSize();

        OverlayManager::get()->addChild(sprite);

        sprite->setPosition(CCPoint(winSize.width / 2, winSize.height + sprite->getContentHeight()));
        sprite->setID("bomb"_spr);

        auto move = CCMoveTo::create(1.5f, CCPoint(winSize.width / 2, 0));
        auto easedMove = CCEaseIn::create(move, 2.0f);
        auto hide = CCHide::create();
        auto explosion = CCCallFunc::create(this, callfunc_selector(MyMenuLayer::kaboom));
        auto action = CCSequence::create(easedMove, hide, explosion, nullptr);

        FMODAudioEngine::get()->playEffect("falling.mp3"_spr);
        sprite->runAction(action);
    }

    void grenade() {
        auto sprite = CCSprite::create("grenade.png"_spr);
        if (!sprite) return;

        auto director = CCDirector::get();
        auto winSize = director->getWinSize();

        OverlayManager::get()->addChild(sprite);

        sprite->setPosition(CCPoint(0 - sprite->getContentWidth(), (winSize.height / 2) + 50));
        sprite->setID("bomb"_spr);

        auto moveX = CCMoveBy::create(2.0f, CCPoint(winSize.width / 2 + sprite->getContentWidth(), 0));
        auto moveY = CCMoveBy::create(1.5f, CCPoint(0, 8 + sprite->getContentHeight() - ((winSize.height / 2) + 50)));

        auto easedMoveX = CCEaseExponentialOut::create(moveX);
        auto easedMoveY = CCEaseBounceOut::create(moveY);

        auto delay = CCDelayTime::create(1.0f);
        auto explosion = CCCallFunc::create(this, callfunc_selector(MyMenuLayer::kaboom));
        auto action = CCSequence::create(CCSpawn::create(easedMoveX, easedMoveY, nullptr), delay, explosion, nullptr);

        FMODAudioEngine::get()->playEffect("chestLand.ogg");
        sprite->runAction(action);
    }

    void kaboom() {
        auto explosionSetup = CCSprite::create("bomb.webp"_spr);
        if (!explosionSetup) return;

        auto boom = imgp::AnimatedSprite::from(explosionSetup);
        if (!boom) return;

        boom->setForceLoop(false);
        boom->setCurrentFrame(0);

        OverlayManager::get()->addChild(boom);
        auto director = CCDirector::get();
        auto winSize = director->getWinSize();
        boom->setPosition(winSize / 2);

        auto contentSize = boom->getContentSize();
        if (contentSize.width > 0 && contentSize.height > 0) {
            boom->setScaleX(winSize.width / contentSize.width);
            boom->setScaleY(winSize.height / contentSize.height);
        }

        boom->setID("boomVideo"_spr);
        boom->setZOrder(200);
        boom->play();
        FMODAudioEngine::get()->playEffect("bomb.mp3"_spr);

        this->schedule(schedule_selector(MyMenuLayer::checkBombFinished), 0.0f);
    }

    void checkBombFinished(float dt) {
        auto bombNode = OverlayManager::get()->getChildByID("boomVideo"_spr);
        if (!bombNode) {
            log::info("bombNode does not exist!");
            this->unschedule(schedule_selector(MyMenuLayer::checkBombFinished));
            return;
        }

        auto bomb = static_cast<imgp::AnimatedSprite*>(bombNode);
        if (bomb->getCurrentFrame() >= bomb->getFrameCount() - 1) {
            this->unschedule(schedule_selector(MyMenuLayer::checkBombFinished));
            geode::utils::game::exit(true);
        }
    }
};