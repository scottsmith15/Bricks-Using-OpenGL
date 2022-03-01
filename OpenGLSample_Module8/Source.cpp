#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <string>

using namespace std;
const float DEG2RAD = 3.14159 / 180;
enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE, PADDLE};
enum ONOFF { ON, OFF };

// release new circle every 5 pts
int score = 0;


class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	// half length of width
	double halfside;
	// side coordinates
	double bottom;
	double right;
	double top;
	double left;	

	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx, y = yy; width = ww; red = rr, green = gg, blue = bb;
		halfside = width / 2;
		bottom = y - halfside;
		right = x + halfside;
		top = y + halfside;
		left = x - halfside;
		onoff = ON;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			// draw paddle as rectangle
			// will be cut in thirds to control circle movement 
			if (brick_type == PADDLE) {
				right = x + halfside;
				left = x - halfside;
				glVertex2d(right, y);
				glVertex2d(right, (y - 0.1));
				glVertex2d(left, (y - 0.1));
				glVertex2d(left, y);
			}
			else {
				glVertex2d(right, top);
				glVertex2d(right, bottom);
				glVertex2d(left, bottom);
				glVertex2d(left, top);
			}

			glEnd();
		}
	}
};

class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float bottom;
	float right;
	float top;
	float left;
	float speed = 0.022;
	int direction;	// 1=down 2=right 3=up 4=left 
					// 5=down,right  6=down,left  7=up,right  8=up,left


	// determine how circle will bounce
	void physics(double edge);

	Circle(double xx, double yy, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		direction = dir;
		radius = rad;
		bottom = y - radius;
		right = x + radius;
		top = y + radius;
		left = x - radius;
		red = r;
		green = g;
		blue = b;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			// contact bottom
			if ((x > brk->left && x <= brk->right) && ((y + radius) > brk->bottom && (y + radius) <= brk->top))
			{
				physics("below");
				brk->brick_type = DESTRUCTABLE;
				brk->green = 0.0;
				brk->red = 1.0;
			}

			// contact right
			else if (((x - radius) >= brk->left && (x - radius) < brk->right) && (y > brk->bottom && y <= brk->top))
			{
				physics("rightside");
				brk->brick_type = DESTRUCTABLE;
				brk->green = 0.0;
				brk->red = 1.0;
			}

			// contact top
			else if ((x >= brk->left && x < brk->right) && ((y - radius) >= brk->bottom && (y - radius) < brk->top))
			{
				physics("above");
				brk->brick_type = DESTRUCTABLE;
				brk->green = 0.0;
				brk->red = 1.0;
			}

			// contact left
			else if (((x + radius) > brk->left && (x - radius) <= brk->right) && (y >= brk->bottom && y < brk->top))
			{
				physics("leftside");
				brk->brick_type = DESTRUCTABLE;
				brk->green = 0.0;
				brk->red = 1.0;
			}

		}

		else if (brk->brick_type == DESTRUCTABLE)
		{
			if (brk->onoff == ON)
			{
				// contact bottom
				if ((x > brk->left && x <= brk->right) && ((y + radius) > brk->bottom && (y + radius) <= brk->top))
				{
					physics("below");
					brk->onoff = OFF;
					score++;
				}

				// contact right
				else if (((x - radius) >= brk->left && (x - radius) < brk->right) && (y > brk->bottom && y <= brk->top))
				{
					physics("rightside");
					brk->onoff = OFF;
					score++;
				}

				// contact top
				else if ((x >= brk->left && x < brk->right) && ((y - radius) >= brk->bottom && (y - radius) < brk->top))
				{
					physics("above");
					brk->onoff = OFF;
					score++;
				}

				// contact left
				else if (((x + radius) > brk->left && (x - radius) <= brk->right) && (y >= brk->bottom && y < brk->top))
				{
					physics("leftside");
					brk->onoff = OFF;
					score++;
				}
			}

		}

		else if (brk->brick_type == PADDLE) 
		{
			if ((x >= brk->left && x <= brk->right) && ((y - radius) >= brk->bottom && (y - radius) < brk->y)) {
				if (x >= brk->left && x < (brk->left + (brk->width / 3))) 
				{
					direction = 8;
				}
				else if (x >= (brk->left + (brk->width / 3)) && x <= (brk->left + ((brk->width / 3) * 2)))
				{
					direction = 3;
				}
				else if (x > (brk->left + ((brk->width / 3) * 2)) && x <= brk->right)
				{
					direction = 7;
				}
			}
		}
	}

	// directions: 1=down 2=right 3=up 4=left  5=down, right  6=down, left  7=up, right  8=up, left
	bool MoveOneStep()
	{
		// is the circle still in play?
		bool alive = true;

		if (direction == 1 || direction == 5 || direction == 6)  // down
		{
			if (y > -1 + radius)
			{
				y -= speed;
				bottom = y - radius;
				top = y + radius;
			}
			else
			{
				alive = false;
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
				right = x + radius;
				left = x - radius;
			}
			else
			{
				physics("leftside");
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // up
		{
			if (y < 1 - radius) {
				y += speed;
				bottom = y - radius;
				top = y + radius;
			}
			else
			{
				physics("below");
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
				right = x + radius;
				left = x - radius;
			}
			else
			{
				physics("rightside");
			}
		}
		return alive;
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}

	// determine movement of circle on contact based on position 
	void physics(string pos) 
	{
		if (pos == "above") {
			if (direction == 1) { direction = 3; }		// down -> up
			else if (direction == 5) { direction = 7; }	// down, right -> up, right
			else { direction = 8; }						// down, left -> up, left
		}
		else if (pos == "leftside") {
			if (direction == 2) { direction = 4; }		// right -> left
			else if (direction == 5) { direction = 6; }	// down, right -> down, left
			else { direction = 8; }						// up, right -> up, left
		}
		else if (pos == "below") {
			if (direction == 3) { direction = 1; }		// up -> down
			else if (direction == 7) { direction = 5; }	// up, right -> down, right
			else { direction = 6; }						// up, left -> down, left
		}
		else if (pos == "rightside") {
			if (direction == 4) { direction = 2; }		// left -> right
			else if (direction == 6) { direction = 5; }	// down, left -> down, right
			else { direction = 7; }						// up, left -> up, right 
		}
	}

	void CheckCircleCollision(Circle* circ) 
	{
		// contact bottom
		if ((x > circ->left && x <= circ->right) && ((y + radius) > circ->bottom && (y + radius) <= circ->top))
		{
			physics("below");
			red = rand() % 10 * .1;
			blue = rand() % 10 * .1;
			green = rand() % 10 * .1;
		}

		// contact right
		else if (((x - radius) >= circ->left && (x - radius) < circ->right) && (y > circ->bottom && y <= circ->top))
		{
			physics("rightside");
			red = rand() % 10 * .1;
			blue = rand() % 10 * .1;
			green = rand() % 10 * .1;
		}

		// contact top
		else if ((x >= circ->left && x < circ->right) && ((y - radius) >= circ->bottom && (y - radius) < circ->top))
		{
			physics("above");
			red = rand() % 10 * .1;
			blue = rand() % 10 * .1;
			green = rand() % 10 * .1;
		}

		// contact left
		else if (((x + radius) > circ->left && (x - radius) <= circ->right) && (y >= circ->bottom && y < circ->top))
		{
			physics("leftside");
			red = rand() % 10 * .1;
			blue = rand() % 10 * .1;
			green = rand() % 10 * .1;
		}
	}
};

vector<Circle> world;
vector<Brick> bricks;

// used for redrawing paddle
Brick paddle(PADDLE, 0.0, -0.8, 0.3, 1.0, 1.0, 1.0);

// pass paddle to input processing function 
void processInput(GLFWwindow* window, Brick* paddle);

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "Scott Smith CS330: BRICKS", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// create bricks
	for (float j = 0.9; j >= 0.0; j -= 0.2) 
	{
		for (float i = -0.9; i <= 0.9; i += 0.2) {
			Brick brick(REFLECTIVE, i, j, 0.2, 0.0, 1.0, 0.0);
			bricks.push_back(brick);
		}
	}

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window, &paddle);

		// check collisions, process movement, draw circles
		for (int i = 0; i < world.size(); i++)
		{
			// paddle collisions
			world[i].CheckCollision(&paddle);
			
			// brick collisions
			for (int j = 0; j < bricks.size(); j++) 
			{
				world[i].CheckCollision(&bricks[j]);
				// draw new circle after breaking brick 
				if (score > 0)
				{
					// limit amount of circles on screen
					if (world.size() < 6)
					{
						Circle B(paddle.x, -0.75, 3, 0.05, 1.0, 1.0, 1.0);
						world.push_back(B);
						score = 0;
					}
				}
			}

			// circle collisions
			for (int j = i + 1; j < world.size(); j++)
			{
				world[j].CheckCircleCollision(&world[i]);
				world[i].CheckCircleCollision(&world[j]);
			}

			// if circle still in play, draw
			if (world[i].MoveOneStep()) 
			{
				world[i].DrawCircle();
			}
			// else, remove from world
			else 
			{
				world.erase(world.begin() + i);
			}
		}

		// draw paddle and bricks
		paddle.drawBrick();
		for (int i = 0; i < bricks.size(); i++) 
		{
			bricks[i].drawBrick();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window, Brick* paddle)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// draw new circle if no circles present
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		if (world.size() < 1) 
		{
			Circle B(paddle->x, -0.75, 3, 0.05, 1.0, 1.0, 1.0);
			world.push_back(B);
		}
	}

	// keep paddle on screen
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		if (paddle->left > -1.0) {
			paddle->x -= .05;
		}
	}

	// keep paddle on screen
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		if (paddle->right < 1.0) {
			paddle->x += .05;
		}
	}
}