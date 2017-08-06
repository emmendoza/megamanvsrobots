#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <map> //used by Animations global function
#include <cmath>
#include "DrawUtils.h"
#include "AnimationStack.h"

using namespace std;

// Set this to true to make the game loop exit.
char shouldExit = 0;

// The previous frame's keyboard state.
unsigned char kbPrevState[SDL_NUM_SCANCODES] = { 0 };

// The current frame's keyboard state.
const unsigned char* kbState = NULL;

// Enum Constants
enum Axes { X, Y };
enum Action { Initial, Standing, Running, Change, Closed, Depart,Open, Arrival};
enum Direction { Left, Right, Up, Down };
enum PorNPC { PLAYER, NPC };

// Game Constants
const float cameraSize_X = 800;
const float cameraSize_Y = 600;
const float worldSize_X = 1600;// exact bg width tile size
const float worldSize_Y = 800;// exact bg width tile size


							  // THESE SHOULD BE DECLARED WITHIN THE OBJECT //
							  // Custom configuration constants
float player_speed = 0.20;
float diagonal_fudge_factor = 0.707;
float enemy_speed = 0.125;


// Time Constants
float currentTime, lastTime, deltaT;


//The music that will be played
static const char *MY_COOL_MP3 = "sound/TetrinetThemeSong.wav";
Mix_Music *music_wav = NULL;

//The sound effects that will be used
Mix_Chunk *enemyShoot_wav = NULL;
Mix_Chunk *enemyDmgd_wav = NULL;
Mix_Chunk *playerShoot_wav = NULL;
Mix_Chunk *playerDmgd_wav = NULL;
Mix_Chunk *explosion_wav = NULL;
Mix_Chunk *gateopen_wav = NULL;
Mix_Chunk *teleport_wav = NULL;
Mix_Chunk *lifeboost_wav = NULL;
Mix_Chunk *gameover_wav = NULL;
Mix_Chunk *victory_wav = NULL;

class Camera {
private:
	float xMax_;
	float yMax_;
public:
	Camera( float xMax_i, float yMax_i);
	float xMax() { return (x + xMax_); }
	float yMax() { return (y + yMax_); }
	float x = 9;
	float y = 9;
};
Camera::Camera(float xMax_i, float yMax_i) {
	xMax_ = xMax_i;
	yMax_ = yMax_i;
}

Camera camera( cameraSize_X, cameraSize_Y );

// Used by NPC to move to a future location
struct Location {
	float x_pos;
	float y_pos;
};

/*
struct BG_Tile {
	GLuint tile;
	int x_size;
	int y_size;
};
*/
class BG_Tile {
public:
	BG_Tile(const char* image, bool collidable);
	bool isCollidable() { return collidable; }
	int getSize() { return x_size; }
	int getTGA() { return tile; }
	void glDrawTile(int x, int y) { glDrawSprite(tile, x, y, x_size, y_size); }
private:
	GLuint tile;
	bool collidable;
	int x_size;
	int y_size;
};
BG_Tile::BG_Tile(const char* image, bool collidable) {
	this->tile = glTexImageTGAFile(image, &this->x_size, &this->y_size);
	this->collidable = collidable;
}

class Background_Controller {
public:
	Background_Controller();
	void drawBackground();
	bool checkTileCollision(int x, int y);
	void switchLevel();
private:
	map <int, BG_Tile*> bg_tile;
	int tile_size;
	int width = 50;
	int height = 25;
	int previous_level;
	int bg_index[25][50] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,0,0,0,0,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,0,0,0 },
		{ 0,0,0,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,0,0,0,0,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,0,0,0 },
		{ 0,0,0,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,1,0,0,0,0,1,1,1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};
	int level2[25][50] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,0,0,0 },
		{ 0,0,0,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,0,0,0 },
		{ 0,0,0,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};
	int level3[25][50] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};
	int level4[25][50] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,0,0,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,1,0,0,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,1,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,0,1,2,2,1,1,0,1,2,2,2,1,0,1,1,2,2,1,1,0,1,2,2,1,0,1,1,2,2,1,1,0,1,2,2,2,1,0,1,1,2,2,2,0,0,0,0 },
		{ 0,0,0,0,1,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,1,1,1,1,2,2,1,1,1,1,2,2,1,1,1,1,2,2,2,1,1,1,1,2,2,2,0,0,0,0 },
		{ 0,0,0,0,1,2,2,1,1,0,1,2,2,2,1,0,1,1,2,2,1,1,0,1,2,2,1,0,1,1,2,2,1,1,0,1,2,2,2,1,0,1,1,2,2,2,0,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0 },
		{ 0,0,1,1,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,0,0,1,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,0,0,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};
};

Background_Controller::Background_Controller() {
	this->previous_level = 1;
	bool collidable = true;
	bool NOTcollidable = false;

	BG_Tile* black_tile = new BG_Tile("imgs/bg/black.tga", collidable);
	this->tile_size = black_tile->getSize();
	bg_tile[0] = black_tile;

	BG_Tile* platform_tile = new BG_Tile("imgs/bg/platform.tga", NOTcollidable);
	bg_tile[1] = platform_tile;

	BG_Tile* vent_tile = new BG_Tile("imgs/bg/vent.tga", NOTcollidable);
	bg_tile[2] = vent_tile;
}
void Background_Controller::switchLevel() {
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			if (this->previous_level == 1) {
				bg_index[h][w] = level2[h][w];
			}
			else if (this->previous_level == 2) {
				bg_index[h][w] = level3[h][w];
			}
			else if (this->previous_level == 3) {
				bg_index[h][w] = level4[h][w];
			}
		}
	}
	this->previous_level++;
}

void Background_Controller::drawBackground() {

	for (int x = floor(camera.x / this->tile_size); x < (ceil((camera.x + cameraSize_X) / this->tile_size)); x++) {
		for (int y = floor(camera.y / this->tile_size); y < (ceil((camera.y + cameraSize_Y) / this->tile_size)); y++) {
		this->bg_tile[ this->bg_index[y][x] ]->glDrawTile( (x*this->tile_size - camera.x), (y*this->tile_size - camera.y) );
		}
	}
}
bool Background_Controller::checkTileCollision(int sprite_x, int sprite_y) {
	int xL = floor(sprite_x / this->tile_size);
	int yL = floor(sprite_y / this->tile_size);



	return this->bg_tile[this->bg_index[yL][xL]]->isCollidable();
}


bool boundaryCheck(float x_pos, float x_size, float y_pos, float y_size) {

	if (x_pos >= (camera.x - x_size) && x_pos < camera.xMax() && y_pos >= (camera.y - y_size) && y_pos < camera.yMax()) {
		return true;
	}
	else {
		return false;
	}
}


map <string, AnimationStack*> animations;

