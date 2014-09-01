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

#include <string.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "G308_Skeleton.h"
#include "define.h"
#include <iostream>
#include <cmath>
#include <unistd.h>

#define malloc_crash(x) if(!x){ printf("could not malloc at %d\n", __LINE__ -1); exit(1); }



Skeleton::Skeleton(char* filename) {
	numBones = 1;
	buffSize = 200;
	maxBones = 60;
	angle = 0;
	root = (bone*) malloc(sizeof(bone) * maxBones);

	for (int i = 0; i < 60; i++) {
		root[i].numChildren = 0;
		root[i].dirx = 0;
		root[i].diry = 0;
		root[i].dirz = 0;
		root[i].rotx = 0;
		root[i].roty = 0;
		root[i].rotz = 0;
		root[i].dof = DOF_NONE;
		root[i].length = 0;
		root[i].name = NULL;
		root[i].children = (bone**) malloc(sizeof(bone*) * 5);

		//Challenge stuff
		root[i].currentTranslatex = 0;
		root[i].currentTranslatey = 0;
		root[i].currentTranslatez = 0;
		root[i].currentRotationx = 0;
		root[i].currentRotationy = 0;
		root[i].currentRotationz = 0;

	}
	char*name = (char*) malloc(sizeof(char) * 5);
	name[0] = 'r';
	name[1] = name[2] = 'o';
	name[3] = 't';
	name[4] = '\0';
	root[0].name = name;
	root[0].dof = DOF_ROOT;
	readASF(filename);
}

Skeleton::~Skeleton() {
	deleteBones(root);
}

void Skeleton::deleteBones(bone* root) {
	for (int i = 0; i < maxBones; i++) {
		if (root[i].name != NULL) {
			free(root[i].name);
		}
		if (root[i].children != NULL) {
			free(root[i].children);
		}
	}
	free(root);
}

void Skeleton::renderState(int x){
	rS = x;
}

// [Assignment2] you may need to revise this function
void Skeleton::display() {
	if (root == NULL) {
		return;
	}
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(0.05, 0.05, 0.05);

	GLUquadric* quad = gluNewQuadric(); //Create a new quadric to allow you to draw cylinders
	if (quad == 0) {
		printf("Not enough memory to allocate space to draw\n");
		exit(EXIT_FAILURE);
	}
	//Actually draw the skeleton
	gluQuadricDrawStyle(quad, GLU_FILL);
	gluQuadricOrientation(quad, GLU_INSIDE);

	display(root, quad);

	gluDeleteQuadric(quad);
	glPopMatrix();
}

// [Assignment2] you need to fill this function
void Skeleton::display(bone* root, GLUquadric* q) {
	if (root == NULL) {
		return;
	}
	float theta = acos(root->dirz) * 180 / M_PI;
	//All options enabled, will show bone joints and x, y, z vectors
	if (rS < 4){		
		glPushMatrix();
		
		//Apply current bone rotations, and translations
		glRotatef(root->rotz, 0, 0, 1.0);
		glRotatef(root->roty, 0, 1.0, 0);
		glRotatef(root->rotx, 1.0, 0, 0);		

		glRotatef(root->currentRotationz, 0, 0, 1.0);
		glRotatef(root->currentRotationy, 0, 1.0, 0);
		glRotatef(root->currentRotationx, 1.0, 0, 0);

		glTranslatef(root->currentTranslatex, root->currentTranslatey, root->currentTranslatez);
	
		glRotatef(-root->rotx, 1.0, 0, 0);
		glRotatef(-root->roty, 0, 1.0, 0);
		glRotatef(-root->rotz, 0, 0, 1.0);

		glRotatef(theta, -root->diry, root->dirx, 0);

		//Drawing the bone joints
		if (rS < 3){

			glColor3f(0, 1, 1);
			glutSolidSphere(0.4, 100, 100);
			//Drawing the bone axis vectors
			if (rS < 2){
				glPushMatrix();
					glColor3f(0,0,1);
					gluCylinder(q, 0.1, 0.1, 1, 100, 100);
					glutSolidCone(0.2,0.2,100,100);
				glPopMatrix();

				glPushMatrix();
					glColor3f(0,1,0);
					glRotatef(-90,1,0,0);
					gluCylinder(q, 0.1, 0.1, 1, 100, 100);
				glPopMatrix();

				glPushMatrix();
					glColor3f(1,0,0);
					glRotatef(-90, 0, 1, 0);
					gluCylinder(q, 0.1, 0.1, 1, 100, 100);
				glPopMatrix();
			}
		}
	}


	//Draw actual bone
	glColor3f(1,1,1);
	gluCylinder(q, 0.2, 0.2, root->length, 50, 50);
	glRotatef(-theta, -root->diry, root->dirx, 0);
	glTranslatef(root->dirx*root->length, root->diry*root->length, root->dirz*root->length);
	int i;
	//Iterate over all children
	for (i = 0; i < root->numChildren; i++){
		display(root->children[i], q);
	}
	glPopMatrix();
}

