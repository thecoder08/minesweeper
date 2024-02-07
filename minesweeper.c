#include <xgfx/drawing.h>
#include <xgfx/window.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// 16x12 board, cells are 40x40
unsigned char cells[16*12];
unsigned char mines[16*12];
int textures[76800];

int cellX;
int cellY;
int done = 0;

int correctColor(int color) {
    return ((color & 0x00ff0000) >> 16) + ((color & 0x000000ff) << 16) + (color & 0xff00ff00);
}

void drawCell(int x, int y, unsigned char cell) {
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            plot(x*40 + j, y*40 + i, correctColor(textures[((cell/4*40)+i)*160 + (cell%4*40)+j]));
        }
    }
}

void paint() {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 16; j++) {
            drawCell(j, i, cells[i*16+j]);
        }
    }
}

void findSurroundingMines(int x, int y) {
    // count mines around current cell
    int mineCount = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (((x+j)>-1) && ((x+j)<16) && ((y+i)>-1) && ((y+i)<12) && !(i==0 && j==0)) {
                if (mines[(y+i)*16+(x+j)]) {
                    mineCount++;
                }
            }
        }
    }
    cells[y*16+x] = mineCount;
    // if mine count is zero, reveal surrounding cells if unrevealed
    if (mineCount == 0) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (((x+j)>-1) && ((x+j)<16) && ((y+i)>-1) && ((y+i)<12) && !(i==0 && j==0)) {
                    if (cells[(y+i)*16+(x+j)] == 9 || cells[(y+i)*16+(x+j)] == 10) {
                        findSurroundingMines(x+j, y+i);
                    }
                }
            }
        }
    }
}

void pointerbutton(unsigned int button, unsigned int state) {
    if (done) {
        return;
    }
    if (button == 1 && state == 1 && cells[cellY*16+cellX] == 10) {
        if (mines[cellY*16+cellX]) {
            for (int i = 0; i < 192; i++) {
                if (mines[i]) {
                    cells[i] = 11;
                }
            }
            printf("You lose!\n");
            done = 1;
            return;
        }
        else {
            findSurroundingMines(cellX, cellY);
        }
        done = 1;
        for (int i = 0; i < 192; i++) {
            if (mines[i] == 0) {
                if (cells[i] == 9 || cells[i] == 10) {
                    done = 0;
                    break;
                }
            }
        }
        if (done) {
            printf("You win!\n");
            return;
        }
    }
    if (button == 3 && state == 1) {
        if (cells[cellY*16+cellX] == 10) {
            cells[cellY*16+cellX] = 9;
        }
        else if (cells[cellY*16+cellX] == 9) {
            cells[cellY*16+cellX] = 10;
        }
    }
}

int main() {
    srand(time(NULL));
    for (int i = 0; i < 192; i++) {
        cells[i] = 10;
    }
    for (int i = 0; i < 20; i++) {
        mines[(rand()%12)*16+(rand()%16)] = 1;
    }
    int texturesFd = open("textures.data", O_RDONLY);
    if (texturesFd == -1) {
        fprintf(stderr, "Failed to open textures!\n");
        return 1;
    }
    read(texturesFd, textures, 76800);
    close(texturesFd);
    initWindow(640, 480, "Minesweeper");
    while(1) {
        Event event;
        while (checkWindowEvent(&event)) {
            if (event.type == WINDOW_CLOSE) {
                return 0;
            }
            if (event.type == MOUSE_MOVE) {
                cellX = event.mousemove.x/40;
                cellY = event.mousemove.y/40;
            }
            if (event.type == MOUSE_BUTTON) {
                pointerbutton(event.mousebutton.button, event.mousebutton.state);
            }
        }
        paint();
        updateWindow();
    }
}