void init_Animations() {
	AnimationStack* megaMan_standing = new AnimationStack("imgs/megaman/z1.tga", 2400);
	megaMan_standing->add("imgs/megaman/z2.tga", 400);
	animations["megaMan_standing"] = megaMan_standing;


	AnimationStack* megaMan_running = new AnimationStack("imgs/megaman/r1.tga", 200);
	megaMan_running->add("imgs/megaman/r2.tga", 200);
	megaMan_running->add("imgs/megaman/r3.tga", 200);
	megaMan_running->add("imgs/megaman/r2.tga", 200);
	animations["megaMan_running"] = megaMan_running;

	AnimationStack* greenBomb_standing = new AnimationStack("imgs/greenbomb/still/s1.tga", 2400);
	animations["greenBomb_standing"] = greenBomb_standing;


	AnimationStack* greenBomb_running = new AnimationStack("imgs/greenbomb/run/r1.tga", 200);
	greenBomb_running->add("imgs/greenbomb/run/r2.tga", 200);
	greenBomb_running->add("imgs/greenbomb/run/r3.tga", 200);
	animations["greenBomb_running"] = greenBomb_running;

	AnimationStack* boss_standing = new AnimationStack("imgs/boss/still/s1.tga", 200);
	boss_standing->add("imgs/boss/still/s2.tga", 200);
	animations["boss_standing"] = boss_standing;


	AnimationStack* boss_running = new AnimationStack("imgs/boss/run/r1.tga", 200);
	boss_running->add("imgs/boss/run/r2.tga", 200);
	boss_running->add("imgs/boss/run/r3.tga", 200);
	boss_running->add("imgs/boss/run/r4.tga", 200);
	boss_running->add("imgs/boss/run/r5.tga", 200);
	animations["boss_running"] = boss_running;

	AnimationStack* redBomb_standing = new AnimationStack("imgs/redbomb/still/s1.tga", 2400);
	animations["redBomb_standing"] = redBomb_standing;

	AnimationStack* megaMan_dead = new AnimationStack("imgs/megaman/dead/d1.tga", 100000);
	animations["megaMan_dead"] = megaMan_dead;


	AnimationStack* redBomb_running = new AnimationStack("imgs/redbomb/run/r1.tga", 200);
	redBomb_running->add("imgs/redbomb/run/r2.tga", 200);
	redBomb_running->add("imgs/redbomb/run/r3.tga", 200);
	redBomb_running->add("imgs/redbomb/run/r4.tga", 200);
	redBomb_running->add("imgs/redbomb/run/r5.tga", 200);
	redBomb_running->add("imgs/redbomb/run/r6.tga", 200);
	redBomb_running->add("imgs/redbomb/run/r7.tga", 200);
	animations["redBomb_running"] = redBomb_running;

	AnimationStack* helmet_standing = new AnimationStack("imgs/helmet/still/s1.tga", 2400);
	animations["helmet_standing"] = helmet_standing;


	AnimationStack* helmet_running = new AnimationStack("imgs/helmet/run/r1.tga", 200);
	helmet_running->add("imgs/helmet/run/r2.tga", 200);
	helmet_running->add("imgs/helmet/run/r3.tga", 200);
	animations["helmet_running"] = helmet_running;

	AnimationStack* chopper_standing = new AnimationStack("imgs/purplechopper/still/s1.tga", 200);
	chopper_standing->add("imgs/purplechopper/still/s2.tga", 200);
	animations["chopper_standing"] = chopper_standing;


	AnimationStack* chopper_running = new AnimationStack("imgs/purplechopper/still/s1.tga", 200);
	chopper_running->add("imgs/purplechopper/still/s2.tga", 200);
	animations["chopper_running"] = chopper_running;

	AnimationStack* greenLegs_standing = new AnimationStack("imgs/greenlegs/still/s1.tga", 2400);
	animations["greenLegs_standing"] = greenLegs_standing;


	AnimationStack* greenLegs_running = new AnimationStack("imgs/greenlegs/run/r1.tga", 200);
	greenLegs_running->add("imgs/greenlegs/run/r2.tga", 200);
	greenLegs_running->add("imgs/greenlegs/run/r3.tga", 200);
	greenLegs_running->add("imgs/greenlegs/run/r4.tga", 200);
	animations["greenLegs_running"] = greenLegs_running;

	AnimationStack* blueLegs_standing = new AnimationStack("imgs/bluelegs/still/s1.tga", 2400);
	animations["blueLegs_standing"] = blueLegs_standing;


	AnimationStack* blueLegs_running = new AnimationStack("imgs/bluelegs/run/r1.tga", 200);
	blueLegs_running->add("imgs/bluelegs/run/r2.tga", 200);
	blueLegs_running->add("imgs/bluelegs/run/r3.tga", 200);
	blueLegs_running->add("imgs/bluelegs/run/r4.tga", 200);
	animations["blueLegs_running"] = blueLegs_running;

	AnimationStack* megaManRed_standing = new AnimationStack("imgs/megaman/red/s1.tga", 2400);
	megaManRed_standing->add("imgs/megaman/red/s2.tga", 400);
	animations["megaManRed_standing"] = megaManRed_standing;


	AnimationStack* megaManRed_running = new AnimationStack("imgs/megaman/red/r1.tga", 200);
	megaManRed_running->add("imgs/megaman/red/r2.tga", 200);
	megaManRed_running->add("imgs/megaman/red/r3.tga", 200);
	megaManRed_running->add("imgs/megaman/red/r2.tga", 200);
	animations["megaManRed_running"] = megaManRed_running;

	AnimationStack* purpleBlast = new AnimationStack("imgs/missle/atk_1.tga", 300);
	purpleBlast->add("imgs/missle/atk_2.tga", 300);
	animations["chopper_projectile"] = purpleBlast;

	AnimationStack* newBlast = new AnimationStack("imgs/missle/player/m1.tga", 300);
	newBlast->add("imgs/missle/player/m2.tga", 300);
	animations["megaMan_projectile"] = newBlast;

	AnimationStack* blueBlast = new AnimationStack("imgs/missle/blue/m1.tga", 300);
	blueBlast->add("imgs/missle/blue/m2.tga", 300);
	animations["redBomb_projectile"] = blueBlast;
	animations["greenBomb_projectile"] = blueBlast;

	AnimationStack* blackBlast = new AnimationStack("imgs/missle/black/m1.tga", 300);
	blackBlast->add("imgs/missle/black/m2.tga", 300);
	animations["boss_projectile"] = blackBlast;

	AnimationStack* rocketBlast = new AnimationStack("imgs/missle/rocket/m1.tga", 300);
	rocketBlast->add("imgs/missle/rocket/m2.tga", 300);
	animations["greenLegs_projectile"] = rocketBlast;
	animations["blueLegs_projectile"] = rocketBlast;

	AnimationStack* redBlast = new AnimationStack("imgs/missle/red/r1.tga", 300);
	redBlast->add("imgs/missle/red/r2.tga", 300);
	animations["helmet_projectile"] = redBlast;

	AnimationStack* health = new AnimationStack("imgs/health/h2.tga", 1000);
	animations["health"] = health;

	AnimationStack* explosion = new AnimationStack("imgs/explosion/e1.tga", 200, false);
	explosion->add("imgs/explosion/e2.tga", 200);
	explosion->add("imgs/explosion/e3.tga", 200);
	animations["explosion"] = explosion;

	AnimationStack* gateArrival = new AnimationStack("imgs/gate/arrived.tga", 300, false);
	gateArrival->add("imgs/gate/o3.tga", 300);
	gateArrival->add("imgs/gate/o1.tga", 300);
	gateArrival->add("imgs/gate/co2.tga", 300);
	gateArrival->add("imgs/gate/co1.tga", 300);
	gateArrival->add("imgs/gate/closed.tga", 300);
	animations["gateArrival"] = gateArrival;

	AnimationStack* gateDepart = new AnimationStack("imgs/gate/closed.tga", 300, false);
	gateDepart->add("imgs/gate/co1.tga", 300);
	gateDepart->add("imgs/gate/co2.tga", 300);
	gateDepart->add("imgs/gate/o1.tga", 300);
	gateDepart->add("imgs/gate/o3.tga", 300);
	//gateDepart->add("imgs/gate/arrived.tga", 300);
	animations["gateDepart"] = gateDepart;

	AnimationStack* gateOpen = new AnimationStack("imgs/gate/o1.tga", 300);
	gateOpen->add("imgs/gate/o2.tga", 300);
	gateOpen->add("imgs/gate/o3.tga", 300);
	gateOpen->add("imgs/gate/o4.tga", 300);
	gateOpen->add("imgs/gate/o5.tga", 300);
	animations["gateOpen"] = gateOpen;

	AnimationStack* gateClosed = new AnimationStack("imgs/gate/closed.tga", 1000);
	animations["gateClosed"] = gateClosed;
}

AnimationStack* getAnimationStack(string animation) { return animations[animation]; }



class Projectile {
public:
	Projectile(float x_pos_i, float y_pos_i, string character, Direction d_i, PorNPC pORnpc, float velocity, int dmg=25, bool bomb = false);
	void update();
	void setAnimationStack(AnimationStack* x);
	GLuint current_animation() { return current_animationStack->currentAnimation(current_frame); }
	int nextAnimation_time() { return current_animationStack->timeTillNextAnimation(current_frame); }
	void changeAction(Action x) { if (action != x) action = x; }
	void draw();
	void faceLeft() { if (direction != Left) direction = Left; }
	void faceRight() { if (direction != Right) direction = Right; }
	Direction currentDirection() { return direction; }
	int sizeX() { return current_animationStack->sizeX; }
	int sizeY() { return current_animationStack->sizeY; }
	float x_pos;
	float y_pos;
	float y_pos_pivot;

	PorNPC pORnpc;
	int dmg_() { return dmg; }
private:
	map <Action, AnimationStack*> actions;
	AnimationStack* current_animationStack;
	Direction direction;
	Action prev_action;
	Action action;

	int current_frame;
	int time;
	int timeInf;

	float velocity;
	int dmg;
	string character;
};

Projectile::Projectile(float x_pos_i, float y_pos_i, string character, Direction d_i, PorNPC pORnpc, float v_i, int dmg, bool bomb) {

	if (getAnimationStack(character + "_projectile") != NULL) {
		actions[Initial] = getAnimationStack(character + "_projectile");
		actions[Initial] = actions[Initial];
	}
	this->character = character;
	x_pos = x_pos_i, y_pos = y_pos_i, time = 0;
	y_pos_pivot = y_pos_i;
	timeInf = 0;
	current_animationStack = actions[Initial];
	current_frame = current_animationStack->initSpriteFrame();
	action = Initial;
	direction = d_i;
	if (bomb == false) {
		if (direction == Left) {
			x_pos -= sizeX();
		}
		else {
			x_pos += sizeX();
		}
	}
	
	this->pORnpc = pORnpc;
	this->velocity = v_i;
	this->dmg = dmg;
}

