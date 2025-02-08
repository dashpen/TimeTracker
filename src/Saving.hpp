#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class Saving {
public:
	static void addTime(GJGameLevel* level, const std::vector<int>& times);
};