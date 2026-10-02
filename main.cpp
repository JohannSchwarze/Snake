#include <ncurses.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string>

//config
#define WIDTH 80
#define HEIGHT 30
#define FPS 15

#define SNAKE_START_X 40
#define SNAKE_START_Y 15
#define SNAKE_START_DIR SNAKE_DIR_UP
#define SNAKE_START_SPEED 15			//in tiles per second

#define NANOSEC pow(10,9)


//bitmasks
#define SNAKE_DIR_UP 0
#define SNAKE_DIR_LEFT 1
#define SNAKE_DIR_DOWN 2
#define SNAKE_DIR_RIGHT 3

#define SCENE_WALL '#'
#define SCENE_EMPTY ' '
#define SCENE_APPLE 'a'
#define SCENE_SNAKE_TAIL 'o'
#define SCENE_SNAKE_HEAD '@'


//scene
char scene[WIDTH][HEIGHT];


//timers
timespec currentTime;
timespec timeOfLastIteration;
long int timeSinceLastUpdate;

//user input
int lastInput;
int quitRequested;
bool paused;

//gameplay variables
int snakePosX;
int snakePosY;
int snakeDir;
float snakeSpeed;

int snakeTailPosX[WIDTH*HEIGHT];
int snakeTailPosY[WIDTH*HEIGHT];
int snakeTailCount;

int applePosX;
int applePosY;

//debug
int counter;


void initScene();

void receiveInput();
void setDirUp();
void setDirRight();
void setDirLeft();
void setDirDown();


void updateScene(long int delta);
void updateSnake(long int delta);
void spawnApple();
void detectCollisions();
bool detectSnakeAppleCollision();
bool detectSnakeSnakeCollision();

void renderScene();
void renderBackground();
void renderSnake();
void renderApples();
void renderDebug();
void renderPaused();

int main() {
	initscr();
//	noecho();
	raw();
	keypad(stdscr, true);
	nodelay(stdscr, true);
	srand((unsigned)time(NULL));


	initScene();

	paused = true;
	quitRequested = 0;
	counter = 0;
	timeSinceLastUpdate = 0;
	while(!quitRequested) {
		//calc delta
		clock_gettime(CLOCK_REALTIME, &currentTime);
		long int delta = currentTime.tv_nsec - timeOfLastIteration.tv_nsec;
		if (delta < 0) {
			long int currentNanoSec = currentTime.tv_nsec + NANOSEC;
			delta = currentNanoSec - timeOfLastIteration.tv_nsec;
		}
		//update game
		timeSinceLastUpdate += delta;
		if (timeSinceLastUpdate >= NANOSEC/FPS) {
			//receive inputs
			receiveInput();
			//update game
			if (!paused) {updateScene(timeSinceLastUpdate);}
			renderScene();
			timeSinceLastUpdate = 0;
		}


		timeOfLastIteration = currentTime;
	}

	getch();
	endwin();

	return 0;
}


void initScene() {
	//load map
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if ((y == 0) || (x == 0) || (y == HEIGHT-1) || (x == WIDTH-1)) {
				scene[x][y] = SCENE_WALL;
			} else {
				scene[x][y] = SCENE_EMPTY;
			}
		}
	}

	//load snake
	snakePosX = SNAKE_START_X;
	snakePosY = SNAKE_START_Y;
	snakeDir = SNAKE_START_DIR;
	snakeSpeed = SNAKE_START_SPEED;

	snakeTailPosX[0] = SNAKE_START_X+2;
	snakeTailPosY[0] = SNAKE_START_Y;
	snakeTailPosX[1] = SNAKE_START_X+1;
	snakeTailPosY[1] = SNAKE_START_Y;
	snakeTailCount = 2;

	//load apple
	spawnApple();
}








void receiveInput() {
	lastInput = getch();
	switch (lastInput) {
		case 'w':
			setDirUp();
			break;
		case 'a':
			setDirLeft();
			break;
		case 's':
			setDirDown();
			break;
		case 'd':
			setDirRight();
			break;
		case KEY_UP:
			setDirUp();
			break;
		case KEY_LEFT:
			setDirLeft();
			break;
		case KEY_DOWN:
			setDirDown();
			break;
		case KEY_RIGHT:
			setDirRight();
			break;
		case '8':
			setDirUp();
			break;
		case '4':
			setDirLeft();
			break;
		case '2':
			setDirDown();
			break;
		case '6':
			setDirRight();
			break;
		case 24:	// control + x
			quitRequested = 1;
			break;
		case ' ':
			paused = !paused;
			break;
		default:
			break;
	}
}

