#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "Textures/BrickTexture.ppm"
#include "Textures/FirstTextures.ppm"
#include "Textures/SkyTexture.ppm"
#include "Textures/GameOverScreen.ppm"
#include "Textures/StartScreen.ppm"
#include "Textures/WinScreen.ppm"
#include "Textures/SpriteTextures.ppm"

#include "Level.h"

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 //one degree in radians

using namespace std;

float FixAngle(float angle){if (angle > 359) {angle -= 360;} if (angle < 0) {angle+= 360;} return angle;}
float degToRad(float angle){return angle*PI/180;}
float px, py, pdx, pdy, pa; //player x and y, delta x, delta y, and player angle
double frame1, frame2, fps;

int gameState=0, timer;
int depth[120];
bool startingGame, clearedLevel;

const int mapX=16, mapY=16, mapS=256;
int mapW[256], mapF[256], mapC[256], mapTex;
Level* level = nullptr;

typedef struct
{
    int type; //static, key, enemy
    int state; //on or off
    int map; //texture to show
    float x,y,z; //position
}sprite; sprite sp[2];

int testtime = 0;

void drawSprite() {
    int x, y, s;
    if (px<sp[0].x+30 && px>sp[0].x-30 && py<sp[0].y+30 && py>sp[0].y-30) {sp[0].state=0;} //pick up key
    if (px<sp[1].x+30 && px>sp[1].x-30 && py<sp[1].y+30 && py>sp[1].y-30) {gameState=4;} //enemy kills on contact

    //enemy movement
    //
    if (sp[1].x>px){sp[1].x-=1*.06;}
    if (sp[1].x<px){sp[1].x+=1*.06;}
    if (sp[1].y>py){sp[1].y-=1*.06;}
    if (sp[1].y<py){sp[1].y+=1*.06;}


    for (s=0; s<2; s++) {
        //relative position to player
        float dx = sp[s].x - px;
        float dy = sp[s].y - py;
        float dz = sp[s].z; // vertical

        //distance from player to sprite
        float distance = sqrt(dx*dx + dy*dy);

        //angle from player to sprite
        float spriteAngle = atan2(dy, dx);
        float angleDiff = spriteAngle - pa;

        //normalize
        while (angleDiff > M_PI)  angleDiff -= 2*M_PI;
        while (angleDiff < -M_PI) angleDiff += 2*M_PI;

        //fisheye correction
        float correctedDistance = distance * cos(angleDiff);
        if (cos(angleDiff) < 0.0001f) {
            correctedDistance = distance * 0.0001f;
        }

        if (correctedDistance <= 0.001f) return; // behind player, don’t draw

        //project to screen
        float fov = M_PI / 3.0f; // 60 degrees FOV
        float screenX = (960 / 2.0f) * (1 + tan(angleDiff) / tan(fov / 2.0f));
        float screenY = (640 / 2.0f) - (dz * 108.0f / correctedDistance);

        //scale by depth

        //Draw sprite if not blocked by wall
        int scale = 32*8*80/(distance/cos(angleDiff));

        if (scale < 0) {scale = 0;}
        if (scale > 120*8){ scale = 120*8;}

        //textures
        float t_x=0, t_y=31, t_x_step=31.5/(float)scale, t_y_step=32.0/(float)scale;

        int sx = (int)screenX;

        glPointSize(8);
        glBegin(GL_POINTS);
        for (x = sx-scale/2; x<sx+scale/2; x++) {
            t_y=31;
            for (y=0; y<scale; y++) {
                if (sp[s].state==1 && sx >= 0 && sx < 960 && (int)correctedDistance < depth[x/8]) {
                    int pixel = ((int)t_y * 32+(int)t_x) * 3+sp[s].map*32*32*3;
                    int red = SpriteTextures[pixel+0];
                    int green = SpriteTextures[pixel+1];
                    int blue = SpriteTextures[pixel+2];
                    if (red != 255 || green != 0 || blue != 255) {
                        glColor3f(red, green, blue); // yellow
                        glVertex2f(x, screenY-y);
                    }
                    t_y-=t_y_step;
                    if (t_y < 0) t_y = 0;
                }
            }
            t_x+=t_x_step;
        }
        glEnd();
    }
}


