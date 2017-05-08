#pragma once

#include <PVEngine/Window.h>
#include <PVEngine/PlanetVulkan.h>

class TesterGame
{
public:
	TesterGame();
	~TesterGame();

	void Run();

private:

	void InitSystems();

	//member variables
	PVEngine::PlanetVulkan m_engine;
};