void Projectile::setAnimationStack(AnimationStack* xyz) {
	current_animationStack = xyz;
	current_frame = current_animationStack->initSpriteFrame();
	time = 0;
}

void Projectile::update() {
	if (action != prev_action) {
		prev_action = action;
		setAnimationStack(actions[action]);
	}

	if (deltaT) {
		time += deltaT;
		timeInf += deltaT;
		if (this->character == "boss") {
			y_pos = y_pos_pivot + 20*sin(timeInf);
		}

		if (direction == Right) {
			x_pos += velocity * deltaT;
		}
		else if (direction == Left) {
			x_pos -= velocity * deltaT;
		}

		if (time >= nextAnimation_time()) {
			current_frame = current_animationStack->nextFrame(current_frame);
			time = 0;
		}
	}

}

void Projectile::draw() {
	if (x_pos >= (camera.x - this->sizeX()) && x_pos < camera.xMax() && y_pos >= (camera.y - this->sizeY() ) && y_pos < camera.yMax()) {
		if (direction == Left) {
			glDrawSpriteL(this->current_animation(), (x_pos - camera.x), (y_pos - camera.y), this->sizeX(), this->sizeY());
		}
		else if (direction == Right) {
			glDrawSpriteR(this->current_animation(), (x_pos - camera.x), (y_pos - camera.y), this->sizeX(), this->sizeY());
		}
	}
}



class Sprite {
public:
	Sprite(float x_pos_i, float y_pos_i, string character, PorNPC pORnpc, int health = 100, bool bomb = false);
	void update();
	void setAnimationStack(AnimationStack* x);
	GLuint current_animation() { return current_animationStack->currentAnimation(current_frame); }
	int nextAnimation_time() { return current_animationStack->timeTillNextAnimation(current_frame); }
	void changeAction(Action x) { if (action != x) action = x; }
	void draw();
	void faceLeft() { if (direction != Left) direction = Left; }
	void faceRight() { if (direction != Right) direction = Right; }
	Direction currentDirection() { return direction; }
	int sizeX() { return current_animationStack->sizeX; }
	int sizeY() { return current_animationStack->sizeY; }
	float far_x_pos() { return (x_pos + sizeX()); };
	float far_y_pos() { return (y_pos + sizeY()); };
	float x_pos;
	float y_pos;

	int health;
	PorNPC pORnpc;
	int npc_time = 0;
	float npc_aggresion = 0;
	float npc_fear = 0;
	Location location;

	void set_upDown(float y) { if (upDown != y) { upDown = y; } }
	void set_leftRight(float x) { if (upDown != x) { leftRight = x; } }
	float upDown;
	float leftRight;
	bool victory;

	string character;
	bool bomb;
	bool dead;
private:
	map <Action, AnimationStack*> actions;
	AnimationStack* current_animationStack;
	Direction direction;
	Action prev_action;
	Action action;

	int current_frame;
	int time;
};

Sprite::Sprite(float x_pos_i, float y_pos_i, string character, PorNPC pORnpc, int health, bool bomb) {
	this->character = character;
	if (getAnimationStack(character + "_standing") != NULL) {
		this->actions[Standing] = getAnimationStack(character + "_standing");
		this->actions[Initial] = this->actions[Standing];
	}
	if (getAnimationStack(character + "_running") != NULL)
		actions[Running] = getAnimationStack( character + "_running");
	
	x_pos = x_pos_i, y_pos = y_pos_i, time = 0;
	current_animationStack = actions[Initial];
	current_frame = current_animationStack->initSpriteFrame();
	direction = Right;
	action = Initial;
	this->pORnpc = pORnpc;
	this->health = health;
	this->location.x_pos = x_pos_i;
	this->location.y_pos = y_pos_i;
	this->victory = false;
	this->bomb = bomb;
	this->dead = false;
}

void Sprite::setAnimationStack(AnimationStack* xyz) {
	current_animationStack = xyz;
	current_frame = current_animationStack->initSpriteFrame();
	time = 0;
}

void Sprite::update() {
	if (action != prev_action) {{
			prev_action = action;
			setAnimationStack( actions[action] );
		}
	}

	if (deltaT) {
		time += deltaT;

		if (pORnpc == NPC)
			npc_time += deltaT;

		if (time >= nextAnimation_time() ) {
			current_frame = current_animationStack->nextFrame(current_frame);
			time = 0;
		}
	}

}

void Sprite::draw() {
	if (boundaryCheck(this->x_pos, this->sizeX(), this->y_pos, this->sizeY())) {
		if (direction == Left) {
			glDrawSpriteL(this->current_animation(), (x_pos - camera.x), (y_pos - camera.y), this->sizeX(), this->sizeY());
		}
		else if (direction == Right) {
			glDrawSpriteR(this->current_animation(), (x_pos - camera.x), (y_pos - camera.y), this->sizeX(), this->sizeY());
		}
	}
}

float step(float npc, float player) {

	if (player > npc) {
		return 1;
	}
	else if (player < npc) {
		return -1;
	}
	else {
		return 0;
	}
}

float diff(float a, float b) {
	return abs(a - b);
}

void moveEnemy(Sprite *enemy_sprite, Location me_sprite) {

	if (deltaT) {
		float direction = step(enemy_sprite->x_pos, me_sprite.x_pos);
		float direction_y = step(enemy_sprite->y_pos, me_sprite.y_pos);


		if (direction > 0) {
			enemy_sprite->faceRight();
			enemy_sprite->changeAction(Running);
		}
		else if (direction < 0) {
			enemy_sprite->faceLeft();
			enemy_sprite->changeAction(Running);
		}
		else if (direction_y != 0) {
			enemy_sprite->changeAction(Running);
		}
		else {
			enemy_sprite->changeAction(Standing);
		}
		float x_dif = diff(enemy_sprite->x_pos, me_sprite.x_pos);
		float y_dif = diff(enemy_sprite->y_pos, me_sprite.y_pos);
		float x_distance2travel = (direction * enemy_speed  * deltaT);
		float y_distance2travel = (step(enemy_sprite->y_pos, me_sprite.y_pos) *enemy_speed  * deltaT);
		enemy_sprite->x_pos += (x_dif < x_distance2travel) ? x_dif : x_distance2travel;
		//enemy_sprite->x_pos += (direction * enemy_speed  * deltaT);
		//enemy_sprite->y_pos += (step(enemy_sprite->y_pos, me_sprite.y_pos) *enemy_speed  * deltaT);
		enemy_sprite->y_pos += (y_dif < y_distance2travel) ? y_dif : y_distance2travel;
	}
	
}
class Explosion {
public:
	Explosion(string x, float x_pos, float y_pos);
	void update();
	void setAnimationStack(AnimationStack* x);
	GLuint current_animation() { return current_animationStack->currentAnimation(current_frame); }
	int nextAnimation_time() { return current_animationStack->timeTillNextAnimation(current_frame); }
	void changeAction(Action x) { if (action != x) action = x; }
		void draw();
	int sizeX() { return current_animationStack->sizeX; }
	int sizeY() { return current_animationStack->sizeY; }
	float far_x_pos() { return (x_pos + sizeX()); };
	float far_y_pos() { return (y_pos + sizeY()); };
	float x_pos;
	float y_pos;
	bool end;
private:
	map <Action, AnimationStack*> actions;
	AnimationStack* current_animationStack;

	Action prev_action;
	Action action;
	bool switch_action;
	int current_frame;
	int time;
	int total_time;
};
Explosion::Explosion(string x, float x_pos, float y_pos) {
	if (getAnimationStack(x) != NULL) {
		actions[Initial] = getAnimationStack(x);
	}
	current_animationStack = actions[Initial];
	current_frame = current_animationStack->initSpriteFrame();
	action = Initial;
	this->time = 0;
	this->total_time = 0;
	this->x_pos = x_pos;
	this->y_pos = y_pos;
	this->end = false;
}
void Explosion::setAnimationStack(AnimationStack* xyz) {
	current_animationStack = xyz;
	current_frame = current_animationStack->initSpriteFrame();
	time = 0;
}
void Explosion::update() {



	if (deltaT) {
		time += deltaT;
		this->total_time += deltaT;

		if (time >= nextAnimation_time()) {
			current_frame = current_animationStack->nextFrame(current_frame);
			time = 0;
		}
		if (total_time >= current_animationStack->getTotalTime()) {
			this->end = true;
		}
	}

}
void Explosion::draw() {
	if (x_pos >= (camera.x - this->sizeX()) && x_pos < camera.xMax() && y_pos >= (camera.y - this->sizeY()) && y_pos < camera.yMax()) {
		glDrawSpriteL(this->current_animation(), (x_pos - camera.x), (y_pos - camera.y), this->sizeX(), this->sizeY());
	}
}

