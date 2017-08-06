#ifndef ANIMATIONSTACK_H
#define ANIMATIONSTACK_H
using namespace std;

class AnimationStack {
	public:
		AnimationStack(const char* image_i, int moment_i, bool loop = true);
		void add(const char* image_i, int moment_i);
		int initSpriteFrame() { return 0; }
		GLuint currentAnimation(int frame) { return animation.at(frame); }
		int timeTillNextAnimation(int frame) { return moment.at(frame); }
		int nextFrame(int previousFrame);
		int sizeX;
		int sizeY;
		bool loop;
		int getTotalTime(){ return total_time; }
	private:
		int total_time;
		int animationStack_size;
		vector <GLuint> animation;
		vector <int> moment;
};

#endif