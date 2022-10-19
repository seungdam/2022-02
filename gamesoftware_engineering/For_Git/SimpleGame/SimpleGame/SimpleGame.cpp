/*
Copyright 2022 Lee Taek Hee (Tech University of Korea)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/

#include "stdafx.h"
#include <iostream>
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"
#include "Global.h"
#include "GSEGame.h"

GSEGame *g_game = NULL;
float g_prev_elapseTime = 0.f;
GSEKeyBoardMapper g_keyMapper;

void RenderScene(void)
{
	float elapsedTime = glutGet(GLUT_ELAPSED_TIME) - g_prev_elapseTime;
	g_prev_elapseTime = glutGet(GLUT_ELAPSED_TIME);
	elapsedTime = elapsedTime / 1000.f;
	g_game->UpdateObjects(g_keyMapper,elapsedTime);
	// Renderer Test
	g_game->RenderScene();

	glutSwapBuffers();
}

void Idle(void)
{
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	RenderScene();
}

void KeyInput(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w'|'W':
		g_keyMapper.W_key = true;
		break;
	case 'a' | 'A':
		g_keyMapper.A_key = true;
		break;
	case 's' | 'S':
		g_keyMapper.S_key = true;
		break;
	case 'd' | 'D':
		g_keyMapper.D_key = true;
		break;
	}
	RenderScene();
}

void KeyUpInput(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w' | 'W':
		g_keyMapper.W_key = false;
		break;
	case 'a' | 'A':
		g_keyMapper.A_key = false;
		break;
	case 's' | 'S':
		g_keyMapper.S_key = false;
		break;
	case 'd' | 'D':
		g_keyMapper.D_key = false;
		break;
	}
	RenderScene();
}

void SpecialKeyInput(int key, int x, int y)
{
	RenderScene();
}

void SpecialKeyUpInput(int key, int x, int y)
{
	RenderScene();
}

int main(int argc, char **argv)
{
	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Game Software Engineering KPU");
	// opengl과 윈도우가 호환이 잘되도록 하는 브릿지 역할 = glew
	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}

	// Initialize Renderer
	//500 x 500 pixel 만큼 렌더링을 수행할 것이다.
	GSEVec2 temp{ 500.f,500.f };
	g_game = new GSEGame(temp);
	// 콜백 함수 등록 파트
	// 다른 스레드에서 동작하는 이벤트를 처리해주는 부분에서 각각의 함수를 호출한다. 
	glutDisplayFunc(RenderScene); // 랜더링
	glutIdleFunc(Idle); // 평상 시
	glutKeyboardFunc(KeyInput); // 키보드
	glutMouseFunc(MouseInput); //마우스
	glutSpecialFunc(SpecialKeyInput);
	glutKeyboardUpFunc(KeyUpInput);
	glutSpecialUpFunc(SpecialKeyUpInput);
	g_prev_elapseTime = glutGet(GLUT_ELAPSED_TIME);
	glutMainLoop(); // 이 루프를 무한으로 돌면서 위의 각각의 콜백 함수를 호출한다.

	if (g_game) {
		delete g_game;
		g_game = NULL;
	}

    return 0;
}

