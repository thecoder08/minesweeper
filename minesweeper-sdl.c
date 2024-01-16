#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
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

SDL_Renderer* renderer;

void drawCell(int x, int y, unsigned char cell) {
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            int color = textures[((cell/4*40)+i)*160 + (cell%4*40)+j];
            SDL_SetRenderDrawColor(renderer, (color & 0x000000ff), (color & 0x0000ff00) >> 8, (color & 0x00ff0000) >> 16, (color & 0xff000000) >> 24);
            SDL_RenderDrawPoint(renderer, x*40 + j, y*40 + i);
        }
    }
}

void pointermotion(int pointerX, int pointerY) {
    cellX = pointerX/40;
    cellY = pointerY/40;
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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL failed to initialize\n");
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    if (window == NULL) {
        fprintf(stderr, "SDL failed to create window\n");
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "SDL failed to create renderer\n");
        return 1;
    }

    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
            if (event.type == SDL_MOUSEMOTION) {
                pointermotion(event.motion.x, event.motion.y);
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                pointerbutton(event.button.button, 1);
            }
        }
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 16; j++) {
                drawCell(j, i, cells[i*16+j]);
            }
        }
        SDL_RenderPresent(renderer);
    }
}