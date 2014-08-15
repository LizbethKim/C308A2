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
	if (rS < 4){
		glPushMatrix();
		glTranslatef(root->currentTranslatex, root->currentTranslatey, root->currentTranslatez);

		glRotatef(theta, -root->diry, root->dirx, 0);
		glRotatef(root->rotz, 0, 0, 1.0);
		glRotatef(root->roty, 0, 1.0, 0);
		glRotatef(root->rotx, 1.0, 0, 0);

		glRotatef(root->currentRotationz, 0, 0, 1.0);
		glRotatef(root->currentRotationy, 0, 1.0, 0);
		glRotatef(root->currentRotationx, 1.0, 0, 0);
		
		glRotatef(-root->rotx, 1.0, 0, 0);
		glRotatef(-root->roty, 0, 1.0, 0);
		glRotatef(-root->rotz, 0, 0, 1.0);
		

		if (rS < 3){

			glColor3f(0, 1, 1);
			glutSolidSphere(0.4, 100, 100);
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
		glRotatef(-theta, -root->diry, root->dirx, 0);
	}

	theta = acos(root->dirz) * 180 / M_PI;

	glRotatef(theta, -root->diry, root->dirx, 0);

	glColor3f(1,1,1);
	gluCylinder(q, 0.2, 0.2, root->length, 50, 50);
	glRotatef(-theta, -root->diry, root->dirx, 0);
	glTranslatef(root->dirx*root->length, root->diry*root->length, root->dirz*root->length);
	int i;
	for (i = 0; i < root->numChildren; i++){
		glPushMatrix();
		display(root->children[i], q);
		glPopMatrix();
	}
	glPopMatrix();
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
		printf("STOP BEING A MISERABLE FAILURE AND PUT IN A VALID ACM FILE %s\n", filename);
		exit(EXIT_FAILURE);
	}

	char* buff = new char[buffSize];
	char *p;
	int frame = 0;

	printf("Starting reading acm file\n");
	while ((p = fgets(buff, buffSize, file)) != NULL){
		char* start = buff;
		trim(&start);

		if (buff[0] == '#' || buff[0] == '\0' || buff[0] == ':'){
			continue;
		}

		if(sscanf(start, "%d\b", &frame)){}
	}
	printf("%d\n", frame);
	if (fseek(file, 0L, SEEK_SET ) == 0){
		animRot = (float***)malloc(sizeof(float*)*frame);
		if (animRot == NULL){
			printf("Memory Allocation FAILED");
		}
		readACMHeading(file, frame);
	} else {
		printf("Return to start of file failed\n");
		return 0;
	}
	numFrames = frame;
	return frame;
}

void Skeleton::animate(int i){
	animDisplay(root, i);
	glutPostRedisplay();
}

void Skeleton::animDisplay(bone* root, int currFrame) {
	if (root == NULL) {
		return;
	}
	float transX = animRot[currFrame][0][3];
	float transY = animRot[currFrame][0][4];
	float transZ = animRot[currFrame][0][5];

	int i;

	root[0].currentTranslatex = transX;
	root[0].currentTranslatey = transY;
	root[0].currentTranslatez = transZ;
	for (i = 0; i < 29; i++){
		if (animRot[currFrame][i] == NULL) continue;
		root[i].currentRotationx = animRot[currFrame][i][0];
		root[i].currentRotationy = animRot[currFrame][i][1];
		root[i].currentRotationz = animRot[currFrame][i][2];
	}
}

