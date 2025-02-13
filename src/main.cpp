/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
#include "Saving.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
	bool init(GJGameLevel * level, bool challenge) {

		if (!LevelInfoLayer::init(level, challenge)) return false;

		auto timeSettings = CCMenuItemSpriteExtra::create(
			CCSprite::create("ttbuttonmidlinedark.png"_spr),
			this,
			menu_selector(MyLevelInfoLayer::onTimeSettings)
		);

		auto menu = this->getChildByID("left-side-menu");
		menu->addChild(timeSettings);

		timeSettings->setID("time-settings"_spr);

		menu->updateLayout(true);

		return true;
	}
	void onTimeSettings(CCObject*) {
		gd::string message;
		std::vector<int> timeObj = Mod::get()->getSavedValue<std::vector<int>>(std::to_string(m_level->m_levelID.value()), std::vector<int>());

		if (!timeObj.empty()) {
			int seconds = timeObj[0];
			int minutes = seconds / 60;
			int hours = seconds / 3600;
			message = "Time played: ";
			message += std::to_string(hours) + " hours, ";
			message += std::to_string(minutes % 60) + " minutes, ";
			message += std::to_string(seconds % 60) + " seconds";
		} else {
			message = "No time recorded";
		}

		if (m_level->m_levelType != GJLevelType::Saved) {
			message = "This level type is not supported";
		} // shouldn't happen but just in case

		FLAlertLayer::create("Time Played", message, "OK")->show();
	}
};

#include <Geode/modify/PlayLayer.hpp>
//#include <Geode/modify/GJGameLevel.hpp>
#include <chrono>

class $modify(PlayLayer) {
	struct Fields {
		std::chrono::steady_clock::time_point m_sessionStart = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_pausePoint = std::chrono::steady_clock::now();
		std::chrono::seconds m_pauseTime = std::chrono::seconds::zero();
		bool m_loggingPaused = false;
		void updatePauseTime() {
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			std::chrono::duration duration = now - m_pausePoint;
			std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
			m_pauseTime += seconds;
			m_loggingPaused = false;
		}
	};
	bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {

		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

		m_fields->m_sessionStart = std::chrono::steady_clock::now();

		return true;
	}

	void destructor() {
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		std::chrono::duration duration = now - m_fields->m_sessionStart;
		std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

		if(m_isPaused && m_fields->m_loggingPaused) m_fields->updatePauseTime();
		log::info("time played: {}, hours: {}", seconds, std::chrono::duration_cast<std::chrono::hours>(seconds));
		log::info("time paused: {}, hours: {}", m_fields->m_pauseTime, std::chrono::duration_cast<std::chrono::hours>(m_fields->m_pauseTime));

		int secondsPlayed = seconds.count();
		int secondsPaused = m_fields->m_pauseTime.count();

		std::vector<int> times = {secondsPlayed, secondsPaused};
		SaveUtils::addTime(m_level, times);

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
		m_fields->m_loggingPaused = true;
		log::info("pauseGame: {}", bl);
	}
	void resume() {
		PlayLayer::resume();
		m_fields->updatePauseTime();
		log::info("resume");
	}
};