////////////////////////////////
class Health {
public:
	Health(string x, float x_pos, float y_pos, Sprite* player);
	void update();
	void setAnimationStack(AnimationStack* x);
	GLuint current_animation() { return current_animationStack->currentAnimation(current_frame); }
	int nextAnimation_time() { return current_animationStack->timeTillNextAnimation(current_frame); }
	void changeAction(Action x) { if (action != x) action = x; }
	void draw();
	int sizeX() { return current_animationStack->sizeX; }
	int sizeY() { return current_animationStack->sizeY; }
	float far_x_pos() { return (x_pos + sizeX()); };
	float far_y_pos() { return (y_pos + sizeY()); };
	float x_pos;
	float y_pos;
	bool end;
	Sprite* player;
private:
	map <Action, AnimationStack*> actions;
	AnimationStack* current_animationStack;

	Action prev_action;
	Action action;
	bool switch_action;
	int current_frame;
	int time;
	int total_time;
};
Health::Health(string x, float x_pos, float y_pos, Sprite* player) {
	if (getAnimationStack(x) != NULL) {
		actions[Initial] = getAnimationStack(x);
	}
	current_animationStack = actions[Initial];
	current_frame = current_animationStack->initSpriteFrame();
	action = Initial;
	this->time = 0;
	this->total_time = 0;
	this->x_pos = x_pos;
	this->y_pos = y_pos;
	this->end = false;
	this->player = player;
}
void Health::setAnimationStack(AnimationStack* xyz) {
	current_animationStack = xyz;
	current_frame = current_animationStack->initSpriteFrame();
	time = 0;
}
void Health::update() {



	if (deltaT) {
		time += deltaT;
		this->total_time += deltaT;

		if (time >= nextAnimation_time()) {
			current_frame = current_animationStack->nextFrame(current_frame);
			time = 0;
		}
		if (abs(player->x_pos - x_pos) < 10 && abs(player->y_pos - y_pos) < 10) {
			if (player->health <= 50) {
				player->health += 50;
			}
			else {
				player->health = 100;
			}
			Mix_PlayChannel(-1, lifeboost_wav, 0);
			this->end = true;
		}
	}

}
void Health::draw() {
	if (x_pos >= (camera.x - this->sizeX()) && x_pos < camera.xMax() && y_pos >= (camera.y - this->sizeY()) && y_pos < camera.yMax()) {
		glDrawSpriteL(this->current_animation(), (x_pos - camera.x), (y_pos - camera.y), this->sizeX(), this->sizeY());
	}
}

class Gate {
public:
	Gate(string x, float x_pos, float y_pos);
	void update();
	void setAnimationStack(AnimationStack* x);
	GLuint current_animation() { return current_animationStack->currentAnimation(current_frame); }
	int nextAnimation_time() { return current_animationStack->timeTillNextAnimation(current_frame); }
	void changeAction(Action x) { if (action != x) action = x; }
	void activateGate() { if (activate_gate == false) { activate_gate = true; Mix_PlayChannel(-1, gateopen_wav, 0); } }
	bool gateActivated() { return activate_gate; }
	void gateDeactivate() { activate_gate = false; }
	bool gotoNextLevel(Sprite* player);
	void draw();
	int sizeX() { return current_animationStack->sizeX; }
	int sizeY() { return current_animationStack->sizeY; }
	float far_x_pos() { return (x_pos + sizeX()); };
	float far_y_pos() { return (y_pos + sizeY()); };
	float x_pos;
	float y_pos;
private:
	map <Action, AnimationStack*> actions;
	AnimationStack* current_animationStack;

	Action prev_action;
	Action action;
	bool activate_gate;
	bool switch_action;
	int current_frame;
	int time;
	int total_time;
};
Gate::Gate(string x, float x_pos, float y_pos) {
	if (getAnimationStack(x) != NULL) {
		actions[Initial] = getAnimationStack(x);
		actions[Closed] = getAnimationStack("gateClosed");
		actions[Depart] = getAnimationStack("gateDepart");
		actions[Open] = getAnimationStack("gateOpen");
		actions[Arrival] = getAnimationStack("gateArrival");
	}
	current_animationStack = actions[Initial];
	current_frame = current_animationStack->initSpriteFrame();
	action = Initial;
	prev_action = Initial;
	this->switch_action = false;
	this->time = 0;
	this->total_time = 0;
	this->x_pos = x_pos;
	this->y_pos = y_pos;
	this->activate_gate = false;
}
void Gate::setAnimationStack(AnimationStack* xyz) {
	current_animationStack = xyz;
	current_frame = current_animationStack->initSpriteFrame();
	time = 0;
}
void Gate::update() {
	//if (action == Initial && this->switch_action == true && current_animationStack->loop == false && current_frame == 0) {
	//	changeAction(Closed);
	//}

	if (action != prev_action) {
		prev_action = action;
		setAnimationStack(actions[action]);
	}

	if (deltaT) {
		time += deltaT;

		if (current_animationStack->loop == false) {
			total_time += deltaT;
			if (total_time >= current_animationStack->getTotalTime() - 20) {
				if (action == Initial || action == Arrival) {
					changeAction(Closed);
					setAnimationStack(actions[action]);
					total_time = 0;
				}
				else if (action == Depart) {
					changeAction(Open);
					setAnimationStack(actions[action]);
					total_time = 0;
				}
				
			}
		}

		if (time >= nextAnimation_time()) {
			current_frame = current_animationStack->nextFrame(current_frame);
			time = 0;
		}


	}
	//if (switch_action == false)
	//	switch_action = true;
}
void Gate::draw() {
	if (x_pos >= (camera.x - this->sizeX()) && x_pos < camera.xMax() && y_pos >= (camera.y - this->sizeY()) && y_pos < camera.yMax()) {
		glDrawSpriteL(this->current_animation(), (x_pos - camera.x), (y_pos - camera.y), this->sizeX(), this->sizeY());
	}
}
bool Gate::gotoNextLevel(Sprite* player) {

	if (player->x_pos > this->x_pos && player->far_x_pos() < this->far_x_pos()) {
		if (player->y_pos > this->y_pos && player->far_y_pos() < this->far_y_pos()) {
			cout << "GOTO NEXT LEVEL ACTIVATED" << endl;
			return true;
		}
	}
	return false;
}

bool sortByY(const Sprite *spriteA,const Sprite *spriteB) { return spriteA->y_pos < spriteB->y_pos; }

class SpriteBucket {

public:
	SpriteBucket(Background_Controller* Backgroud_Controller, float xStart_i, float xEnd_i);
	void setPlayer1(Sprite * player1) { this->player1 = player1; }
	void addSprite(Sprite *sprite_i) { bucket.push_back(sprite_i); if (sprite_i->pORnpc == NPC) enemies++; };
	void addBlast(Projectile *sprite_i) { bucket_blast.push_back(sprite_i); }
	void addExplosions(Explosion *x) { explosions.push_back(x); }
	void addHealth(Health *x) { healths.push_back(x); }
	void collisionResolution(Sprite *sprite);
	void updateProjectiles(); 
	void drawProjectiles();
	void updateSprites();
	void drawSprites();
	void updateExplosions();
	void drawExplosions();
	void updateHealth();
	void drawHealth();
	float xStart() { return x_start; }
	float xEnd() { return x_end; }
	bool allEnemiesDead() { return (enemies == 0) ? true : false; }
	void addStartGate(Gate* start_gate) { this->start_gate = start_gate; }
	void addNextGate(Gate* next_gate) { this->next_gate = next_gate; }
	
private:
	//bool sortByY(const Sprite &spriteA,const Sprite &spriteB) { return spriteA.y_pos < spriteB.y_pos; }
	void sortBucket() { sort(bucket.begin(), bucket.end(), sortByY); }
	bool circleCollision(float sprite_x_pos, float sprite_y_pos, int sprite_w, int sprite_h, float projectile_x_pos, float projectile_y_pos, int projectile_w, int projectile_h, PorNPC sprite_pORnpc, PorNPC projectile_pORnpc);
	void NPC_Controller(Sprite * npc);