void Skeleton::poseStateSet(int x){
	poseState = x;
}

bool Skeleton::readASF(char* filename) {
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		printf("Failed to open file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	char* buff = new char[buffSize];
	char *p;

	printf("Starting reading skeleton file\n");
	while ((p = fgets(buff, buffSize, file)) != NULL) {
		//Reading actually happened!

		char* start = buff;
		trim(&start);

		//Check if it is a comment or just empty
		if (buff[0] == '#' || buff[0] == '\0') {
			continue;
		}

		start = strchr(buff, ':');
		if (start != NULL) {
			//This actually is a heading
			//Reading may actually consume the next heading
			//so headings need to be a recursive construct?
			readHeading(buff, file);
		}
	}

	delete[] buff;
	fclose(file);
	printf("Completed reading skeleton file\n");
	return true;
}

int Skeleton::readACM(char* filename){
	FILE* file = fopen(filename, "r");
	buffSize = 200;
	if (file == NULL){
		printf("PUT IN A VALID ACM FILE %s\n", filename);
		exit(EXIT_FAILURE);
	}

	char* buff = new char[buffSize];
	char *p;
	int frame = 0;

	printf("Starting reading acm file\n");
	//The very last lone number in a file is the number of frames in
	//the current motion capture file. This iterates through the file
	//until reaching that number. Stores in frame
	while ((p = fgets(buff, buffSize, file)) != NULL){
		char* start = buff;
		trim(&start);

		if (buff[0] == '#' || buff[0] == '\0' || buff[0] == ':'){
			continue;
		}
		if(sscanf(start, "%d\b", &frame)){}
	}
	printf("%d\n", frame);
	//Go back to the start of the file, for actual file reading
	if (fseek(file, 0L, SEEK_SET ) == 0){
		animRot = (float***)malloc(sizeof(float**)*frame);
		malloc_crash(animRot);
		readACMHeading(file, frame);
	} else {
		printf("Return to start of file failed\n");
		return 0;
	}
	numFrames = frame;
	return frame;
}

void Skeleton::animate(int i, int mode){
	animDisplay(root, i, mode);
	glutPostRedisplay();
}

void Skeleton::animDisplay(bone* root, int currFrame, int mode) {
	if (root == NULL) {
		return;
	}
	//Reading in an ACM file
	if (mode == 0){
		//Set current root bone translation
		float transX = animRot[currFrame][0][3];
		float transY = animRot[currFrame][0][4];
		float transZ = animRot[currFrame][0][5];

		int i;

		root[0].currentTranslatex = transX;
		root[0].currentTranslatey = transY;
		root[0].currentTranslatez = transZ;
		for (i = 0; i < 29; i++){
			if (animRot[currFrame][i] == NULL) continue;
			//For every bone, insert the current rotation values into the x,y,z
			root[i].currentRotationx = animRot[currFrame][i][0];
			root[i].currentRotationy = animRot[currFrame][i][1];
			root[i].currentRotationz = animRot[currFrame][i][2];
		}
	//For custom poses, only 3 possibilities, rest same as above
	} else if (mode == 1){
		float transX = animPose[0][3];
		float transY = animPose[0][4];
		float transZ = animPose[0][5];

		int i;

		root[0].currentTranslatex = transX;
		root[0].currentTranslatey = transY;
		root[0].currentTranslatez = transZ;
		for (i = 0; i < 29; i++){
			if (animRot[currFrame][i] == NULL) continue;
			root[i].currentRotationx = animPose[i][0];
			root[i].currentRotationy = animPose[i][1];
			root[i].currentRotationz = animPose[i][2];
		}
	}
}

void Skeleton::pose(int pose){
	char const* poseFile[29];
	int j, df;
	float v1, v2, v3, v4, v5, v6;
	char name[200];
	//Manual addition of 3 individual poses from motion capture files
	switch (pose){
		case 0:
		poseFile[0] = "root -0.00368816 16.2109 0.900333 -7.83816 17.0395 -3.03423";
		poseFile[1] = "lowerback 11.9247 3.68627 -1.94793";
		poseFile[2] = "upperback 3.60027 4.79134 2.81308";
		poseFile[3] = "thorax -3.29766 2.39669 3.31961";
		poseFile[4] = "lowerneck -20.7899 0.64661 3.67078";
		poseFile[5] = "upperneck 33.7031 0.737089 -2.64472";
		poseFile[6] = "head 15.3296 0.481401 -1.59538";
		poseFile[7] = "rclavicle 1.61016e-014 -2.72335e-014";
		poseFile[8] = "rhumerus 64.8893 36.0376 -51.268";
		poseFile[9] = "rradius 51.5009";
		poseFile[10] = "rwrist -8.04063";
		poseFile[11] = "rhand -9.68608 28.7397";
		poseFile[12] = "rfingers 7.12502";
		poseFile[13] = "rthumb 16.2859 -0.596351";
		poseFile[14] = "lclavicle 1.61016e-014 -2.72335e-014";
		poseFile[15] = "lhumerus 38.5572 -11.7359 52.3875";
		poseFile[16] = "lradius 47.3848";
		poseFile[17] = "lwrist -9.66758";
		poseFile[18] = "lhand -22.0274 18.9423";
		poseFile[19] = "lfingers 7.12502";
		poseFile[20] = "lthumb 4.38227 48.877";
		poseFile[21] = "rfemur 44.4261 -145.848 -86.3876";
		poseFile[22] = "rtibia 36.1183";
		poseFile[23] = "rfoot -112.459 -132.107";
		poseFile[24] = "rtoes 56.4287";
		poseFile[25] = "lfemur -27.9 6.43214 -25.952";
		poseFile[26] = "ltibia 55.3268";
		poseFile[27] = "lfoot -9.01973 4.86985";
		poseFile[28] = "ltoes 4.53009";
		break;
		case 1:
		poseFile[0] = "root 7.46979 15.9909 -36.6099 6.97364 1.17677 -3.42769";
		poseFile[1] = "lowerback -4.02947 0.124717 0.686617";
		poseFile[2] = "upperback -1.74341 0.122466 2.40941";
		poseFile[3] = "thorax 0.46583 0.0663449 2.09235";
		poseFile[4] = "lowerneck -18.3092 -4.94712 -12.866";
		poseFile[5] = "upperneck 13.9808 -6.45508 16.5981";
		poseFile[6] = "head 8.36631 -2.21315 7.00226";
		poseFile[7] = "rclavicle 9.405e-015 1.19271e-015";
		poseFile[8] = "rhumerus -39.2962 17.4621 -84.4896";
		poseFile[9] = "rradius 56.9177";
		poseFile[10] = "rwrist -29.7836";
		poseFile[11] = "rhand -32.5089 -26.7042";
		poseFile[12] = "rfingers 7.12502";
		poseFile[13] = "rthumb -5.7401 -56.6064";
		poseFile[14] = "lclavicle 9.405e-015 1.19271e-015";
		poseFile[15] = "lhumerus -36.6135 -3.05608 86.2016";
		poseFile[16] = "lradius 27.605";
		poseFile[17] = "lwrist 7.3759";
		poseFile[18] = "lhand -13.2527 -23.6778";
		poseFile[19] = "lfingers 7.12502";
		poseFile[20] = "lthumb 12.8496 5.90672";
		poseFile[21] = "rfemur -9.10523 4.99331 29.8867";
		poseFile[22] = "rtibia 23.7929";
		poseFile[23] = "rfoot 0 0";
		poseFile[24] = "rtoes 0";
		poseFile[25] = "lfemur -40.6314 5.96489 -16.4186";
		poseFile[26] = "ltibia 41.7651";
		poseFile[27] = "lfoot -14.4396 7.79462";
		poseFile[28] = "ltoes -11.6514";
		break;
		case 2:
		poseFile[0] = "root 0.129681 15.655 -6.34388 338.531 -27.0544 -358.348";
		poseFile[1] = "lowerback 24.0774 1.59426 -1.73394";
		poseFile[2] = "upperback 12.9197 1.36177 -0.772827";
		poseFile[3] = "thorax 0.804141 0.483394 -0.155251";
		poseFile[4] = "lowerneck -6.99849 0.73177 -6.39631";
		poseFile[5] = "upperneck 9.52563 1.36264 3.0843";
		poseFile[6] = "head 3.92502 0.601811 2.05643";
		poseFile[7] = "rclavicle -2.56432e-014 -5.1684e-015";
		poseFile[8] = "rhumerus -37.9902 76.0358 -79.4384";
		poseFile[9] = "rradius 89.1912";
		poseFile[10] = "rwrist 40.5623";
		poseFile[11] = "rhand -51.4538 33.6299";
		poseFile[12] = "rfingers 7.12502";
		poseFile[13] = "rthumb -23.9839 5.10817";
		poseFile[14] = "lclavicle -2.56432e-014 -5.1684e-015";
		poseFile[15] = "lhumerus -18.8041 -81.9355 61.3737";
		poseFile[16] = "lradius 84.1348";
		poseFile[17] = "lwrist -23.877";
		poseFile[18] = "lhand -39.2768 10.1551";
		poseFile[19] = "lfingers 7.12502";
		poseFile[20] = "lthumb -12.2706 39.7751";
		poseFile[21] = "rfemur -38.039 -6.21629 4.50249";
		poseFile[22] = "rtibia 78.3629";
		poseFile[23] = "rfoot 5.94074 -2.3791";
		poseFile[24] = "rtoes -4.19363";
		poseFile[25] = "lfemur -35.1821 7.9344 -8.70375";
		poseFile[26] = "ltibia 71.9979";
		poseFile[27] = "lfoot 10.0395 -10.6905";
		poseFile[28] = "ltoes -4.94233";
		break;
	}
	animPose = (float**)malloc(sizeof(float*) *32);
	for (j = 0; j < 29; j++){
		char const* poseLine = poseFile[j];
		int num = sscanf(poseLine, "%s %f %f %f %f %f %f", name, &v1, &v2, &v3, &v4, &v5, &v6);

		int bone = 0;
		//Iterate until bone name is the same as the name read in
		for (bone = 0; strcmp(root[bone].name, name); bone++){}
		//Bit unpacking to figure out bone degrees of freedom
		df = (root[bone].dof&DOF_RX);
		df += (root[bone].dof&DOF_RY);
		df += (root[bone].dof&DOF_RZ);
		df += (root[bone].dof&DOF_ROOT);
		*(animPose + bone) = (float*)malloc(sizeof(float) *10);
		malloc_crash(*(animPose + bone));

		//REMINDER TO SELF: USE VECTORS NEXT TIME
		switch(num){
			//Only ever the root bone
			case 7: *(*(animPose + bone)+0) = v4;
				*(*(animPose + bone)+1) = v5;
				*(*(animPose + bone)+2) = v6;
				*(*(animPose + bone)+3) = v1;
				*(*(animPose + bone)+4) = v2;
				*(*(animPose + bone)+5) = v3;
			//A joint with 3 degrees of freedom doesn't need special consideration
			break;
			case 4: *(*(animPose+ bone)+0) = v1;
				*(*(animPose + bone)+1) = v2;
				*(*(animPose + bone)+2) = v3;
				*(*(animPose + bone)+3) = 0;
				*(*(animPose + bone)+4) = 0;
				*(*(animPose + bone)+5) = 0;
			//Depending on the bitpacked value for Degrees of Freedom, different angles need to be read
			break;
			case 3: 
			if (df == 3){
				*(*(animPose + bone)+0) = v1;
				*(*(animPose + bone)+1) = v2;
				*(*(animPose + bone)+2) = 0;
				*(*(animPose + bone)+3) = 0;
				*(*(animPose + bone)+4) = 0;
				*(*(animPose + bone)+5) = 0;
			} else if (df == 5){
				*(*(animPose + bone)+0) = v1;
				*(*(animPose + bone)+1) = 0;
				*(*(animPose + bone)+2) = v2;
				*(*(animPose + bone)+3) = 0;
				*(*(animPose + bone)+4) = 0;
				*(*(animPose + bone)+5) = 0;
			} else if (df == 6){
				*(*(animPose + bone)+0) = 0;
				*(*(animPose + bone)+1) = v1;
				*(*(animPose + bone)+2) = v2;
				*(*(animPose + bone)+3) = 0;
				*(*(animPose + bone)+4) = 0;
				*(*(animPose + bone)+5) = 0;
			}
			break;
			case 2: 
			if (df == 1){
				*(*(animPose + bone)+0) = v1;
				*(*(animPose + bone)+1) = 0;
				*(*(animPose + bone)+2) = 0;
				*(*(animPose + bone)+3) = 0;
				*(*(animPose + bone)+4) = 0;
				*(*(animPose + bone)+5) = 0;
			} else if (df == 2){
				*(*(animPose + bone)+0) = 0;
				*(*(animPose + bone)+1) = v1;
				*(*(animPose + bone)+2) = 0;
				*(*(animPose + bone)+3) = 0;
				*(*(animPose + bone)+4) = 0;
				*(*(animPose + bone)+5) = 0;
			} else if (df == 4){
				*(*(animPose + bone)+0) = 0;
				*(*(animPose + bone)+1) = 0;
				*(*(animPose + bone)+2) = v1;
				*(*(animPose + bone)+3) = 0;
				*(*(animPose + bone)+4) = 0;
				*(*(animPose + bone)+5) = 0;
			}
			break;
			default: printf("This is going seriously wrong %d %s\n", num, name);
			break;
		}
	}
}

void Skeleton::setRot(int bone, float rotx, float roty, float rotz){
	root[bone].currentRotationx = rotx;
	root[bone].currentRotationy = roty;
	root[bone].currentRotationz = rotz;
}

void Skeleton::setTrans(float trax, float tray, float traz){
	root[0].currentTranslatex = trax;
	root[0].currentTranslatey = tray;
	root[0].currentTranslatez = traz;
}

//This method is the same as above, should've condensed into one method that
//could work for both
void Skeleton::readACMHeading(FILE* file, int frame){
	int i, j, q, df;
	float v1, v2, v3, v4, v5, v6;
	char name[200];
	char* temp = new char[buffSize];
	char* line = new char[buffSize];
	while ((line = fgets(temp, buffSize, file))){
 		if (*line == ':') continue;
		if (*line == '#') continue;
		if (line == NULL) {
			printf("Something serious going on here\n");
			break;
		}
		for (i = 0; i < frame; i++){
			*(animRot + i) = (float**)malloc(sizeof(float*) *32);
			malloc_crash(*(animRot + i));

			for (j = 0; j < 29; j++){
		 		line = fgets(temp, buffSize, file);
				int num = sscanf(line, "%s %f %f %f %f %f %f", name, &v1, &v2, &v3, &v4, &v5, &v6);

				int bone = 0;
				for (bone = 0; strcmp(root[bone].name, name); bone++){}
				df = (root[bone].dof&DOF_RX);
				df += (root[bone].dof&DOF_RY);
				df += (root[bone].dof&DOF_RZ);
				df += (root[bone].dof&DOF_ROOT);
				*(*(animRot + i) + bone) = (float*)malloc(sizeof(float) *10);
				malloc_crash(*(*(animRot + i) + bone));

				switch(num){
					case 7: *(*(*(animRot + i) + bone)+0) = v4;
					*(*(*(animRot + i) + bone)+1) = v5;
					*(*(*(animRot + i) + bone)+2) = v6;
					*(*(*(animRot + i) + bone)+3) = v1;
					*(*(*(animRot + i) + bone)+4) = v2;
					*(*(*(animRot + i) + bone)+5) = v3;
					break;
					case 4: *(*(*(animRot + i) + bone)+0) = v1;
					*(*(*(animRot + i) + bone)+1) = v2;
					*(*(*(animRot + i) + bone)+2) = v3;
					*(*(*(animRot + i) + bone)+3) = 0;
					*(*(*(animRot + i) + bone)+4) = 0;
					*(*(*(animRot + i) + bone)+5) = 0;
					break;
					case 3: 
					if (df == 3){
						*(*(*(animRot + i) + bone)+0) = v1;
						*(*(*(animRot + i) + bone)+1) = v2;
						*(*(*(animRot + i) + bone)+2) = 0;
						*(*(*(animRot + i) + bone)+3) = 0;
						*(*(*(animRot + i) + bone)+4) = 0;
						*(*(*(animRot + i) + bone)+5) = 0;
					} else if (df == 5){
						*(*(*(animRot + i) + bone)+0) = v1;
						*(*(*(animRot + i) + bone)+1) = 0;
						*(*(*(animRot + i) + bone)+2) = v2;
						*(*(*(animRot + i) + bone)+3) = 0;
						*(*(*(animRot + i) + bone)+4) = 0;
						*(*(*(animRot + i) + bone)+5) = 0;
					} else if (df == 6){
						*(*(*(animRot + i) + bone)+0) = 0;
						*(*(*(animRot + i) + bone)+1) = v1;
						*(*(*(animRot + i) + bone)+2) = v2;
						*(*(*(animRot + i) + bone)+3) = 0;
						*(*(*(animRot + i) + bone)+4) = 0;
						*(*(*(animRot + i) + bone)+5) = 0;
					}
					break;
					case 2: 
					if (df == 1){
						*(*(*(animRot + i) + bone)+0) = v1;
						*(*(*(animRot + i) + bone)+1) = 0;
						*(*(*(animRot + i) + bone)+2) = 0;
						*(*(*(animRot + i) + bone)+3) = 0;
						*(*(*(animRot + i) + bone)+4) = 0;
						*(*(*(animRot + i) + bone)+5) = 0;
					} else if (df == 2){
						*(*(*(animRot + i) + bone)+0) = 0;
						*(*(*(animRot + i) + bone)+1) = v1;
						*(*(*(animRot + i) + bone)+2) = 0;
						*(*(*(animRot + i) + bone)+3) = 0;
						*(*(*(animRot + i) + bone)+4) = 0;
						*(*(*(animRot + i) + bone)+5) = 0;
					} else if (df == 4){
						*(*(*(animRot + i) + bone)+0) = 0;
						*(*(*(animRot + i) + bone)+1) = 0;
						*(*(*(animRot + i) + bone)+2) = v1;
						*(*(*(animRot + i) + bone)+3) = 0;
						*(*(*(animRot + i) + bone)+4) = 0;
						*(*(*(animRot + i) + bone)+5) = 0;
					}
					break;
					default: printf("This is going seriously wrong %d %s\n", num, name);
					break;
				}
			}
			line = fgets(temp, buffSize, file);
			q++;
		}
	}
	cout << "Done reading AMC" << endl;
}

/**
 * Trim the current string, by adding a null character into where the comments start
 */
 void Skeleton::decomment(char * buff) {
 	char* comStart = strchr(buff, '#');
 	if (comStart != NULL) {
		//remove irrelevant part of string
 		*comStart = '\0';
 	}
 }

 void Skeleton::readHeading(char* buff, FILE* file) {
 	char* temp = buff;
 	decomment(buff);
 	trim(&temp);

 	char head[50];
 	char rest[200];
 	int num = sscanf(temp, ":%s %s", head, rest);
 	if (num == 0) {
 		printf("Could not get heading name from %s, all is lost\n", temp);
 		exit(EXIT_FAILURE);
 	}
 	if (strcmp(head, "version") == 0) {
		//version string - must be 1.10
 		char* version = rest;
 		if (num != 2) {
 			char *p = { '\0' };
 			while (strlen(p) == 0) {
 				char* p = fgets(buff, buffSize, file);
 				decomment(p);
 				trim(&p);
 				version = p;
 			}
 		}
 		if (strcmp(version, "1.10") != 0) {
 			printf("Invalid version: %s, must be 1.10\n", version);
 			exit(EXIT_FAILURE);
 		}
		//Finished reading version so read the next thing?
 	} else if (strcmp(head, "name") == 0) {
		//This allows the skeleton to be called something
		//other than the file name
		//We don't actually care what the name is, so can
		//ignore this whole section

 	} else if (strcmp(head, "documentation") == 0) {
		//Documentation section has no meaningful information
		//only of use if you want to copy the file. So we skip it
 	} else if (strcmp(head, "units") == 0) {
		//Has factors for the units
		//To be able to model the real person,
		//these must be parsed correctly
		//Only really need to check if deg or rad, but even
		//that is probably not needed for the core or extension
 	} else if (strcmp(head, "root") == 0) {
		//Read in information about root
		//Or be lazy and just assume it is going to be the normal CMU thing!
 	} else if (strcmp(head, "bonedata") == 0) {
		//Description of each bone
		//This does need to actually be read :(
 		char *p;
 		while ((p = fgets(buff, buffSize, file)) != NULL) {
 			decomment(p);
 			trim(&p);
 			if (strlen(p) > 0) {
 				if (p[0] == ':') {
 					return readHeading(buff, file);
 				} else if (strcmp(p, "begin") != 0) {
 					printf("Expected begin for bone data %d, found \"%s\"", numBones, p);
 					exit(EXIT_FAILURE);
 				} else {
 					readBone(buff, file);
 					numBones++;
 				}

 			}
 		}
 	} else if (strcmp(head, "hierarchy") == 0) {
		//Description of how the bones fit together
 		char *p;
 		while ((p = fgets(buff, buffSize, file)) != NULL) {
 			trim(&p);
 			decomment(p);
 			if (strlen(p) > 0) {
 				if (p[0] == ':') {
 					return readHeading(buff, file);
 				} else if (strcmp(p, "begin") != 0) {
 					printf("Expected begin in hierarchy, found %s", p);
 					exit(EXIT_FAILURE);
 				} else {
 					readHierarchy(buff, file);
 				}

 			}
 		}
 	} else {
 		printf("Unknown heading %s\n", head);
 	}

 }

 void Skeleton::readHierarchy(char* buff, FILE* file) {
 	char *p;
 	char t1[200];
 	while ((p = fgets(buff, buffSize, file)) != NULL) {
 		trim(&p);
 		decomment(p);
 		if (strlen(p) > 0) {
 			if (strcmp(p, "end") == 0) {
				//end of hierarchy
 				return;
 			} else {
				//Read the root node
 				sscanf(p, "%s ", t1);
 				bone* rootBone = NULL;
 				for (int i = 0; i < numBones; i++) {
 					if (strcmp(root[i].name, t1) == 0) {
 						rootBone = root + i;
 						break;
 					}
 				}
				//Read the connections
 				p += strlen(t1);
 				bone* other = NULL;
 				while (*p != '\0') {
 					sscanf(p, "%s ", t1);

 					for (int i = 0; i < numBones; i++) {
 						if (strcmp(root[i].name, t1) == 0) {
 							other = root + i;
 							break;
 						}
 					}
 					if (other == NULL) {
 						printf("Unknown bone %s found in hierarchy. Failure", t1);
 						exit(EXIT_FAILURE);
 					}
 					rootBone->children[rootBone->numChildren] = other;
 					rootBone->numChildren++;
 					p += strlen(t1) + 1;

 				}
 			}
 		}

 	}
 }

 void Skeleton::readBone(char* buff, FILE* file) {
 	char *p;
 	char t1[50];
 	while ((p = fgets(buff, buffSize, file)) != NULL) {
 		trim(&p);
 		decomment(p);
 		if (strlen(p) > 0) {
 			if (strcmp(p, "end") == 0) {
				//end of this bone
 				return;
 			} else {
 				sscanf(p, "%s ", t1);
 				if (strcmp(t1, "name") == 0) {
					//Read the name and actually remember it
 					char* name = (char*) malloc(sizeof(char) * 10);
 					sscanf(p, "name %s", name);
 					root[numBones].name = name;
 				} else if (strcmp(t1, "direction") == 0) {
					//Also actually important
 					float x, y, z;
 					sscanf(p, "direction %f %f %f", &x, &y, &z);
 					root[numBones].dirx = x;
 					root[numBones].diry = y;
 					root[numBones].dirz = z;
 				} else if (strcmp(t1, "length") == 0) {
					//Also actually important
 					float len;
 					sscanf(p, "length %f", &len);
 					root[numBones].length = len;
 				} else if (strcmp(t1, "dof") == 0) {
					//Read the degrees of freedom!
 					char d1[5];
 					char d2[5];
 					char d3[5];
 					int num = sscanf(p, "dof %s %s %s", d1, d2, d3);
 					switch (num) {
 						DOF dof;
 						case 3:
 						dof = dofFromString(d3);
 						root[numBones].dof = root[numBones].dof | dof;
					//fallthrough!!
					/* no break */
 						case 2:
 						dof = dofFromString(d2);
 						root[numBones].dof = root[numBones].dof | dof;
					//fallthrough!!
					/* no break */
 						case 1:
 						dof = dofFromString(d1);
 						root[numBones].dof = root[numBones].dof | dof;
 						break;
 					}
 				} else if (strcmp(t1, "axis") == 0) {
					//Read the rotation axis
 					float x, y, z;
 					int num = sscanf(p, "axis %f %f %f XYZ", &x, &y, &z);
 					if (num != 3) {
 						printf("axis format doesn't match expected\n");
 						printf("Expected: axis %%f %%f %%f XYZ\n");
 						printf("Got: %s", p);
 						exit(EXIT_FAILURE);
 					}
 					root[numBones].rotx = x;
 					root[numBones].roty = y;
 					root[numBones].rotz = z;
 				}
				//There are more things but they are not needed for the core
 			}

 		}
 	}
 }

/**
 * Helper function to turn a DOF from the AMC file into the correct DOF value
 */
 DOF Skeleton::dofFromString(char* s) {
 	if (strcmp(s, "rx") == 0)
 		return DOF_RX;
 	if (strcmp(s, "ry") == 0)
 		return DOF_RY;
 	if (strcmp(s, "rz") == 0)
 		return DOF_RZ;
 	printf("Unknown DOF found: %s", s);
 	return DOF_NONE;
 }

/*
 * Remove leading and trailing whitespace. Increments the
 * pointer until it points to a non whitespace char
 * and then adds nulls to the end until it has no
 * whitespace on the end
 */
 void trim(char **p) {
 	if (p == NULL) {
 		printf("File terminated without version number!\n");
 		exit(EXIT_FAILURE);
 	}

	//Remove leading whitespace
 	while (**p == ' ' || **p == '\t') {
 		(*p)++;
 	}

 	int len = strlen(*p);
 	while (len > 0) {
 		char last = (*p)[len - 1];
 		if (last == '\r' || last == '\n' || last == ' ' || last == '\t') {
 			(*p)[--len] = '\0';
 		} else {
 			return;
 		}
 	}
 }
