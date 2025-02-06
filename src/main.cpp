/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>


using namespace geode::prelude;

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
	bool init(GJGameLevel * level, bool challenge) {

		if (!LevelInfoLayer::init(level, challenge)) return false;

		auto timeSettings = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
			this,
			/**
			 * Here we use the name we set earlier for our modify class.
			*/
			menu_selector(MyLevelInfoLayer::onMyButton)
		);

		auto menu = this->getChildByID("left-side-menu");
		menu->addChild(timeSettings);

		timeSettings->setID("time-settings-button"_spr);

		menu->updateLayout();

		log::debug("Hello! This is a debug message in init!");

		return true;
	}
	void onMyButton(CCObject*) {
		log::debug("Hello! This is a debug message");
		FLAlertLayer::create("Geode", "Hello from my custom mod!", "OK")->show();
	}
};

#include <Geode/modify/PlayLayer.hpp>
#include <chrono>

class $modify(PlayLayer) {
	struct Fields {
		std::chrono::steady_clock::time_point m_sessionStart = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_pausePoint;
		std::chrono::seconds m_pauseTime = std::chrono::seconds::zero();
		void updatePauseTime() {
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			std::chrono::duration duration = now - m_pausePoint;
			std::chrono::seconds hours = std::chrono::duration_cast<std::chrono::seconds>(duration);
			m_pauseTime += hours;
		}
	};
	bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {

		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

		m_fields->m_sessionStart = std::chrono::steady_clock::now();

		log::debug("Hooked into play layer!!!");

		return true;
	}

	void destructor() {
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		std::chrono::duration duration = now - m_fields->m_sessionStart;
		std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
		//double times = seconds.count();

		if(m_isPaused) m_fields->updatePauseTime();

		log::info("time played: {}, hours: {}", seconds, std::chrono::duration_cast<std::chrono::hours>(seconds));
		log::info("time paused: {}, hours: {}", m_fields->m_pauseTime, std::chrono::duration_cast<std::chrono::hours>(m_fields->m_pauseTime));
		log::info("now: {}, duration: {}, seconds: {}", now.time_since_epoch(), duration, seconds);
		log::info("Is the game paused? {}!", m_isPaused);
		PlayLayer::~PlayLayer();
	}
	void onExit() {
		PlayLayer::onExit();
		log::info("onExit");
	}
	void onQuit() {
		PlayLayer::onQuit();
		m_fields->updatePauseTime();
		log::info("onQuit");
	}
	void pauseGame(bool bl) {
		PlayLayer::pauseGame(bl);
		m_fields->m_pausePoint = std::chrono::steady_clock::now();
		log::info("pauseGame: {}", bl);
	}
	void resume() {
		PlayLayer::resume();
		m_fields->updatePauseTime();
		log::info("resume");
	}
};