//Non-continuous input, such as pressing a button
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_E && sp[0].state==0) {
            int xo = 0;
            if (pdx < 0) {xo=-25;}
            else {xo=25;}

            int yo = 0;
            if (pdy < 0) {yo=-25;}
            else {yo=25;}

            int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
            int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;

            if (mapW[ipy_add_yo*mapX+ipx_add_xo]==4) {mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
            if (mapW[ipy_add_yo*mapX+ipx_add_xo]==6) {gameState = 3;}
        }
        if (key == GLFW_KEY_ENTER) {
            if (!startingGame && gameState == 1) {
                startingGame = true;
            }

        }
    }
}

//Continuous input, such as moving
void handleInput(GLFWwindow* window) {
    if (glfwGetKey(window,GLFW_KEY_LEFT) == GLFW_PRESS){
        //px -= 5;
        pa -= 0.025;
        if (pa < 0) {
            pa += 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    if (glfwGetKey(window,GLFW_KEY_RIGHT) == GLFW_PRESS) {
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

    if (glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) {
        // right strafe direction
        float sdx = -pdy;
        float sdy =  pdx;

        //offsets for strafing direction
        int xo_s = 20;
        int yo_s = 20;
        if (sdx < 0) {xo_s = -20;}
        if (sdy < 0) {yo_s = -20;}

        //collision
        int ipx_add_sx = (px + xo_s) / 64.0;
        int ipy_add_sy = (py + yo_s) / 64.0;

        if (mapW[ipy*mapX + ipx_add_sx] == 0){
            px+=sdx*.0025*fps;
        }
        if (mapW[ipy_add_sy*mapX + ipx] == 0) {
            py+=sdy*.0025*fps;
        }
    }
    if (glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) {
        //left strafe direction
        float sdx = pdy;
        float sdy = -pdx;

        //offsets for strafing direction
        int xo_s = 20;
        int yo_s = 20;
        if (sdx < 0) {xo_s = -20;}
        if (sdy < 0) {yo_s = -20;}

        //collision
        int ipx_add_sx = (px + xo_s) / 64.0;
        int ipy_add_sy = (py + yo_s) / 64.0;

        if (mapW[ipy*mapX + ipx_add_sx] == 0){
            px+=sdx*.0025*fps;
        }
        if (mapW[ipy_add_sy*mapX + ipx] == 0) {
            py+=sdy*.0025*fps;
        }
    }
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

float dist(float ax, float ay, float bx, float by, float ang) {
    return sqrt ((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}

const int CELL_SIZE = 64;
const int TEXTURE_SIZE = 32;

void drawRays3D() {
    int r, mx, my, mp, dof;
    float rx, ry, ra, xo, yo, disT;

    ra = pa - DR * 30;
    if (ra < 0) {ra += 2 * PI;}
    if (ra > 2 * PI){ra -= 2 * PI;}

    for (r = 0; r < 120; r++) {
        float true_ra = ra;
        int virTex = 0, horTex = 0;

        //Check Horizontal Lines
        dof = 0;
        float disH = 1000000;
        float hx = px;
        float hy = py;

        float aTan = -1 / tan(ra);

        if (ra > PI) {   // looking up
            ry = (int(py) / CELL_SIZE) * CELL_SIZE - 0.0001f;
            rx = (py - ry) * aTan + px;
            yo = -CELL_SIZE;
            xo = -yo * aTan;
        }
        else if (ra < PI) {  // looking down
            ry = (int(py) / CELL_SIZE) * CELL_SIZE + CELL_SIZE;
            rx = (py - ry) * aTan + px;
            yo = CELL_SIZE;
            xo = -yo * aTan;
        }
        else {  // looking straight left/right
            rx = px;
            ry = py;
            dof = 8;
        }

        while (dof < 8) {
            mx = int(rx) / CELL_SIZE;
            my = int(ry) / CELL_SIZE;
            mp = my * mapX + mx;

            if (mp >= 0 && mp < mapX * mapY && mapW[mp] > 0) { //wall collision
                horTex = mapW[mp] - 1;
                hx = rx;
                hy = ry;
                disH = dist(px, py, hx, hy, ra);
                dof = 8;
            } else {
                rx += xo;
                ry += yo;
                dof++;
            }
        }

        //Check Vertical Lines
        dof = 0;
        float disV = 1000000;
        float vx = px;
        float vy = py;

        float nTan = -tan(ra);

        if (ra > P2 && ra < P3) { // looking left
            rx = (int(px) / CELL_SIZE) * CELL_SIZE - 0.0001f;
            ry = (px - rx) * nTan + py;
            xo = -CELL_SIZE;
            yo = -xo * nTan;
        }
        else if (ra < P2 || ra > P3) { // looking right
            rx = (int(px) / CELL_SIZE) * CELL_SIZE + CELL_SIZE;
            ry = (px - rx) * nTan + py;
            xo = CELL_SIZE;
            yo = -xo * nTan;
        }
        else { // looking straight up/down
            rx = px;
            ry = py;
            dof = 8;
        }

        while (dof < 8) {
            mx = int(rx) / CELL_SIZE;
            my = int(ry) / CELL_SIZE;
            mp = my * mapX + mx;

            if (mp >= 0 && mp < mapX * mapY && mapW[mp] > 0) {
                virTex = mapW[mp] - 1;
                vx = rx;
                vy = ry;
                disV = dist(px, py, vx, vy, ra);
                dof = 8;
            } else {
                rx += xo;
                ry += yo;
                dof++;
            }
        }

        //pick nearest
        float shade = 1;
        if (disV < disH) {
            horTex = virTex;
            shade = 0.5;
            rx = vx;
            ry = vy;
            disT = disV;
        } else {
            rx = hx;
            ry = hy;
            disT = disH;
        }

        //fisheye correction
        float ca = pa - ra;
        if (ca < 0)      ca += 2 * PI;
        if (ca > 2 * PI) ca -= 2 * PI;
        disT = disT * cos(ca);
        if (disT < 0.1f) disT = 0.1f;

        depth[r] = disT;

        //wall projection
        float lineH = (CELL_SIZE * 640) / disT;
        float ty_step = float(TEXTURE_SIZE) / lineH;
        float ty_off = 0;

        if (lineH > 640) {
            ty_off = (lineH - 640) / 2.0f;
            lineH = 640;
        }

        float lineO = 320 - lineH / 2;

        float ty = ty_off * ty_step;
        float tx;

        // texture X coordinate based on hit side
        if (shade == 1) {
            tx = int(rx / (CELL_SIZE / TEXTURE_SIZE)) % TEXTURE_SIZE;
        } else {
            tx = int(ry / (CELL_SIZE / TEXTURE_SIZE)) % TEXTURE_SIZE;
        }

        //drawing walls
        for (int y = 0; y < lineH; y++) {
            int pixel = (int(ty) * TEXTURE_SIZE + int(tx)) * 3 +
                         horTex * TEXTURE_SIZE * TEXTURE_SIZE * 3;

            int red   = FirstTextures[pixel + 0] * shade;
            int green = FirstTextures[pixel + 1] * shade;
            int blue  = FirstTextures[pixel + 2] * shade;

            glPointSize(8);
            glColor3ub(red, green, blue);
            glBegin(GL_POINTS);
            glVertex2i(r * 8, y + lineO);
            glEnd();

            ty += ty_step;
        }

        //floor and ceiling
        for (int y = lineO + lineH; y < 640; y++) {

            // screen space distance from center
            float dy = y - 320.0f;
            float raFix = cos(pa - true_ra);

            // projection formula stablized
            float projDist = (CELL_SIZE * 320.0f) / (dy * raFix);

            float floorx = px + cos(true_ra) * projDist;
            float floory = py + sin(true_ra) * projDist;

            //map tile coords
            int fmx = (int)(floorx / CELL_SIZE);
            int fmy = (int)(floory / CELL_SIZE);

            //bounds check
            if (fmx < 0 || fmy < 0 || fmx >= mapX || fmy >= mapY)
                continue;

            //floor texture id
            int floorID = mapF[fmy * mapX + fmx];
            int floorOffset = floorID * TEXTURE_SIZE * TEXTURE_SIZE * 3;

            int tx = ((int)floorx & (TEXTURE_SIZE - 1));
            int ty = ((int)floory & (TEXTURE_SIZE - 1));

            int pixel = (ty * TEXTURE_SIZE + tx) * 3 + floorOffset;

            glColor3ub(
                FirstTextures[pixel + 0] * 0.7,
                FirstTextures[pixel + 1] * 0.7,
                FirstTextures[pixel + 2] * 0.7
            );

            glPointSize(8);
            glBegin(GL_POINTS);
            glVertex2i(r * 8, y);
            glEnd();

            //Ceiling
            int ceilID = mapC[fmy * mapX + fmx];
            if (ceilID > 0) {

                int ceilOffset = ceilID * TEXTURE_SIZE * TEXTURE_SIZE * 3;
                pixel = (ty * TEXTURE_SIZE + tx) * 3 + ceilOffset;

                glColor3ub(
                    FirstTextures[pixel + 0],
                    FirstTextures[pixel + 1],
                    FirstTextures[pixel + 2]
                );

                glPointSize(8);
                glBegin(GL_POINTS);
                glVertex2i(r * 8, 640 - y);
                glEnd();
            }
        }


        // next ray
        ra += 0.5 * DR;
        if (ra < 0)       ra += 2 * PI;
        if (ra > 2 * PI)  ra -= 2 * PI;
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
    //drawSky();
    //drawRays3D();
    //drawSprite();
    //drawPlayer();
    glfwSwapBuffers(window);
}

void resize(GLFWwindow* window, int w, int h) {
    glfwSetWindowSize(window, 960, 640);
}

void StateScreens(int v) {
    int x, y;
    int *T;
    if (v==1) {T=StartScreen;}
    if (v==2) {T=WinScreen;}
    if (v==3) {T=GameOverScreen;}
    for (y = 0; y < 80; y++) {
        for (x = 0; x < 120; x++) {
            int pixel = (y*120+x)*3;
            int red = T[pixel+0];
            int green = T[pixel+1];
            int blue = T[pixel+2];
            glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
        }
    }
}

bool randBool() {
    static random_device ranD;
    static mt19937 gen(ranD());
    static uniform_int_distribution<> dis(0, 1);
    return dis(gen) == 1;
}

void mapInit() {
    mapTex = randBool() + 1;
    level = new Level(mapX, mapY, mapTex);
    cout << *level;

    px = level->getPlayerPos().x;
    py = level->getPlayerPos().y;

    sp[0].type=1;
    sp[0].state=1;
    sp[0].map=0;
    //sp[0].x=1.5*64;
    //sp[0].y=5*64;
    sp[0].x=level->getKeyPos().x;
    sp[0].y=level->getKeyPos().y;
    sp[0].z=-2*64;

    sp[1].type=1;
    sp[1].state=1;
    sp[1].map=2;
    //sp[1].x=4.5*64;
    //sp[1].y=6*64;
    sp[1].x=level->getEnemyPos().x;
    sp[1].y=level->getEnemyPos().y;
    sp[1].z=-2*64;

    for (int i = 0; i < mapS; i++) {
        mapW[i] = level->getCell(i);
        mapF[i] = mapTex-1;
        mapC[i] = mapTex-1;
    }
}

void init() {
    glClearColor(0.3, 0.3, 0.3, 0);
    //glOrtho(0, 960, 640, 0, -1, 1);
    //px = 300;
    //py = 300;
    pdx = cos(pa)*5;
    pdy = sin(pa)*5;
}

void render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //drawMap2D();
    if (gameState == 0) {init(); startingGame = false; gameState=1;} //initialize game
    if (gameState == 1) {
        StateScreens(1);
        timer = 0;
        if (startingGame) {
            startingGame = false;
            clearedLevel = false;
            mapInit();
            gameState=2;
            cout << "Starting!" << endl;
        }
    } //start screen
    if (gameState == 2) { //game
        drawSky();
        drawRays3D();
        drawSprite();
        if (clearedLevel) {
            gameState == 3;
            clearedLevel = false;
        }
    }
    if (gameState == 3){StateScreens(2); timer+=1*fps; if (timer>30000){delete level; pdx = cos(pa)*5; pdy = sin(pa)*5; gameState=1;}}
    if (gameState == 4){StateScreens(3); timer+=1*fps; if (timer>30000){delete level; pdx = cos(pa)*5; pdy = sin(pa)*5; gameState=1;}}
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

    glOrtho(0, 960, 640, 0, -1, 1);
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