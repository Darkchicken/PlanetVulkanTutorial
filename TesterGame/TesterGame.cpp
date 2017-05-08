#include "TesterGame.h"



TesterGame::TesterGame()
{
}


TesterGame::~TesterGame()
{
}

void TesterGame::Run()
{
	InitSystems();

}

void TesterGame::InitSystems()
{
	m_engine.InitVulkan();

	m_engine.GameLoop();
}

