#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define PI 3.1415926535

using namespace std;

float px, py, pdx, pdy, pa; //player x and y, delta x, delta y, and player angle

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W) {
        px += pdx;
        py += pdy;
    }
    if (key == GLFW_KEY_A) {
        //px -= 5;
        pa -= 0.1;
        if (pa < 0) {
            pa += 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    if (key == GLFW_KEY_S) {
        px -= pdx;
        py -= pdy;
    }
    if (key == GLFW_KEY_D) {
        pa += 0.1;
        if (pa > 2 * PI) {
            pa -= 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }

}

void drawPlayer() {
    glColor3f(1, 1, 0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(px,py);
    glEnd();

    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px + pdx*5, py+pdy*5);
    glEnd();
}

int mapX=8, mapY=8, mapS=64;
int map[]=
{
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

void drawMap2D() {
    int xo, yo;
    for (int y = 0; y < mapY; y++) {
        for (int x = 0; x < mapX; x++) {
            if (map[y * mapX + x] == 1) {
                glColor3f(1, 1, 1);
            }
            else {
                glColor3f(0, 0, 0);
            }
            xo = x*mapS;
            yo = y * mapS;
            glBegin(GL_QUADS);
            glVertex2i(xo + 1, yo+1);
            glVertex2i(xo + 1, yo+mapS-1);
            glVertex2i(xo+mapS-1, yo+mapS-1);
            glVertex2i(xo+mapS-1, yo +1);
            glEnd();
        }
    }
}

void display(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    drawMap2D();
    drawPlayer();
    glfwSwapBuffers(window);
}

void init() {
    glClearColor(0.3, 0.3, 0.3, 0);
    glOrtho(0, 1024, 512, 0, -1, 1);
    px = 300;
    py = 300;
    pdx = cos(pa)*5;
    pdy = sin(pa)*5;
}

void render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMap2D();
    drawPlayer();
}

int main(int argc, char* argv[]){
    //initialize GLFW
    if (!glfwInit()) {
        cerr << "Cannot initalize GLFW" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1024, 512, "RandomTower", nullptr, nullptr);


    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, display);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    init();

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {

        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}