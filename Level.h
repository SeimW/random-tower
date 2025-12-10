//
// Created by seimw on 12/9/2025.
//

#ifndef RANDOMTOWER_LEVEL_H
#define RANDOMTOWER_LEVEL_H

#include <iostream>
#include <cstdlib>
#include <queue>
#include <algorithm>
#include <random>

struct ObjPosition {
    int x,y;
    bool exists; //needed because key may not exist
};

class Level {
    public:
        Level(int width, int height, int walltex);
        ~Level();
        int idx(int x, int y) const {return y * WIDTH + x;}
        void carve(int x, int y);
        bool findPath(int start, int goal, int outPath[], int &outSize);
        void getDistances(int dist[], int cell);
        int countNeighbors(int cell, int ignore = -1);
        void placeObjects();
        int getCell(int x) const {return maze[x];}
        void print(std::ostream &os) const;

        //getters for object positions
        ObjPosition getPlayerPos() const;
        ObjPosition getKeyPos() const;
        ObjPosition getEnemyPos() const;
    private:
        const int NONE = 0, GOAL = 6, DOOR = 4;
        const int OBJ_NONE = -1, OBJ_KEY = 7, OBJ_ENEMY = 8, OBJ_PLAYER = 9;
        //wall may have different textures so it can change (should be between 1-3 or 5)
        //allow for adjustment of level capacity (but main will always call it 15x15)
        int WALL, WIDTH, HEIGHT, SIZE;
        //dx and dy allow for traversial in the 2D space the 1D array represents
        //ex: (dx[0], dy[0]) or (1, 0), moves right, (dx[3], dy[3]) or (0, 1), moves down
        int dx[4] = {1, -1, 0, 0};
        int dy[4] = {0, 0, 1, -1};
        int* maze = nullptr;
        int* objects = nullptr;
        std::mt19937 rng;
};

inline std::ostream &operator <<(std::ostream &os, const Level &l) {l.print(os); return os;}

#endif //RANDOMTOWER_LEVEL_H

inline Level::Level(int width, int height, int walltex) {
    WIDTH = width;
    HEIGHT = height;
    WALL = walltex;
    SIZE = WIDTH * HEIGHT;
    maze = new int[SIZE];
    objects = new int[SIZE];
    rng.seed(std::random_device()());

    for (int i = 0; i < SIZE; i++) {
        maze[i] = WALL;
    }

    carve(1, 1);
    placeObjects();
}

inline Level::~Level() {
    delete[] maze;
    delete[] objects;
    maze = nullptr;
    objects = nullptr;
}

//carve() carves out a path starting from the given cell
inline void Level::carve(int x, int y) {
    maze[idx(x, y)] = NONE;

    int dirs[4] = {0, 1, 2, 3};
    std::shuffle(dirs, dirs+4, rng);

    for (int i = 0; i < 4; i++) {
        int dir = dirs[i];
        int nextx = x + dx[dir] * 2;
        int nexty = y + dy[dir] * 2;

        if (nextx <= 0 || nexty <= 0 || nextx >= WIDTH-1 || nexty >= HEIGHT-1) {
            continue;
        }

        //all cells are WALLS by default, so if it's a wall, we can carve it
        //carves the wall between current x and the next cell at nextx and nexty
        if (maze[idx(nextx, nexty)] == WALL) {
            maze[idx(x + dx[dir], y + dy[dir])] = NONE;
            carve(nextx, nexty);
        }
    }
}

inline void Level::getDistances(int dist[], int cell) {
    for (int i = 0; i < SIZE; i++) {dist[i] = -1;}

    std::queue<int> q;
    q.push(cell);
    dist[cell] = 0;

    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        int x = curr % WIDTH;
        int y = curr / WIDTH;

        for (int d = 0; d < 4; d++) {
            int nextx = x + dx[d];
            int nexty = y + dy[d];

            if (nextx < 0 || nexty < 0 || nextx >= WIDTH || nexty >= HEIGHT) {
                continue;
            }

            int nextindex = idx(nextx, nexty);
            if (maze[nextindex] == WALL) {continue;}
            if (dist[nextindex] != -1) {continue;}

            dist[nextindex] = dist[curr] + 1;
            q.push(nextindex);
        }
    }
}

inline int Level::countNeighbors(int cell, int ignore) {
    int x = cell % WIDTH;
    int y = cell / WIDTH;
    int c = 0;

    for (int d = 0; d < 4; d++) {
        int nextx = x + dx[d];
        int nexty = y + dy[d];
        if (nextx < 0 || nexty < 0 || nextx >= WIDTH || nexty >= HEIGHT) {continue;}

        int ni = idx(nextx, nexty);
        if (ni == ignore) {continue;}
        if (maze[ni] != WALL) {c++;}
    }

    return c;
}