	Background_Controller* background_Controller;
	vector <Sprite*> bucket;
	vector <Projectile*> bucket_blast;
	vector <Explosion*> explosions;
	vector <Health*> healths;
	Sprite* player1;
	float x_start;
	float x_end;
	int enemies;
	Gate* start_gate;
	Gate* next_gate;
	int level;
	int bossesAlive;
};
bool SpriteBucket::circleCollision(float sprite_x_pos, float sprite_y_pos, int sprite_w, int sprite_h, float projectile_x_pos, float projectile_y_pos, int projectile_w, int projectile_h, PorNPC sprite_pORnpc, PorNPC projectile_pORnpc) {
	if (deltaT) {
		float sprite_center_x = sprite_x_pos + (sprite_w / 2);
		float sprite_center_y = sprite_y_pos + (sprite_h / 2);

		float projectile_center_x = projectile_x_pos + (projectile_w / 2);
		float projectile_center_y = projectile_y_pos + (projectile_h / 2);

		float sprite_r = (sprite_w / 4);
		float projectile_r = (projectile_w / 2);

		float SQ_dx = pow((sprite_center_x - projectile_center_x), 2);
		float SQ_dy = pow((sprite_center_y - projectile_center_y), 2);
		float SQ_dr = pow((sprite_r + projectile_r), 2);

		if ((sprite_pORnpc != projectile_pORnpc) && ((SQ_dx + SQ_dy) < (float)SQ_dr)) {
			//cout << "Collision detected!" << endl; //debugging
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}
void SpriteBucket::NPC_Controller(Sprite * npc) {

	if (deltaT) {
		if (npc->npc_time >= 600) {

			int random = rand() % 3;

			if (random == 0) {
				npc->location.y_pos = player1->y_pos;
				npc->location.x_pos = player1->x_pos;
			}
			npc->npc_time = 0;
		}
		else {
			moveEnemy(npc, npc->location);
		}

		if (player1->health > 0 && boundaryCheck(npc->x_pos, npc->sizeX(), npc->y_pos, npc->sizeY()) && (abs(player1->y_pos - npc->y_pos) < 10) && npc->npc_time > 300) {
			if (player1->x_pos < npc->x_pos) {
				npc->faceLeft();
			}
			else {
				npc->faceRight();
			}
			if (npc->bomb == false) {
				Mix_PlayChannel(-1, enemyShoot_wav, 0);
				Projectile* missle = new Projectile(npc->x_pos, npc->y_pos, npc->character, npc->currentDirection(), NPC, 0.35); //uncomment
				addBlast(missle); //uncomment
				npc->npc_time = 0;
			}
			else if (npc->bomb == true && abs(npc->x_pos - player1->x_pos) < 5) {
				Mix_PlayChannel(-1, explosion_wav, 0);
				Projectile* missle = new Projectile(npc->x_pos, npc->y_pos, npc->character, npc->currentDirection(), NPC, 0.35, 50, true); //uncomment
				addBlast(missle); 
				npc->npc_time = 0;
				npc->dead = true;
				Explosion* explosion = new Explosion("explosion",player1->x_pos,player1->y_pos);
				addExplosions(explosion);
			}
		}
	}
}

void SpriteBucket::updateProjectiles() {
	for (Projectile *projectile : bucket_blast) {
		projectile->update();
	}

}
void SpriteBucket::updateExplosions() {
	for (Explosion *explosion : explosions) {
		explosion->update();
	}

}
void SpriteBucket::drawExplosions() {

	for (int i = 0; i < explosions.size(); i++) {
		explosions.at(i)->draw();
		if (explosions.at(i)->end == true) {
			Explosion* tmp = explosions.at(i);
			explosions.at(i) = explosions.back();
			explosions.pop_back();
			delete tmp;
			i--;
		}
	}

}
void SpriteBucket::updateHealth() {
	for (Health *health : healths) {
		health->update();
	}

}
void SpriteBucket::drawHealth() {

	for (int i = 0; i < healths.size(); i++) {
		healths.at(i)->draw();
		if (healths.at(i)->end == true) {
			Health* tmp = healths.at(i);
			healths.at(i) = healths.back();
			healths.pop_back();
			delete tmp;
			i--;
		}
	}

}
void SpriteBucket::drawProjectiles() {

	for (int i = 0; i < bucket_blast.size(); i++) {
		if (boundaryCheck(bucket_blast.at(i)->x_pos, bucket_blast.at(i)->sizeX(), bucket_blast.at(i)->y_pos, bucket_blast.at(i)->sizeY())) {
			bucket_blast.at(i)->draw();
		}//projectile outside of camera view -> delete
		else {
			Projectile* tmp = bucket_blast.at(i);
			bucket_blast.at(i) = bucket_blast.back();
			bucket_blast.pop_back();
			delete tmp;
			i--;
		}
	}
	//if (deltaT)
		//printf("Projectile Bucket Size: %i\n", bucket_blast.size()); //debugging

}

void SpriteBucket::updateSprites() {
	bool hit;
	for (int j = 0; j < bucket.size(); j++) {
		for (int i = 0; i < bucket_blast.size(); i++) {

			if (bucket.at(j)->pORnpc == NPC) {
				if (bucket_blast.at(i) != NULL && bucket_blast.at(i)->currentDirection() != bucket.at(j)->currentDirection()) {
					if (bucket_blast.at(i)->y_pos - 10 < bucket.at(j)->y_pos &&  bucket_blast.at(i)->y_pos + 10 > bucket.at(j)->y_pos) {
						if (bucket.at(j)->npc_time >= 301) {

							// Dodge Attack
							int random = rand() % 2;
							if (random == 0) {
								
								bucket.at(j)->location.y_pos = bucket.at(j)->location.y_pos + 50;
							}
							else {
								
								bucket.at(j)->location.y_pos = bucket.at(j)->location.y_pos - 50;
							}
							
							// NPC Decision Making
							float current_decision = rand() / (float)RAND_MAX;
							if (current_decision < bucket.at(j)->npc_aggresion) {
								//Dodge Attack and Continue Charging
							}else if(current_decision > bucket.at(j)->npc_fear) {
								//Dodge Attack and Run Away
								if (bucket.at(j)->currentDirection() == Left) {
									bucket.at(j)->location.x_pos = bucket.at(j)->x_pos + 60;
								}
								else {
									bucket.at(j)->location.x_pos = bucket.at(j)->x_pos - 60;
								}
							}
							else {
								//Dodge Attack
								bucket.at(j)->location.x_pos = bucket.at(j)->x_pos;
							}
							bucket.at(j)->npc_time = 0;
						}
					}
				}
			}

			hit = circleCollision(bucket.at(j)->x_pos, bucket.at(j)->y_pos, bucket.at(j)->sizeX(), bucket.at(j)->sizeY(), bucket_blast.at(i)->x_pos, bucket_blast.at(i)->y_pos, bucket_blast.at(i)->sizeX(), bucket_blast.at(i)->sizeY(), bucket.at(j)->pORnpc, bucket_blast.at(i)->pORnpc);
			if (hit) {
				bucket.at(j)->health -= bucket_blast.at(i)->dmg_();
				if (bucket.at(j)->pORnpc == PLAYER) {
					Mix_PlayChannel(-1, playerDmgd_wav, 0);
					cout << "I'M HIT! - Life: " << bucket.at(j)->health << "/100" << endl;
				}
				else {
					if (bucket.at(j)->health > 0) {
						Mix_PlayChannel(-1, enemyDmgd_wav, 0);
						cout << "GOT ONE!" << endl;
					}
					else {
						Mix_PlayChannel(-1, explosion_wav, 0);
						if (bucket.at(j)->bomb == true || bucket.at(j)->character != "megaMan") {
							Explosion* explosion = new Explosion("explosion", bucket.at(j)->x_pos, bucket.at(j)->y_pos);
							addExplosions(explosion);
						}
						if (bucket.at(j)->character == "boss") {
							this->bossesAlive--;
							if (this->bossesAlive == 0) {
								Mix_HaltMusic();
								Mix_PlayChannel(-1, victory_wav, 0);
								player1->victory = true;
							}
						}
						cout << "KILLED ONE!" << endl;
					}
				}
				Projectile* tmp = bucket_blast.at(i);
				bucket_blast.at(i) = bucket_blast.back();
				bucket_blast.pop_back();
				delete tmp;
				i--;
			}


		}
		/*
		if (bucket.at(j)->health <= 0) {
			Sprite* tmp2 = bucket.at(j);
			bucket.at(j) = bucket.back();
			bucket.pop_back();
			// Temporary fix - Player1 Died
			if (tmp2->pORnpc == PLAYER)
				cout << "GAME OVER!" << endl;
			delete tmp2;
			j--;
		}*/
	}
	for (int j = 0; j < bucket.size(); j++) {
		if (bucket.at(j)->health <= 0 || bucket.at(j)->dead == true) {
			Sprite* tmp2 = bucket.at(j);
			bucket.at(j) = bucket.back();
			bucket.pop_back();
			// Temporary fix - Player1 Died
			if (tmp2->pORnpc == PLAYER) {
				Explosion* explosion = new Explosion("megaMan_dead", player1->x_pos, player1->y_pos);
				addExplosions(explosion);
				Mix_HaltMusic();
				Mix_PlayChannel(-1, gameover_wav, 0);
				cout << "GAME OVER!" << endl;
			}
			else if (tmp2->pORnpc == NPC) {
				enemies--;
			}
			delete tmp2;
			j--;
		}
	}
	for (Sprite *sprite : bucket) {
		if (sprite->pORnpc == NPC)
			NPC_Controller(sprite);

		if(sprite->character != "boss")
			this->collisionResolution(sprite);

		sprite->update();
	}


	if (this->allEnemiesDead()) {
		this->next_gate->activateGate();
		bool startNextLevel = this->next_gate->gotoNextLevel(player1);
		this->next_gate->changeAction(Depart);
		if (startNextLevel) {
			for (Health *health : healths) {
				health->end = true; //remove previous remaining health items
			}
			this->level++;
			this->next_gate->gateDeactivate();
			this->start_gate->changeAction(Arrival);
			this->next_gate->changeAction(Closed);
			Mix_PlayChannel(-1, teleport_wav, 0);
			background_Controller->switchLevel();
			player1->x_pos = 132;
			player1->y_pos = 132;
			player1->faceRight();
			camera.x = 9;
			camera.y = 9;
			if (this->level == 1) {
				for (int i = 128; i <= 628; i += 500) {
					Sprite* enemy_sprite = new Sprite(i + 352, 128, "redBomb", NPC, 100, true);
					enemy_sprite->faceLeft();
					enemy_sprite->npc_aggresion = 0.4;
					enemy_sprite->npc_fear = 0.5;
					addSprite(enemy_sprite);
					Health* health = new Health("health", i + 352, 400, this->player1);
					addHealth(health);
					Sprite* enemy_sprite2 = new Sprite(i + 352, 608, "greenBomb", NPC, 100, true);
					enemy_sprite2->faceLeft();
					enemy_sprite->npc_aggresion = 0.5;
					enemy_sprite2->npc_fear = 0.4;
					addSprite(enemy_sprite2);
				}
			}
			else if (this->level == 2) {
				for (int i = 128; i <= 628; i += 500) {
					Sprite* enemy_sprite = new Sprite(i + 352, 128, "blueLegs", NPC, 125);
					enemy_sprite->faceLeft();
					enemy_sprite->npc_aggresion = 0.4;
					enemy_sprite->npc_fear = 0.5;
					addSprite(enemy_sprite);
					Health* health = new Health("health", i + 352, 400, this->player1);
					addHealth(health);
					Sprite* enemy_sprite2 = new Sprite(i + 352, 608, "greenLegs", NPC, 125);
					enemy_sprite2->faceLeft();
					enemy_sprite->npc_aggresion = 0.5;
					enemy_sprite2->npc_fear = 0.4;
					addSprite(enemy_sprite2);
				}
			}
			else if (this->level == 3) {
				for (int i = 128; i <= 628; i += 500) {
					Sprite* enemy_sprite = new Sprite(i + 352, 128, "boss", NPC,300);
					enemy_sprite->faceLeft();
					enemy_sprite->npc_aggresion = 0.4;
					enemy_sprite->npc_fear = 0.5;
					addSprite(enemy_sprite);
					Health* health = new Health("health", i + 352, 400, this->player1);
					addHealth(health);
					Sprite* enemy_sprite2 = new Sprite(i + 352, 608, "boss", NPC,300);
					enemy_sprite2->faceLeft();
					enemy_sprite->npc_aggresion = 0.5;
					enemy_sprite2->npc_fear = 0.4;
					addSprite(enemy_sprite2);
				}
			}
		}
	}
}

void SpriteBucket::drawSprites() {
	sortBucket();
	for (Sprite *sprite : bucket) {
		if (boundaryCheck(sprite->x_pos, sprite->sizeX(), sprite->y_pos, sprite->sizeY())) {
			sprite->draw();
		}
	}
}

SpriteBucket::SpriteBucket(Background_Controller* background_Controller, float xStart_i=0, float xEnd_i=worldSize_X) {
	this->background_Controller = background_Controller;
	this->x_start = xStart_i;
	this->x_end = xEnd_i;
	this->enemies = 0;
	this->level = 0;
	this->bossesAlive = 4;
}
void SpriteBucket::collisionResolution(Sprite * sprite) {
	bool tl = background_Controller->checkTileCollision(sprite->x_pos, sprite->y_pos);
	bool tr = background_Controller->checkTileCollision(sprite->x_pos + sprite->sizeX(), sprite->y_pos);
	bool bl = background_Controller->checkTileCollision(sprite->x_pos, sprite->y_pos+sprite->sizeY());
	bool br = background_Controller->checkTileCollision(sprite->x_pos + sprite->sizeX(), sprite->y_pos + sprite->sizeY() );



	if (bl || br)
		sprite->y_pos -= player_speed*deltaT;
	if (tl || tr)
		sprite->y_pos += player_speed*deltaT;
	if (tl || bl)
		sprite->x_pos += player_speed*deltaT;
	if (tr || br)
		sprite->x_pos -= player_speed*deltaT;

	if (bl && br)
		sprite->y_pos -= player_speed*deltaT;
	if (tl && tr)
		sprite->y_pos += player_speed*deltaT;
	if (tl && bl)
		sprite->x_pos += player_speed*deltaT;
	if (tr && br)
		sprite->x_pos -= player_speed*deltaT;
	

		
	
}
/*
class SpriteMultiverse {
public:
	SpriteMultiverse(float worldSize_x, float multiverseSize_x);
	void addSprite(Sprite *sprite_i);
	void updateAndDrawSprites();
private:
	vector <SpriteBucket*> multiverse;
	float multiverseSize_X;
};

SpriteMultiverse::SpriteMultiverse(float worldSize_x, float multiverseSize_x) {
	multiverseSize_X = multiverseSize_x;
	int bucketStart_x = 0.0;
	int totalBuckets = ceil(worldSize_X / multiverseSize_x);
	for (int i = 0; i < totalBuckets; i++) {
		if (worldSize_x > multiverseSize_x) {
			worldSize_x -= multiverseSize_x;
		}else {
			multiverseSize_x = worldSize_x;
		}
		SpriteBucket world (bucketStart_x, bucketStart_x + multiverseSize_x);
		cout << "in MultiverseConstructor" << endl;
		cout << "world.xStart() = " << world.xStart() << " ; world.xEnd() = " << world.xEnd() << endl;
		cout << "world.add = " << &world << endl;
		cout << "==========================\n";
		multiverse.push_back( &world );
		bucketStart_x += multiverseSize_x;
	}
}

void SpriteMultiverse::addSprite(Sprite *sprite_i) {

	for (SpriteBucket *bucket : multiverse) {
		cout << "in addSprite > bucket for" << endl;
		cout << "bucket s: " << bucket->xStart() << "  e:" << bucket->xEnd() << endl; //debugging
		cout << "bucket.add = " << bucket << endl;
		cout << "sprite x: " << sprite_i->x_pos << "  s:" << sprite_i->sizeX() << endl; //debugging
		if (bucket->xStart() <= sprite_i->x_pos && sprite_i->x_pos <= bucket->xEnd()) {
			cout << "sprite added\n";
			//cout << "sprite.addr = " << &sprite_i << endl;
			bucket->addSprite( sprite_i );
			return;
		}
	}
}

void SpriteMultiverse::updateAndDrawSprites() {
	for (SpriteBucket *bucket : multiverse) {
		if (bucket->xStart() >= (camera.x - multiverseSize_X) && bucket->xEnd() < camera.xMax()) {
			cout << "Drawing from bucket.addr = " << bucket << endl;
			//bucket->drawSprites();
		}
	}
}
*/


/*
void moveEnemy(Sprite &enemy_sprite, Sprite me_sprite) {

	int direction = step(enemy_sprite.x_pos, me_sprite.x_pos);
	int direction_y = step(enemy_sprite.y_pos, me_sprite.y_pos);

	if (direction > 0) {
		enemy_sprite.faceRight();
		enemy_sprite.changeAction(Running);
	}
	else if (direction < 0) {
		enemy_sprite.faceLeft();
		enemy_sprite.changeAction(Running);
	}
	else if (direction_y != 0) {
		enemy_sprite.changeAction(Running);
	}
	else {
		enemy_sprite.changeAction(Standing);
	}

	enemy_sprite.x_pos += (direction * enemy_speed/2 * deltaT);
	enemy_sprite.y_pos += (step(enemy_sprite.y_pos, me_sprite.y_pos) *enemy_speed/2 * deltaT);
}
*/
class Image {
public:
	Image(const char* image){ this->image = glTexImageTGAFile(image, &this->x_size, &this->y_size); };
	int get_x_size() { return x_size; }
	int get_y_size() { return y_size; }
	int getTGA() { return image; }
	void glDrawImage(int x, int y) { glDrawSprite(image, x, y, x_size, y_size); }
	int x_pos;
	int y_pos;
private:
	GLuint image;
	int x_size;
	int y_size;

};
class Hud {
public:
	Hud(Sprite* player, Direction hud_position, float cameraSizeX, float cameraSizeY);
	void gameOverScreen();
	void drawHud();
private:
	Sprite* player;
	Direction hud_position;
	float hud_x_pos;
	float hud_y_pos;
	float hud_offset;
	Image* bluePlayer;
	Image* redPlayer;
	Image* fullHeart;
	Image* halfHeart;
	Image* emptyHeart;
	Image* brokenHeart;

	Image* gameOver;
	Image* victory;
	Image* finalMessage;
};
Hud::Hud(Sprite* player, Direction hud_position, float cameraSizeX, float cameraSizeY){
	this->bluePlayer = new Image("imgs/hud/blue.tga");
	this->redPlayer = new Image("imgs/hud/red.tga");

	this->fullHeart = new Image("imgs/hud/fh.tga");
	this->halfHeart = new Image("imgs/hud/hh.tga");
	this->emptyHeart = new Image("imgs/hud/eh.tga");
	this->brokenHeart = new Image("imgs/hud/bh.tga");

	this->gameOver = new Image("imgs/endgame/gameover.tga");
	this->gameOver->x_pos = (cameraSizeX / 2) - (this->gameOver->get_x_size() / 2);
	this->gameOver->y_pos = (cameraSizeY / 2) - (this->gameOver->get_y_size() / 2);
	this->victory = new Image("imgs/endgame/victory.tga");
	this->finalMessage = new Image("imgs/endgame/victory2.tga");
	this->victory->x_pos = (cameraSizeX / 2) - (this->victory->get_x_size() / 2);
	this->victory->y_pos = (cameraSizeY / 2) - (this->victory->get_y_size() / 2) - (this->finalMessage->get_y_size() / 2);
	this->finalMessage->x_pos = (cameraSizeX / 2) - (this->finalMessage->get_x_size() / 2);
	this->finalMessage->y_pos = (cameraSizeY / 2) - (this->victory->get_y_size() / 2) - (this->finalMessage->get_y_size() / 2)+ this->victory->get_y_size();
	if (hud_position == Left) {
		this->hud_x_pos = 0 + this->fullHeart->get_x_size();
		this->hud_y_pos = cameraSizeY - (2 * this->fullHeart->get_y_size());
	}
	else if (hud_position == Right) {
		this->hud_x_pos = cameraSizeX - (11 * this->fullHeart->get_x_size());
		this->hud_x_pos -= this->redPlayer->get_x_size();
		this->hud_y_pos = cameraSizeY - (2 * this->fullHeart->get_y_size());
	}
	this->hud_position = hud_position;
	this->player = player;
	this->hud_offset = this->fullHeart->get_x_size();

}
void Hud::gameOverScreen() {
	if (this->player->health <= 0) {
		this->gameOver->glDrawImage(this->gameOver->x_pos, this->gameOver->y_pos);
	}
	else if(this->player->victory){
		this->victory->glDrawImage(this->victory->x_pos, this->victory->y_pos);
		this->finalMessage->glDrawImage(this->finalMessage->x_pos, this->finalMessage->y_pos);
	}
}
void Hud::drawHud() {
	int hud_offset_x = 0;

	if (this->hud_position == Left) {
		this->bluePlayer->glDrawImage(this->hud_x_pos + hud_offset_x, this->hud_y_pos);
		hud_offset_x += this->bluePlayer->get_x_size();
	}
	else if (this->hud_position == Right) {
		this->redPlayer->glDrawImage(this->hud_x_pos + hud_offset_x, this->hud_y_pos);
		hud_offset_x += this->redPlayer->get_x_size();
	}
	for (int i = 0; i < 100; i += 10) {

		if (this->player->health == 100) {
			this->fullHeart->glDrawImage(this->hud_x_pos + hud_offset_x, this->hud_y_pos);
		}
		else if (this->player->health == i + 5) {
			this->halfHeart->glDrawImage(this->hud_x_pos + hud_offset_x, this->hud_y_pos);
		}
		else if (this->player->health == i+10) {
			this->brokenHeart->glDrawImage(this->hud_x_pos + hud_offset_x, this->hud_y_pos);
		}
		else if (this->player->health > i) {
			this->fullHeart->glDrawImage(this->hud_x_pos + hud_offset_x, this->hud_y_pos);
		}
		else {
			this->emptyHeart->glDrawImage(this->hud_x_pos + hud_offset_x, this->hud_y_pos);
		}
		hud_offset_x += this->hud_offset;
	}
	this->gameOverScreen();
}


int main(void)
{
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Could not initialize SDL. ErrorCode=%s\n", SDL_GetError());
		return 1;
	}

	// Initialize SDL.
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		return 1;

	//Initialize SDL_mixer
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		fprintf(stderr, "Could not initialize SDL_mixer.\n");
		return 1;
	}
	// Create the window and OpenGL context.
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_Window* window = SDL_CreateWindow(
		"SDLTemplate",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		cameraSize_X, cameraSize_Y,
		SDL_WINDOW_OPENGL);
	if (!window) {
		fprintf(stderr, "Could not create window. ErrorCode=%s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	SDL_GL_CreateContext(window);

	// Make sure we have a recent version of OpenGL.
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		fprintf(stderr, "Could not initialize glew. ErrorCode=%s\n", glewGetErrorString(glewError));
		SDL_Quit();
		return 1;
	}
	if (GLEW_VERSION_2_0) {
		fprintf(stderr, "OpenGL 2.0 or greater supported: Version=%s\n",
			glGetString(GL_VERSION));
	}
	else {
		fprintf(stderr, "OpenGL max supported version is too low.\n");
		SDL_Quit();
		return 1;
	}

	// Setup OpenGL state.
	glViewport(0, 0, cameraSize_X, cameraSize_Y);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, cameraSize_X, cameraSize_Y, 0, 0, 100);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Game initialization goes here.
	cout << "Camera Controls: A S D W" << endl;
	cout << "Player Controls: Arrow keys & Space Bar to Shoot.\n" << endl;

	init_Animations();// Initialize Animation Defs

	//Load the music
	music_wav = Mix_LoadMUS(MY_COOL_MP3);
	if (music_wav == NULL)
	{
		fprintf(stderr, "Failed to load music.\n");
		return 1;
	}

	//Load sounds
	enemyShoot_wav = Mix_LoadWAV("sound/EnemyShoot.wav");
	enemyDmgd_wav = Mix_LoadWAV("sound/EnemyDamage.wav");
	playerShoot_wav = Mix_LoadWAV("sound/MegaBuster.wav");
	playerDmgd_wav = Mix_LoadWAV("sound/MegamanDamage.wav");
	explosion_wav = Mix_LoadWAV("sound/Explosion.wav");
	gateopen_wav = Mix_LoadWAV("sound/GateOpen.wav");
	teleport_wav = Mix_LoadWAV("sound/Teleport.wav");
	lifeboost_wav = Mix_LoadWAV("sound/LifeBoost.wav");
	gameover_wav = Mix_LoadWAV("sound/GameOver.wav");
	victory_wav = Mix_LoadWAV("sound/Victory.wav");

	//PLAY MUSIC
	//Mix_PlayMusic(music_wav, 1);
	
	
	if (Mix_PlayingMusic() == 0)
	{
		//Play the music
		if (Mix_PlayMusic(music_wav, -1) == -1)
		{
			fprintf(stderr, "Failed to play music.\n");
			return 1;
		}
	}

	//BG_Tile bg;
	//bg.tile = glTexImageTGAFile("imgs/bg/bg_s.tga", &bg.x_size, &bg.y_size);
	Background_Controller background_Controller;// = new Background_Controller();

	SpriteBucket spritebucket(&background_Controller, 0, worldSize_X);
	Gate* start_gate = new Gate("gateArrival", 96, 96);
	Gate* next_gate = new Gate("gateClosed", 1376, 576);

	spritebucket.addStartGate(start_gate);
	spritebucket.addNextGate(next_gate);

	Sprite* me_sprite = new Sprite(132, 132, "megaMan", PLAYER);

	Hud* me_hud = new Hud(me_sprite, Left, cameraSize_X, cameraSize_Y);
	//MARKER
	for (int i = 128; i <= 628; i += 500) {
		Sprite* enemy_sprite = new Sprite(i + 352, 128, "chopper", NPC,100);
		enemy_sprite->faceLeft();
		enemy_sprite->npc_aggresion = 0.7;
		enemy_sprite->npc_fear = 0.1;
		spritebucket.addSprite(enemy_sprite);
		Health* health = new Health("health", i + 352, 400, me_sprite);
		spritebucket.addHealth(health);
		Sprite* enemy_sprite2 = new Sprite(i + 352, 608, "helmet", NPC,100);
		enemy_sprite2->faceLeft();
		enemy_sprite->npc_aggresion = 0.5;
		enemy_sprite2->npc_fear = 0.4;
		spritebucket.addSprite(enemy_sprite2);
	}
	//Sprite* enemy_sprite = new Sprite(700, 250, "megaMan", NPC);
	//Sprite* enemy_sprite2 = new Sprite(500, 500, "megaMan", NPC);
	//enemy_sprite->faceLeft();
	//enemy_sprite2->faceLeft();
	

	spritebucket.addSprite(me_sprite);
	spritebucket.setPlayer1(me_sprite);
	//spritebucket.addSprite(enemy_sprite);
	//spritebucket.addSprite(enemy_sprite2);
	/*
	SpriteMultiverse spriteMultiverse(worldSize_X, worldSize_X); //worldSize , multiverseSize
	spriteMultiverse.addSprite(me_sprite);
	spriteMultiverse.addSprite(enemy_sprite);
	*/

	// The game loop.
	kbState = SDL_GetKeyboardState(NULL);
	while ( ! shouldExit ) {
		assert(glGetError() == GL_NO_ERROR);
		memcpy(kbPrevState, kbState, sizeof(kbPrevState));

		// Handle OS message pump.
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				shouldExit = 1;
			}
		}

		currentTime = SDL_GetTicks();
		deltaT = (currentTime - lastTime);
		lastTime = currentTime;


		// Game logic goes here.
		if (kbState[SDL_SCANCODE_ESCAPE]) {
			shouldExit = 1;
		}
		

		// CAMERA CONTROLS //

		// Camera RIGHT
		if (kbState[SDL_SCANCODE_D] && !kbPrevState[SDL_SCANCODE_A]) {
			if (camera.x < (worldSize_X - cameraSize_X)-8) {
				camera.x += player_speed * deltaT;
			}
		}// Camera LEFT
		else if (kbState[SDL_SCANCODE_A] && !kbPrevState[SDL_SCANCODE_D]) {
			if (camera.x > 8) {
				camera.x -= player_speed * deltaT;
			}
		}

		// Camera DOWN
		if (kbState[SDL_SCANCODE_S] && !kbPrevState[SDL_SCANCODE_W]) {
			//printf("Down: %0.2f", camera.y); //debugging
			if (camera.y < (worldSize_Y - cameraSize_Y)-8) {
				camera.y += player_speed * deltaT;
			}
		}// Camera UP
		else if (kbState[SDL_SCANCODE_W] && !kbPrevState[SDL_SCANCODE_S]) {
			if (camera.y > 8) {
				camera.y -= player_speed * deltaT;
			}
		}





		if (me_sprite != NULL) {
			me_sprite->changeAction(Standing);
			me_sprite->leftRight=0;
			me_sprite->upDown=0;


			if (kbState[SDL_SCANCODE_SPACE] && !kbPrevState[SDL_SCANCODE_SPACE]) {
				Mix_PlayChannel(-1, playerShoot_wav, 0);
				Projectile* eb_sprite = new Projectile(me_sprite->x_pos, me_sprite->y_pos, me_sprite->character, me_sprite->currentDirection(), PLAYER, 0.35); //uncomment
				spritebucket.addBlast(eb_sprite); //uncomment
	
			}

			
			if (kbState[SDL_SCANCODE_RIGHT] && !kbPrevState[SDL_SCANCODE_LEFT]) {
				if (me_sprite->x_pos < (worldSize_X - me_sprite->sizeX())) {
					me_sprite->faceRight();
					me_sprite->changeAction(Running);
					me_sprite->set_leftRight(1);
					//me_sprite->x_pos += player_speed * deltaT;
					
				}
			}
			else if (kbState[SDL_SCANCODE_LEFT] && !kbPrevState[SDL_SCANCODE_RIGHT]) {
				if (me_sprite->x_pos > 0) {
					me_sprite->faceLeft();
					me_sprite->changeAction(Running);
					me_sprite->set_leftRight(-1);
					//me_sprite->x_pos -= player_speed * deltaT;
					
				}
			}



			if (kbState[SDL_SCANCODE_DOWN] && !kbPrevState[SDL_SCANCODE_UP]) {
				if (me_sprite->y_pos < (worldSize_Y - me_sprite->sizeY())) {
					me_sprite->changeAction(Running);
					me_sprite->set_upDown(1);
					//me_sprite->y_pos += player_speed * deltaT;
					
				}
			}
			else if (kbState[SDL_SCANCODE_UP] && !kbPrevState[SDL_SCANCODE_DOWN]) {
				if (me_sprite->y_pos > 0) {
					me_sprite->changeAction(Running);
					me_sprite->set_upDown(-1);
					//me_sprite->y_pos -= player_speed * deltaT;
					
				}
			}


			float damp_factor = 1;
			if (me_sprite->upDown != 0 && me_sprite->leftRight != 0)
				damp_factor = diagonal_fudge_factor;

			me_sprite->x_pos += me_sprite->leftRight * player_speed * damp_factor * deltaT;
			me_sprite->y_pos += me_sprite->upDown * player_speed * damp_factor * deltaT;
			
			float camera_x_offset = me_sprite->leftRight * player_speed * damp_factor * deltaT;
			float camera_y_offset = me_sprite->upDown * player_speed * damp_factor * deltaT;
			if(camera.x + camera_x_offset > 8 && camera.x + camera_x_offset < (worldSize_X - cameraSize_X) - 8)
				camera.x += me_sprite->leftRight * player_speed * damp_factor * deltaT;
			if (camera.y+ camera_y_offset > 8 && (camera.y+ camera_y_offset < (worldSize_Y - cameraSize_Y) - 8))
				camera.y += me_sprite->upDown * player_speed * damp_factor * deltaT;

		}



		//moveEnemy(enemy_sprite, me_sprite); //debugging


		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw Background //
		background_Controller.drawBackground();
		/*
		for (int x = (floor(camera.x / bg.x_size)*bg.x_size) ; x < (ceil((camera.x + cameraSize_X) / bg.x_size)*bg.x_size); x += bg.x_size) {
			for (int y = (floor(camera.y / bg.y_size)*bg.y_size); y < (ceil((camera.y + cameraSize_Y) / bg.y_size)*bg.y_size); y += bg.y_size) {
				//if(x >= ( camera.x - bg.x_size ) && x <  camera.xMax() && y >= (camera.y - bg.y_size) && y < camera.yMax())
					glDrawSprite(bg.tile, (x - camera.x), (y - camera.y), bg.x_size, bg.y_size);
			}
		}*/

		start_gate->update();
		start_gate->draw();
		next_gate->update();
		next_gate->draw();

		spritebucket.updateProjectiles();
		spritebucket.updateSprites();
		spritebucket.updateExplosions();
		spritebucket.updateHealth();

		spritebucket.drawHealth();
		spritebucket.drawSprites();
		spritebucket.drawProjectiles();
		spritebucket.drawExplosions();
		me_hud->drawHud();
		// Present the most recent frame.
		SDL_GL_SwapWindow(window);
	}




	//Free the sound effects
	Mix_FreeChunk(enemyShoot_wav);
	Mix_FreeChunk(enemyDmgd_wav);
	Mix_FreeChunk(playerShoot_wav);
	Mix_FreeChunk(playerDmgd_wav);
	Mix_FreeChunk(teleport_wav);
	Mix_FreeChunk(gateopen_wav);
	Mix_FreeChunk(lifeboost_wav);
	Mix_FreeChunk(gameover_wav);
	Mix_FreeChunk(victory_wav);

	//Free the music
	Mix_FreeMusic(music_wav);

	//Quit SDL_mixer
	Mix_CloseAudio();

	SDL_Quit();

	return 0;
}
