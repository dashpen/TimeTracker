/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
#include "SaveUtils.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
	bool init(GJGameLevel * level, bool challenge) {

		if (!LevelInfoLayer::init(level, challenge)) return false;

		auto timeSettings = CCMenuItemSpriteExtra::create(
			CCSprite::create("TTSettingsButton.png"_spr),
			this,
			menu_selector(MyLevelInfoLayer::onTimeSettings)
		);

		auto menu = this->getChildByID("left-side-menu");
		menu->addChild(timeSettings);

		timeSettings->setID("time-settings"_spr);

		menu->updateLayout();

		return true;
	}
	void onTimeSettings(CCObject*) {
		gd::string description;
		std::vector<int> timeObj = Mod::get()->getSavedValue<std::vector<int>>(std::to_string(m_level->m_levelID.value()), std::vector<int>());

		if (timeObj.empty()) {
			FLAlertLayer::create("Time Played", "<cr>No time recorded</c>", "OK")->show();
			return;
		}

		bool hhmmssFormat = Mod::get()->getSettingValue<bool>("hhmmss-time-format");
		bool hoursFormat = Mod::get()->getSettingValue<bool>("hours-only-time-format");
		bool timeWithPaused = Mod::get()->getSettingValue<bool>("time-with-paused");
		bool timeWithoutPaused = Mod::get()->getSettingValue<bool>("time-without-paused");

		if (!(timeWithoutPaused && timeWithPaused)) {
			FLAlertLayer::create("Time Played", "You need to select either total time or time excluding pause time in settings!", "OK")->show();
			return;
		}

		if (timeWithPaused) {
			int seconds = timeObj[0];
			int minutes = seconds / 60;
			int hours = seconds / 3600;

			description += "<cy>Total Time</c>:\n";
			if (hhmmssFormat) {
				description += std::to_string(hours) + " hours, ";
				description += std::to_string(minutes % 60) + " minutes, ";
				description += std::to_string(seconds % 60) + " seconds\n";
			}
			if (hoursFormat) {
				char buffer[20];
				std::sprintf(buffer, "%.3f", seconds / 3600.0f);
				if (hhmmssFormat) description += "or ";
				description.append(buffer);
				description += " hours\n";
			}
		}
		if (timeWithoutPaused) {
			int seconds = timeObj[0] - timeObj[1];
			int minutes = seconds / 60;
			int hours = seconds / 3600;

			description += "<cy>Time excluding pause menu</c>:\n";
			if (hhmmssFormat) {
				description += std::to_string(hours) + " hours, ";
				description += std::to_string(minutes % 60) + " minutes, ";
				description += std::to_string(seconds % 60) + " seconds\n";
			}
			if (hoursFormat) {
				char buffer[20];
				std::sprintf(buffer, "%.3f", seconds / 3600.0f);
				if (hhmmssFormat) description += "or ";
				description.append(buffer);
				description += " hours\n";
			}
		}
		FLAlertLayer::create("Time Played", description, "OK")->show();
	}
};

#include <Geode/modify/PlayLayer.hpp>
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

		int secondsPlayed = seconds.count();
		int secondsPaused = m_fields->m_pauseTime.count();

		std::vector<int> times = {secondsPlayed, secondsPaused};
		SaveUtils::addTime(m_level, times);

		PlayLayer::~PlayLayer();
	}

	void onQuit() {
		PlayLayer::onQuit();
		m_fields->updatePauseTime();
	}
	void pauseGame(bool bl) {
		PlayLayer::pauseGame(bl);
		m_fields->m_pausePoint = std::chrono::steady_clock::now();
		m_fields->m_loggingPaused = true;
	}
	void resume() {
		PlayLayer::resume();
		m_fields->updatePauseTime();
	}
};