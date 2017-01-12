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
	m_window.Create();

	m_engine.InitVulkan();

	GameLoop();
}

void TesterGame::GameLoop()
{
	while (!glfwWindowShouldClose(m_window.window))
	{
		glfwPollEvents();
	}
}