void setDirUp() {if (snakeDir != SNAKE_DIR_DOWN) {snakeDir = SNAKE_DIR_UP;}}
void setDirLeft() {if (snakeDir != SNAKE_DIR_RIGHT) {snakeDir = SNAKE_DIR_LEFT;}}
void setDirRight() {if (snakeDir != SNAKE_DIR_LEFT) {snakeDir = SNAKE_DIR_RIGHT;}}
void setDirDown() {if (snakeDir != SNAKE_DIR_UP) {snakeDir = SNAKE_DIR_DOWN;}}


void updateScene(long int delta) {


	updateSnake(delta);
	detectCollisions();

	counter++;
}



void updateSnake(long int delta) {
	int newHolePosX = snakePosX;
	int newHolePosY = snakePosY;

	float deltaInSec = delta / NANOSEC;
	float diffInPos = snakeSpeed * deltaInSec;

	switch(snakeDir) {
		case SNAKE_DIR_UP:
			snakePosX = snakePosX;
			snakePosY --;
			break;
		case SNAKE_DIR_LEFT:
			snakePosX --;
			snakePosY = snakePosY;
			break;
		case SNAKE_DIR_RIGHT:
			snakePosX ++;
			snakePosY = snakePosY;
			break;
		case SNAKE_DIR_DOWN:
			snakePosX = snakePosX;
			snakePosY ++;
			break;
		default:
			break;
	}


	if ((snakePosX == applePosX) && (snakePosY == applePosY)) {
		snakeTailPosX[snakeTailCount] = newHolePosX;
		snakeTailPosY[snakeTailCount] = newHolePosY;
		snakeTailCount++;
		snakeSpeed++;
		spawnApple();
	} else {
		for (int k = 0; k < snakeTailCount; k++) {
			snakeTailPosX[k] = snakeTailPosX[k+1];
			snakeTailPosY[k] = snakeTailPosY[k+1];
			if (k == snakeTailCount-1) {
				snakeTailPosX[k] = newHolePosX;
				snakeTailPosY[k] = newHolePosY;
			}
		}
	}
}

void spawnApple() {
	while (true) {
		applePosX = rand() % WIDTH;
		applePosY = rand() % HEIGHT;
		if (scene[applePosX][applePosY] == SCENE_EMPTY) {
			if (!detectSnakeAppleCollision()) {
				break;
			}
		}

	}
}

void detectCollisions() {
	char currentTile = scene[snakePosX][snakePosY];
	if (currentTile == SCENE_WALL) {
		quitRequested = 1;
	}
	if (detectSnakeSnakeCollision()) {
		quitRequested = 1;
	}
}

bool detectSnakeAppleCollision() {
	if ((applePosX == snakePosX) && (applePosY == snakePosY)) {
		return true;
	}
	for (int s = 0; s < snakeTailCount; s++) {
		if ((applePosX == snakeTailPosX[s]) && (applePosY == snakeTailPosY[s])) {
			return true;
		}
	}
	return false;
}

bool detectSnakeSnakeCollision() {
	for (int s = 0; s < snakeTailCount; s++) {
		if ((snakePosX == snakeTailPosX[s]) && (snakePosY == snakeTailPosY[s])) {
			return true;
		}
	}
	return false;
}





void renderScene() {
	renderBackground();
	if (!paused) {
		renderSnake();
		renderApples();
		renderDebug();
	} else {
		renderPaused();
	}
	refresh();
}


void renderBackground() {
	move(0,0);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			mvaddch(y,x,scene[x][y]);
		}
	}
}

void renderSnake() {
	for (int i = 0; i < snakeTailCount; i++) {
		mvaddch(snakeTailPosY[i], snakeTailPosX[i],SCENE_SNAKE_TAIL);
	}
	move(snakePosY,snakePosX);
	addch(SCENE_SNAKE_HEAD);
}

void renderApples() {
	mvaddch(applePosY, applePosX, SCENE_APPLE);
}

void renderDebug() {
	move(HEIGHT+1,0);
	printw("Input: %c \n", lastInput);
	printw("Counter: %i \n", counter);
}

void renderPaused() {
	std::string message ("Paused! Press SPACE to unpause.");
	move(HEIGHT/2, WIDTH/2 - message.length()/2);
	printw(message.c_str());
}
