#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "Textures/BrickTexture.ppm"
#include "Textures/FirstTextures.ppm"
#include "Textures/SkyTexture.ppm"

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 //one degree in radians

using namespace std;

float FixAngle(float angle){if (angle > 359) {angle -= 360;} if (angle < 0) {angle+= 360;} return angle;}
float degToRad(float angle){return angle*PI/180;}
float px, py, pdx, pdy, pa; //player x and y, delta x, delta y, and player angle

double frame1, frame2, fps;

int mapX=8, mapY=8, mapS=64;
int mapW[]=
{
    1, 1, 1, 1, 2, 2, 2, 2,
    6, 0, 0, 1, 0, 0, 0, 2,
    1, 0, 0, 4, 0, 2, 0, 2,
    1, 5, 4, 5, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 0, 0, 1, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

int mapF[]=
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 2, 0, 1, 0,
    0, 0, 0, 0, 1, 1, 1, 0,
    0, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

int mapC[]=
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

//Non-continuous input, such as pressing a button
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_E) {
            int xo = 0;
            if (pdx < 0) {xo=-25;}
            else {xo=25;}

            int yo = 0;
            if (pdy < 0) {yo=-25;}
            else {yo=25;}

            int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
            int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;

            if (mapW[ipy_add_yo*mapX+ipx_add_xo]==4) {mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
        }
    }
}

