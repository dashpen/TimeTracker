#include <Geode/Geode.hpp>
#include "SaveUtils.hpp"
using namespace geode::prelude;

void SaveUtils::addTime(GJGameLevel* level, const std::vector<int>& times) {
	if (level->m_levelType != GJLevelType::Saved) return; //todo: add support for editor, default, local levels
	std::vector<int> savedTimes = Mod::get()->getSavedValue<std::vector<int>>(std::to_string(level->m_levelID.value()));
	if (savedTimes.empty()) {
		Mod::get()->setSavedValue<std::vector<int>>(std::to_string(level->m_levelID.value()), times);
		return;
	}
	savedTimes[0] += times[0];
	savedTimes[1] += times[1];
	Mod::get()->setSavedValue<std::vector<int>>(std::to_string(level->m_levelID.value()), savedTimes);
}