void Skeleton::readACMHeading(FILE* file, int frame){
	int i, j, q, df;
	float v1, v2, v3, v4, v5, v6;
	char* name = new char[buffSize];
	char* temp = new char[buffSize];
	char* line;
	while ((line = fgets(temp, buffSize, file))){
 		if (*line == ':') continue;
		if (*line == '#') continue;
		if (line == NULL) {
			printf("Some serious shit going on here\n");
			break;
		}
		for (i = 0; i < frame; i++){
			*(animRot + i) = (float**)malloc(sizeof(float*) *29);
			for (j = 0; j < 29; j++){
				df = (root[j].dof&DOF_RX);
				df += (root[j].dof&DOF_RY);
				df += (root[j].dof&DOF_RZ);
				df += (root[j].dof&DOF_ROOT);
				cout << root[j].name << " " << df << endl;
		 		line = fgets(temp, buffSize, file);
				*(*(animRot + i) + j) = (float*)malloc(sizeof(float) *6);
				int num = sscanf(line, "%s %f %f %f %f %f %f", name, &v1, &v2, &v3, &v4, &v5, &v6);
				switch(num){
					case 7: *(*(*(animRot + i) + j)+0) = v4;
					*(*(*(animRot + i) + j)+1) = v5;
					*(*(*(animRot + i) + j)+2) = v6;
					*(*(*(animRot + i) + j)+3) = v1;
					*(*(*(animRot + i) + j)+4) = v2;
					*(*(*(animRot + i) + j)+5) = v3;
					break;
					case 4: *(*(*(animRot + i) + j)+0) = v1;
					*(*(*(animRot + i) + j)+1) = v2;
					*(*(*(animRot + i) + j)+2) = v3;
					*(*(*(animRot + i) + j)+3) = 0;
					*(*(*(animRot + i) + j)+4) = 0;
					*(*(*(animRot + i) + j)+5) = 0;
					break;
					case 3: 
					if (df == 3){
						*(*(*(animRot + i) + j)+0) = v1;
						*(*(*(animRot + i) + j)+1) = v2;
						*(*(*(animRot + i) + j)+2) = 0;
						*(*(*(animRot + i) + j)+3) = 0;
						*(*(*(animRot + i) + j)+4) = 0;
						*(*(*(animRot + i) + j)+5) = 0;
					} else if (df == 5){
						*(*(*(animRot + i) + j)+0) = v1;
						*(*(*(animRot + i) + j)+1) = 0;
						*(*(*(animRot + i) + j)+2) = v2;
						*(*(*(animRot + i) + j)+3) = 0;
						*(*(*(animRot + i) + j)+4) = 0;
						*(*(*(animRot + i) + j)+5) = 0;
					} else if (df == 6){
						*(*(*(animRot + i) + j)+0) = 0;
						*(*(*(animRot + i) + j)+1) = v1;
						*(*(*(animRot + i) + j)+2) = v2;
						*(*(*(animRot + i) + j)+3) = 0;
						*(*(*(animRot + i) + j)+4) = 0;
						*(*(*(animRot + i) + j)+5) = 0;
					}
					break;
					case 2: 
					if (df == 1){
						*(*(*(animRot + i) + j)+0) = v1;
						*(*(*(animRot + i) + j)+1) = 0;
						*(*(*(animRot + i) + j)+2) = 0;
						*(*(*(animRot + i) + j)+3) = 0;
						*(*(*(animRot + i) + j)+4) = 0;
						*(*(*(animRot + i) + j)+5) = 0;
					} else if (df == 2){
						*(*(*(animRot + i) + j)+0) = 0;
						*(*(*(animRot + i) + j)+1) = v1;
						*(*(*(animRot + i) + j)+2) = 0;
						*(*(*(animRot + i) + j)+3) = 0;
						*(*(*(animRot + i) + j)+4) = 0;
						*(*(*(animRot + i) + j)+5) = 0;
					} else if (df == 4){
						*(*(*(animRot + i) + j)+0) = 0;
						*(*(*(animRot + i) + j)+1) = 0;
						*(*(*(animRot + i) + j)+2) = v1;
						*(*(*(animRot + i) + j)+3) = 0;
						*(*(*(animRot + i) + j)+4) = 0;
						*(*(*(animRot + i) + j)+5) = 0;
					}
					break;
					default: printf("This shit is going seriously wrong %d %s\n", num, name);
					break;
				}
			}
			line = fgets(temp, buffSize, file);
			q++;
		}
	}
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
