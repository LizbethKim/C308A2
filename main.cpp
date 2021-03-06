//---------------------------------------------------------------------------
//
// This software is provided 'as-is' for assignment of COMP308
// in ECS, Victoria University of Wellington,
// without any express or implied warranty.
// In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
// Copyright (c) 2012 by Taehyun Rhee
//
// Edited by Roma Klapaukh, Daniel Atkins, and Taehyun Rhee
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "define.h"
#include "G308_Skeleton.h"
#include <iostream>
#include <unistd.h>

GLuint g_mainWnd;
GLuint g_nWinWidth = G308_WIN_WIDTH;
GLuint g_nWinHeight = G308_WIN_HEIGHT;

void G308_keyboardListener(unsigned char, int, int);
void G308_Reshape(int w, int h);
void G308_display();
void G308_init();
void G308_SetCamera();
void G308_SetLight();
void G308_mouseListener(int, int, int, int);
void G308_motionListener(int, int);
void G308_timerCallBack(int);
int lastX = 0, lastY = 0, curX = 0, curY = 0;
int arcball = false;
int maxY = 0;
int frames;
int currFrame = 0;
int currentPose = 0;

Skeleton* skeleton;

int main(int argc, char** argv) {
	if (argc < 2 || argc > 3) {
		//Usage instructions for core and challenge
		printf("Usage\n");
		printf("./Ass2 priman.asf [priman.amc]\n");
		exit(EXIT_FAILURE);
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(g_nWinWidth, g_nWinHeight);
	g_mainWnd = glutCreateWindow("COMP308 Assignment2");

	glutKeyboardFunc(G308_keyboardListener);
	glutMouseFunc(G308_mouseListener);
	glutMotionFunc(G308_motionListener);
	glutDisplayFunc(G308_display);
	glutReshapeFunc(G308_Reshape);

	G308_init();
	frames = 0;

	// [Assignment2] : Read ASF file
	skeleton = new Skeleton(argv[1]);
	//If an additional argument is given, parse the motion capture file
	if (argc == 3){
		frames = skeleton->readACM(argv[2]);
	}
	skeleton->renderState(1);
	if (frames > 0){
		//If there is motion capture data stored, set animation start point
		//To frame 0
		skeleton->animate(0, 0);
	}

	glutMainLoop();

	free(skeleton);
	return EXIT_SUCCESS;
}

// Init Light and Camera
void G308_init() {

	G308_SetLight();
	G308_SetCamera();
}

// Display call back
void G308_display() {

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glShadeModel(GL_SMOOTH);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("%s\n", gluErrorString(err));
	}

	// [Assignmet2] : render skeleton
	if (skeleton != NULL) {
		skeleton->display();
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glutSwapBuffers();
}

void G308_keyboardListener(unsigned char key, int x, int y) {
	switch (key){
		//Keyboard turning!
		case 'a': glRotatef(-5, 0, 1, 0);
		break;
		case 'd': glRotatef(5, 0, 1, 0);
		break;
		case 'w': glRotatef(-5, 1, 0, 0);
		break;
		case 's': glRotatef(5, 1, 0, 0);
		break;
		//Shows everything, bone joints and axis vectors
		case '1': skeleton->renderState(1);
		break;
		//Only shows skeleton and bone joints
		case '2': skeleton->renderState(2);
		break;
		//Only shows skeleton
		case '3': skeleton->renderState(3);
		break;
		//Start motion capture animation
		case 'o': if (frames != 0) {
			G308_timerCallBack(0);
		} else {
			cout << "No animation loaded" << endl;
		}
		//Flip through the poses
		case 'p': skeleton->pose(currentPose);
		currentPose = (currentPose + 1)%3;
		skeleton->animate(0, 1);
		G308_display();
		break;
		break;
	}
	G308_display();
}

//Timer function for animation
void G308_timerCallBack (int value){
   skeleton->animate(value, 0);
   glutPostRedisplay();
   glutTimerFunc (1, G308_timerCallBack, (value + 1)%frames);
}

//Mouse rotations. GLUT has seperate listeners for mouse button presses
//and mouse movement. This means you have to listen to both seperately.
//Boolean helps to differentiate when mouse button is held down
void G308_mouseListener(int button, int state, int x, int y){
	//0 = Left GLUT_LEFT_BUTTON
	//1 = Middle GLUT_MIDDLE_BUTTON
	//2 = Right GLUT_RIGHT_BUTTON
	//3 = MouseUp GLUT_SOMETHING
	//4 = MouseDown GLUT_SOMETHINGELSE
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		arcball = true;
		lastX = curX = x;
		lastY = curY = y;
	} else {
		arcball = false;
	}
}

//If button is held down, track the change in mouse movement, and move accordingly
void G308_motionListener(int x, int y){
	if (arcball){
		curX = x;
		curY = y;
		glRotatef((curX-lastX), 0, 1, 0);
		if (maxY > -180 && maxY < 180){
			glRotatef((curY-lastY), 1, 0, 0);
		}


		if ((maxY + (curY-lastY)) < 200 && (maxY + (curY-lastY)) > (-200)){
			maxY += (curY-lastY);
		}
		lastY = curY;
		lastX = curX;
		glutPostRedisplay();
	}
}

// Reshape function
void G308_Reshape(int w, int h) {
	if (h == 0)
		h = 1;

	g_nWinWidth = w;
	g_nWinHeight = h;

	glViewport(0, 0, g_nWinWidth, g_nWinHeight);
}

// Set Camera Position
void G308_SetCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(G308_FOVY, (double) g_nWinWidth / (double) g_nWinHeight, G308_ZNEAR_3D, G308_ZFAR_3D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, 7.0, 0.0, 0., 0.0, 0.0, 1.0, 0.0);
}

// Set View Position
void G308_SetLight() {
	float direction[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float diffintensity[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, direction);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffintensity);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	glEnable(GL_LIGHT0);
}