//Continuous input, such as moving
void handleInput(GLFWwindow* window) {
    if (glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS){
        //px -= 5;
        pa -= 0.025;
        if (pa < 0) {
            pa += 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    if (glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) {
        pa += 0.025;
        if (pa > 2 * PI) {
            pa -= 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    int xo = 0;
    if (pdx<0) {xo = -20;}
    else {xo = 20;}

    int yo = 0;
    if (pdy<0) {yo = -20;}
    else {yo = 20;}

    //setting up variables for collision detection
    int ipx = px/64.0;
    int ipx_add_xo = (px+xo)/64.0;
    int ipx_sub_xo = (px-xo)/64.0;

    int ipy = py/64.0;
    int ipy_add_yo = (py+yo)/64.0;
    int ipy_sub_yo = (py-yo)/64.0;

    if (glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) {
        if (mapW[ipy*mapX + ipx_add_xo] == 0){
            px+=pdx*.0025*fps;
        }
        if (mapW[ipy_add_yo*mapX + ipx] == 0) {
            py+=pdy*.0025*fps;
        }
    }

    if (glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) {
        if (mapW[ipy*mapX + ipx_sub_xo] == 0){
            px-=pdx*.0025*fps;
        }
        if (mapW[ipy_sub_yo*mapX + ipx] == 0) {
            py-=pdy*.0025*fps;
        }
    }

}

/*void drawPlayer() {
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
}*/

/*void drawMap2D() {
    int xo, yo;
    for (int y = 0; y < mapY; y++) {
        for (int x = 0; x < mapX; x++) {
            if (mapW[y * mapX + x] > 0) {
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
}*/

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
    for (r = 0; r < 120; r++) {
        float true_ra = ra;
        int virTex = 0, horTex = 0;
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
            if (mp > 0 && mp < mapX*mapY && mapW[mp] > 0) { //wall collision detection
                horTex = mapW[mp]-1;
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
            if (mp > 0 && mp < mapX*mapY && mapW[mp] > 0) { //wall collision detection
                virTex = mapW[mp]-1;
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
        float shade = 1;
        if (disV<disH) {
            horTex = virTex;
            shade = .5;
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

        //lines for the rays on the map
        /*glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(px,py);
        glVertex2i(rx,ry);
        glEnd();*/

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
        float lineH=(mapS*640)/disT;
        float ty_step=32.0/(float)lineH;
        //helping to prevent walls appearing flat on screen
        //when too close to camera
        float ty_off=0;
        if (lineH>640) {
            ty_off=(lineH-640)/2.0;
            lineH=640;
        }
        float lineO=320-lineH/2; //line offset

        //setting up displaying individual pixels
        //draws walls
        int y;
        float ty = ty_off*ty_step;//+horTex*32;
        float tx;

        if (shade == 1) {
            tx = (int)(rx/2.0)%32;
            if (ra > 0 && ra < 180 * DR) {tx = 31-tx;}
        }
        else {
            tx= (int)(ry/2.0)%32;
            if (ra > 90 * DR && ra < 270 * DR) {tx = 31-tx;}
        }

        //ty += 32;

        for (y = 0; y < lineH; y++){
            //float c = All_Textures[(int)(ty)*32 + (int)(tx)] * shade;
            /*if (horTex==0) {glColor3f(c, c/2.0, c/2.0);} //check red
            if (horTex==1) {glColor3f(c, c, c/2.0);} //brick yellow
            if (horTex==2) {glColor3f(c/2.0, c/2.0, c);} //window blue
            if (horTex==3) {glColor3f(c/2.0, c, c/2.0);} //door green
            //glColor3f(c, c, c);
            glPointSize(8);
            glBegin(GL_POINTS);
            glVertex2i(r*8+530,y + lineO);
            glEnd();*/
            int pixel = ((int)ty * 32+(int)tx) * 3+(horTex*32*32*3);
            int red = FirstTextures[pixel+0] * shade;
            int green = FirstTextures[pixel+1] * shade;
            int blue = FirstTextures[pixel+2] * shade;
            glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(r*8,y + lineO); glEnd();
            ty+=ty_step;
        }

        //draw floors (and later ceilings!)
        for (y = lineO+lineH; y < 640; y++) {

            float dy = y -(640/2.0), deg = true_ra, raFix = cos(pa-true_ra);

            tx = px/2.0f + cos(deg)*158*2*32/dy/raFix;
            ty = py/2.0f + sin(deg)*158*2*32/dy/raFix;

            /*if (tx < 0) tx = 0;
            if (tx >= mapX * 32) tx = mapX * 32 - 1;
            if (ty < 0) ty = 0;
            if (ty >= mapY * 32) ty = mapY * 32 - 1;

            if (dy == 0) dy = 0.0001f;*/

            int Ftex = mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
            /*float c = FirstTextures[((int)(ty)&31) * 32 + ((int)(tx)&31)+Ftex] * 0.7;
            glColor3f(c/1.3, c/1.3, c);
            glPointSize(8);
            glBegin(GL_POINTS);
            glVertex2i(r*8+530,y);
            glEnd();*/
            int pixel = (((int)(ty)&31)*32 +((int)(tx)&31))* 3+Ftex*3;
            int red = FirstTextures[pixel+0] * 0.7;
            int green = FirstTextures[pixel+1] * 0.7;
            int blue = FirstTextures[pixel+2] * 0.7;;
            glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(r*8,y); glEnd();

            //draw ceiling (OLD)
            //also warping...
            /*int Ctex = mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
            c = FirstTextures[((int)(ty)&31) * 32 + ((int)(tx)&31)+Ctex] * 0.7;
            glColor3f(c/2.0, c/1.2, c/2.0);
            glPointSize(8);
            glBegin(GL_POINTS);
            glVertex2i(r*8+530,320-y);
            glEnd();*/
            //new ceiling code
            int Ctex = mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
            pixel = (((int)(ty)&31)*32 +((int)(tx)&31))* 3+Ctex*3;
            red = FirstTextures[pixel+0];
            green = FirstTextures[pixel+1];
            blue = FirstTextures[pixel+2];
            if (Ctex > 0) {
                glPointSize(8);
                glColor3ub(red, green, blue);
                glBegin(GL_POINTS);
                glVertex2i(r*8,640-y);
                glEnd();
            }
        }


        ra+=.5 * DR;
        if (ra < 0) {
            ra+=2*PI;
        }
        if (ra > 2*PI) {
            ra-=2*PI;
        }
    }
}

void drawSky() {
    int x, y;
    for (y = 0; y < 40; y++) {
        for (x = 0; x < 120; x++) {
            int xo = (int)FixAngle(pa * 19)-x; if (xo < 0) {xo += 120;} xo=xo % 120;
            int pixel = (y*120+xo)*3;
            int red = SkyTexture[pixel+0];
            int green = SkyTexture[pixel+1];
            int blue = SkyTexture[pixel+2];
            glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
        }
    }
}

void display(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    //drawMap2D();
    drawSky();
    drawRays3D();
    //drawPlayer();
    glfwSwapBuffers(window);
}

void resize(GLFWwindow* window, int w, int h) {
    glfwSetWindowSize(window, 960, 640);
}

void init() {
    glClearColor(0.3, 0.3, 0.3, 0);
    glOrtho(0, 960, 640, 0, -1, 1);
    px = 300;
    py = 300;
    pdx = cos(pa)*5;
    pdy = sin(pa)*5;
}

void render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //drawMap2D();
    drawSky();
    drawRays3D();
    //drawPlayer();
    //code for testing texture by adding a still image to top left of screen
    /*int x, y;
    for (y = 0; y < 32; y++) {
        for (x = 0; x < 32; x++) {
            int pixel = (y * 32+x) * 3;
            int red = BrickTexture[pixel+0];
            int green = BrickTexture[pixel+1];
            int blue = BrickTexture[pixel+2];
            glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
        }
    }*/
}

int main(int argc, char* argv[]){
    //initialize GLFW
    if (!glfwInit()) {
        cerr << "Cannot initalize GLFW" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(960, 640, "RandomTower", nullptr, nullptr);

    //center the game window to the user's screen
    int xCenter, yCenter;

    glfwGetWindowSize(window, &xCenter, &yCenter);

    xCenter = (xCenter / 2) + 960/2;
    yCenter = (yCenter / 2) + 640/2;

    glfwSetWindowPos(window, xCenter, yCenter);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, display);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    init();

    glfwSetWindowSizeCallback(window, resize);

    glfwSetKeyCallback(window, keyCallback);

    frame1 = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        handleInput(window);
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