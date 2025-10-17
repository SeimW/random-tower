#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 //one degree in radians


using namespace std;

float px, py, pdx, pdy, pa; //player x and y, delta x, delta y, and player angle

double frame1, frame2, fps;

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_A) {
        //px -= 5;
        pa -= 0.1;
        if (pa < 0) {
            pa += 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    if (key == GLFW_KEY_D) {
        pa += 0.1;
        if (pa > 2 * PI) {
            pa -= 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    int xo = 0;
    if (pdx<0) {
        xo = -20;
    }
    else {
        xo = 20;
    }
    int yo = 0;

    if (pdy<0) {
        yo = -20;
    }
    else {
        yo = 20;
    }

    //setting up variables for collision detection
    int ipx = px/64.0;
    int ipx_add_xo = (px+xo)/64.0;
    int ipx_sub_xo = (px-xo)/64.0;

    int ipy = py/64.0;
    int ipy_add_yo = (py+yo)/64.0;
    int ipy_sub_yo = (py-yo)/64.0;

    //frame2=glfwGetTime();
    //fps = (frame2-frame1);
    //frame1=glfwGetTime();

    if (key == GLFW_KEY_W) {
        if (map[ipy*mapX + ipx_add_xo] == 0){
            px+=pdx*.01*fps;
        }
        if (map[ipy_add_yo*mapX + ipx] == 0) {
            py+=pdy*.01*fps;
        }
        //px += pdx;
        //py += pdy;
    }
    if (key == GLFW_KEY_S) {
        if (map[ipy*mapX + ipx_sub_xo] == 0){
            px-=pdx*.01*fps;
        }
        if (map[ipy_sub_yo*mapX + ipx] == 0) {
            py-=pdy*.01*fps;
        }
        //px -= pdx;
        //py -= pdy;
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

float dist(float ax, float ay, float bx, float by, float ang) {
    return sqrt ((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}
void drawRays3D() {
    int r,mx,my,mp,dof;
    float rx,ry,ra,xo,yo,disT;
    ra=pa-DR*30;
    if (ra < 0) {
        ra+=2*PI;
    }
    if (ra > 2*PI) {
        ra-=2*PI;
    }
    for (r = 0; r < 60; r++) {
        //Check Horizontal Lines
        dof=0;
        float disH=1000000;
        float hx=px;
        float hy=py;
        float aTan=-1/tan(ra);
        if (ra>PI) { //looking downwards
            ry = ((int(py)>>6)<<6)-0.0001;
            rx = (py-ry) * aTan+px;
            yo = -64;
            xo = -yo*aTan;
        }
        if (ra<PI) { //looking upwards
            ry = ((int(py)>>6)<<6)+64;
            rx = (py-ry) * aTan+px;
            yo = 64;
            xo = -yo*aTan;
        }
        if (ra == 0 || abs(ra - PI) < 0.0001) { //looking straight left or right
            rx=px;
            ry=py;
            dof=8;
        }
        while (dof < 8) {
            mx = int(rx)>>6;
            my = int(ry)>>6;
            mp=my*mapX+mx;
            if (mp > 0 && mp < mapX*mapY && map[mp] == 1) { //wall collision detection
                hx=rx;
                hy=ry;
                disH=dist(px,py,hx,hy,ra);
                dof=8;
            }
            else {
                rx+=xo;
                ry+=yo;
                dof+=1;
            }
            /*glColor3f(0, 1, 0);
            glLineWidth(1);
            glBegin(GL_LINES);
            glVertex2i(px,py);
            glVertex2i(rx,ry);
            glEnd();*/
        }
        //Check Vertical Lines
        dof=0;
        float disV=1000000;
        float vx=px;
        float vy=py;
        float nTan=-tan(ra); //negative tangent
        if (ra>P2 && ra<P3) { //looking left
            rx = ((int(px)>>6)<<6)-0.0001;
            ry = (px-rx) * nTan+py;
            xo = -64;
            yo = -xo*nTan;
        }
        if (ra<P2 || ra>P3) { //looking right
            rx = ((int(px)>>6)<<6)+64;
            ry = (px-rx) * nTan+py;
            xo = 64;
            yo = -xo*nTan;
        }
        if (ra == 0 || abs(ra - PI) < 0.0001) { //looking straight up or down
            rx=px;
            ry=py;
            dof=8;
        }
        while (dof < 8) {
            mx = int(rx)>>6;
            my = int(ry)>>6;
            mp=my*mapX+mx;
            if (mp > 0 && mp < mapX*mapY && map[mp] == 1) { //wall collision detection
                vx=rx;
                vy=ry;
                disV=dist(px,py,vx,vy,ra);
                dof=8;
            }
            else {
                rx+=xo;
                ry+=yo;
                dof+=1;
            }
        }
        if (disV<disH) {
            rx=vx;
            ry=vy;
            disT=disV;
            glColor3f(0.9, 0, 0); //simple lighting
        }
        if (disH<disV) {
            rx=hx;
            ry=hy;
            disT=disH;
            glColor3f(0.7, 0, 0); //simple lighting
        }

        //glColor3f(1, 0, 0);
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex2i(px,py);
        glVertex2i(rx,ry);
        glEnd();
        //Draw 3D Walls
        //ca is used to resolve the fisheye effect
        //the raycasters naturally cause
        //as the rays to the further side of the player are naturally
        //longer which causes the effect
        float ca=pa-ra;
        if (ca < 0) {
            ca+=2*PI;
        }
        if (ca > 2*PI) {
            ca-=2*PI;
        }
        disT=disT*cos(ca);
        if (disT < 0.1f) {
            disT = 0.1f;
        }
        //window is 320x160 px
        //finding line height
        float lineH=(mapS*320)/disT;
        if (lineH>320) {
            lineH=320;
        }
        float lineO=160-lineH/2; //line offset
        glLineWidth(8);
        glBegin(GL_LINES);
        glVertex2i(r*8+530,lineO);
        glVertex2i(r*8+530,lineH+lineO);
        glEnd();

        ra+=DR;
        if (ra < 0) {
            ra+=2*PI;
        }
        if (ra > 2*PI) {
            ra-=2*PI;
        }
    }
}

void display(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    drawMap2D();
    drawRays3D();
    drawPlayer();
    glfwSwapBuffers(window);
}

void resize(GLFWwindow* window, int w, int h) {
    glfwSetWindowSize(window, 1024, 512);
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
    drawRays3D();
    drawPlayer();
}

int main(int argc, char* argv[]){
    //initialize GLFW
    if (!glfwInit()) {
        cerr << "Cannot initalize GLFW" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1024, 512, "RandomTower", nullptr, nullptr);
    glfwSetWindowPos(window, 200, 20);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, display);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    init();

    glfwSetKeyCallback(window, key_callback);

    glfwSetWindowSizeCallback(window, resize);

    frame1 = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        frame2 = glfwGetTime();
        double deltaTime = frame2 - frame1;
        frame1 = frame2;

        fps = 1.0/deltaTime;
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}