inline void Level::placeObjects() {
    bool success = false;

    while (!success) {
        //Remove all objects
        for (int i = 0; i < SIZE; i++) {
            objects[i] = OBJ_NONE;
            if (maze[i] == DOOR || maze[i] == GOAL) {
                maze[i] = NONE;
            }
        }

        //Place player
        int start;
        do {start = rng() % SIZE;} while (maze[start] != NONE);
        objects[start] = OBJ_PLAYER;

        //Place door
        int door;
        do {door = rng() % SIZE;} while (maze[door] != NONE);
        maze[door] = DOOR;

        //Check if path exists. If not, retry while loop
        int path[SIZE], pLength;
        if (!findPath(start, door, path, pLength)) {
            objects[start] = OBJ_NONE;
            maze[door] = NONE;
            continue;
        }

        //Place key
        int temp = maze[door];
        maze[door] = WALL; //block door for now
        int dist[SIZE];
        getDistances(dist, start);
        maze[door] = temp;

        int keyCandidate = -1, bestMetric = -1;

        for (int i = 0; i < SIZE; i++) {
            if (maze[i] != NONE) {continue;}
            if (dist[i] == -1) {continue;}
            if (i == start || i == door) {continue;}

            int x = i % WIDTH, y = i / WIDTH;
            int dxDoor = abs((door%WIDTH) - x);
            int dyDoor = abs((door/WIDTH) - y);
            int distFromDoor = dxDoor + dyDoor;

            int neighbors = countNeighbors(i, door);
            bool deadEnd = (neighbors == 1);

            int metric = distFromDoor;
            if (deadEnd) {metric += 100;};

            if (metric > bestMetric) {
                bestMetric = metric;
                keyCandidate = i;
            }

        }

        //there are mazes that may not be able to appropiately have a key and a door
        //in those circumstances, generate a maze without either
        if (keyCandidate == -1) {
            maze[door] = NONE;
            success = true;
            continue;
        }

        objects[keyCandidate] = OBJ_KEY;

        //Make goal far away from player

        getDistances(dist, start);

        int goal = -1;
        int bestDist = -1;
        for (int i = 0; i < SIZE; i++) {
            //goal must be placed on empty space
            if (maze[i] != NONE || objects[i] != OBJ_NONE) {continue;}
            if (dist[i] > bestDist) {
                bestDist = dist[i];
                goal = i;
            }
        }

        if (goal == -1) {continue;} //try again

        maze[goal] = GOAL;

        //Keep enemy a good distance from player and goal

        int distGoal[SIZE];
        getDistances(distGoal, goal);

        int bestScore = -1;
        int enemy = -1;

        for (int i = 0; i < SIZE; i++) {
            if (maze[i] != NONE) {continue;}
            if (objects[i] != OBJ_NONE) {continue;}

            if (dist[i] == -1 || distGoal[i] == -1) {continue;}

            int score = std::min(dist[i], distGoal[i]);
            if (score > bestScore) {
                bestScore = score;
                enemy = i;
            }
        }

        if (enemy != -1) {
            objects[enemy] = OBJ_ENEMY;
        }

        success = true; //All objects placed well!
    }
}

//reconstruct the path through breadth-first search
inline bool Level::findPath(int start, int goal, int outPath[], int &outSize) {
    int previous[SIZE];
    for (int i = 0; i < SIZE; i++) {
        previous[i] = -1;
    }

    std::queue<int> q;
    q.push(start);
    previous[start] = start;

    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        //if we get to the goal, we're done
        if (curr == goal) {break;}
        int x = curr % WIDTH;
        int y = curr / WIDTH;
        for (int d = 0; d < 4; d++) {
            int nextx = x + dx[d];
            int nexty = y + dy[d];

            //check if neighbour cell is within bounds
            if (nextx < 0 || nexty < 0 || nextx >= WIDTH || nexty >= HEIGHT) {
                continue;
            }

            int nextdex = idx(nextx, nexty);

            //if neighbour is a wall, skip
            if (maze[nextdex] == WALL) {continue;}
            //if we visited the cell before, skip
            if (previous[nextdex] != -1) {continue;}

            //mark preceding cell and add to queue
            previous[nextdex] = curr;
            q.push(nextdex);

        }
    }

    //goal not set, return false
    if (previous[goal] == -1) {return false;}

    //walk backwards from goal to the start with the prev[] chain
    int temp[SIZE];
    int count = 0;
    int curr = goal;

    while (curr != start) {
        temp[count++] = curr;
        curr = previous[curr];
    }

    //add to start node itself
    temp[count++] = start;

    //reverse into outpath so it can be a path from the start to the goal
    outSize = count;
    for (int i = 0; i < count; i++) {outPath[i] = temp[count - 1 - i];}

    return true;
}

inline ObjPosition Level::getPlayerPos() const {
    for (int i = 0; i < SIZE; i++) {
        if (objects[i] == OBJ_PLAYER) {
            return {(i % WIDTH) * 64 + 32, (i / WIDTH) * 64 + 32, true};
        }
    }
    return {-1, -1, false};
}

inline ObjPosition Level::getKeyPos() const {
    for (int i = 0; i < SIZE; i++) {
        if (objects[i] == OBJ_KEY) {
            return {(i % WIDTH) * 64 + 32, (i / WIDTH) * 64 + 32, true};
        }
    }
    return {-1, -1, false};
}

inline ObjPosition Level::getEnemyPos() const {
    for (int i = 0; i < SIZE; i++) {
        if (objects[i] == OBJ_ENEMY) {
            return {(i % WIDTH) * 64 + 32, (i / WIDTH) * 64 + 32, true};
        }
    }
    return {-1, -1, false};
}

inline void Level::print(std::ostream &os) const {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int i = idx(x, y);
            if ((objects[i]) == OBJ_PLAYER) std::cout << "P ";
            else if (objects[i] == OBJ_KEY) std::cout << "K ";
            else if (objects[i] == OBJ_ENEMY) std::cout << "E ";
            else if (maze[i] == WALL) std::cout << "# ";
            else if (maze[i] == DOOR) std::cout << "D ";
            else if (maze[i] == GOAL) std::cout << "G ";
            else std::cout << ". ";
        }
        std::cout << std::endl;
    }
}
