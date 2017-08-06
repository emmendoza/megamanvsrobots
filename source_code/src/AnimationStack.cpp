#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include "DrawUtils.h"
#include "AnimationStack.h"
using namespace std;

AnimationStack::AnimationStack(const char* image_i, int moment_i, bool loop) {
	this->total_time = moment_i;
	add(image_i, moment_i);
	this->loop = loop;
}

void AnimationStack::add(const char* image_i, int moment_i) {
	this->total_time += moment_i;
	GLuint sprite = glTexImageTGAFile( image_i, &sizeX, &sizeY);
	animation.push_back(sprite);
	moment.push_back(moment_i);
	animationStack_size++;
}

int AnimationStack::nextFrame(int previousFrame) {
	if (previousFrame < animationStack_size -1) {
		return ++previousFrame;
	}
	else {
		return 0; //loop animation
	}
}




/*
void AnimationStack::add(const char* image_i, int moment_i) {
GLuint sprite = glTexImageTGAFile( image_i, &sizeX, &sizeY);
animation.push_back(sprite);
moment.push_back(moment_i);
}

void AnimationStack::current_animation(GLuint &current_sprite, int &time) {
for (int i = 0; i < animation.size(); i++) {
if (time >= moment[animation.size() - 1])
time = 0;//reset time to re-loop animation from the beginning
if (time == moment[i]) {
current_sprite = animation[i];
return;
}
}
}

GLuint AnimationStack::init_sprite() {
return animation[0];//at least 1 sprite animation should always exist...
}

*/