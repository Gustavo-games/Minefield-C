#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "Tile.h"

#define WIDTH 640
#define HEIGHT 420
#define Grey SDL_MapRGBA(canva->format, 120, 120, 122, 255)
#define Green SDL_MapRGBA(canva->format, 0, 160, 0, 255)
#define BLACK SDL_MapRGBA(canva->format, 0, 0, 0, 255)

// Global vars
bool running = true;
bool pause = false;
bool WIN = false;
bool LOSE = false;
int FreeTiles = 0;
int FreedTiles = 0;

void DisplayScore(SDL_Renderer *canva, char text[], SDL_Rect rect, TTF_Font *font, SDL_Color color){
    SDL_Surface *number = TTF_RenderText_Solid(font, text, color);
    if(number == NULL){
        printf("Could not create SDL_Surface\n: %s", SDL_GetError());
        return;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(canva, number);
    SDL_RenderCopy(canva, tex, NULL, &rect);
    SDL_FreeSurface(number);
    SDL_DestroyTexture(tex);
}

void render(SDL_Renderer *canva, Tile **tiles, TTF_Font *font){

    SDL_Color ColorBlack = {0,0,0,255};
    SDL_Color ColorWhite = {255,255,255,255};
    SDL_Color ColorBlue = {100,100,255,255};
    SDL_Color ColorRed = {255,0,0,255};

    SDL_Rect middleText = {(WIDTH/2)-128, (HEIGHT/2)-64, 128*2, 64*2};
    SDL_Rect BIGGERmiddleText = {(WIDTH/2)-128*2, (HEIGHT/2)-64, 128*4, 128};
    SDL_Rect TopText = {0+32, 4, 24*4, 24};


    // Background + Field
    SDL_SetRenderDrawColor(canva, 120, 120, 122, 255);
    SDL_RenderClear(canva);

    SDL_Rect field;
    
    field.x = TILE_SIZE;
    field.y = TILE_SIZE;
    field.w = WIDTH-64;
    field.h = HEIGHT-68;

    SDL_SetRenderDrawColor(canva, 0, 190, 0, 255);
    SDL_RenderFillRect(canva, &field);

    SDL_SetRenderDrawColor(canva, 255, 255, 255, 255);
    
    RenderTiles(&canva, tiles, WIDTH-32, HEIGHT-32);

    char score[12];
    snprintf(score, 12, "%d / %d", FreeTiles, FreedTiles);

    DisplayScore(canva, score, TopText,font, ColorBlack);

    
    if(WIN){
        char *state = "YOU WIN!!";
        DisplayScore(canva, state, middleText, font, ColorBlue);
    }
    if(LOSE){
        char *state = "YOU LOSE!!";
        DisplayScore(canva, state, BIGGERmiddleText, font, ColorRed);    
    }


    SDL_RenderPresent(canva);

}

void startGame(SDL_Renderer *canva, SDL_Event event){
    
    TTF_Font *font = TTF_OpenFont("Tiny5-Regular.ttf", 128);

    int SpaceWidth = WIDTH-32;
    int SpaceHeight = HEIGHT-32;
    int FirstClick = true;
    int restartGame = false;
    
    SDL_AudioSpec spec;
    Uint8 *audio_buf;
    Uint32 audio_len;
    
    // Load explosion sound 
    SDL_LoadWAV("Explosion.wav", &spec, &audio_buf, &audio_len);
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    // 8 the directions around the tile and the tile itself
    int ClosePosX[9] = {-1,  0,  1, -1,  1, -1,  0,  1, 0};
    int ClosePosY[9] = {-1, -1, -1,  0,  0,  1,  1,  1, 0};

    Tile** tiles = CreateTiles(&canva, WIDTH-32, HEIGHT-32, &FreeTiles);

    // To win the game you free every non mine tiles

    while(running){
        while(SDL_PollEvent(&event)){
            

            if(event.type == SDL_QUIT){
                running = 0;
            }

            // Logic for playing the game
            if(!pause){
                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                    running = 0;
                }

                // Loop starts with 1 to not used invalid tiles at the border
                if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
                    for(int i = 1; i < SpaceWidth/TILE_SIZE; i++){
                        for(int e = 1; e < SpaceHeight/TILE_SIZE; e++){
                            if(event.button.x > tiles[i][e].rect.x && event.button.x < tiles[i][e].rect.x+TILE_SIZE &&
                            event.button.y < tiles[i][e].rect.y+TILE_SIZE && event.button.y > tiles[i][e].rect.y)
                            {
                                // Creates safe area
                                if(FirstClick){
                                    for(int o = 0; o < 9; o++){
                                        int detectingPosX = i+ClosePosX[o];
                                        int detectingPosY = e+ClosePosY[o];

                                        if((detectingPosX > 0 && detectingPosY > 0 && detectingPosX < (SpaceWidth/TILE_SIZE) && detectingPosY < (SpaceHeight/TILE_SIZE))){
                                            if(tiles[detectingPosX][detectingPosY].mine == false) FreedTiles++;
                                            tiles[detectingPosX][detectingPosY].state = FREE;
                                            tiles[detectingPosX][detectingPosY].mine = false;
                                        }
                                    }
                                    DetectMines(SpaceWidth, SpaceHeight , tiles, &canva);
                                    FirstClick = false;
                                }else{
                                    if(tiles[i][e].mine){
                                        RevealTiles(tiles, 1, 1, SpaceWidth, SpaceHeight);
                                        LOSE = true;
                                        SDL_QueueAudio(device, audio_buf, audio_len);
                                        SDL_PauseAudioDevice(device, 0);
                                        pause = 1;
                                    }else if(tiles[i][e].state == NORMAL){
                                        tiles[i][e].state = FREE;
                                        FreedTiles+=1;
                                    }
                                }
                            }
                        }
                    }
                }
            }else{
                if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
                   restartGame = true;
                   break;
                }
            }
            if(FreeTiles <= FreedTiles){
                WIN = true;
                pause = true;
            }
        }           
        
        render(canva, tiles, font);
        if(restartGame) break;
    }

    if(restartGame){
        running = true;
        restartGame = false;
    }
    // Frees and resets variables after end of the game
    // First destroy any per-tile NumberOfMines textures
    for (int i = 0; i < SpaceWidth / TILE_SIZE; ++i) {
        for (int e = 0; e < SpaceHeight / TILE_SIZE; ++e) {
            if (tiles[i][e].NumberOfMines != NULL) {
                SDL_DestroyTexture(tiles[i][e].NumberOfMines);
                tiles[i][e].NumberOfMines = NULL;
            }
        }
    }

    // Destroy the three shared tile textures once (they're shared across tiles)
    if (SpaceWidth / TILE_SIZE > 0 && SpaceHeight / TILE_SIZE > 0) {
        SDL_Texture *shared0 = tiles[0][0].tex[0];
        SDL_Texture *shared1 = tiles[0][0].tex[1];
        SDL_Texture *shared2 = tiles[0][0].tex[2];
        if (shared0) SDL_DestroyTexture(shared0);
        if (shared1) SDL_DestroyTexture(shared1);
        if (shared2) SDL_DestroyTexture(shared2);
    }

    // Free tile rows and the container
    for (int i = 0; i < SpaceWidth / TILE_SIZE; ++i) {
        free(tiles[i]);
    }
    free(tiles);

    SDL_FreeWAV(audio_buf);
    SDL_CloseAudioDevice(device);
    TTF_CloseFont(font);

    WIN = false;
    LOSE = false;
    FreedTiles = 0;
    FreeTiles = 0;
    pause = false;

    printf("Terminando Jogo");
}

int main(){
    srand(time(NULL));
    
    SDL_Event event;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1){
        printf("SDL could not initialize\n: %s", SDL_GetError());
        exit(-1);
    }
    if(TTF_Init() == -1){
        printf("TFF could not initialize\n: %s", TTF_GetError());
        exit(-1);
    }
    

    printf("SDL was initialized\n");
 
    // Start Game, Window and renderer 
    
    SDL_Window *win = SDL_CreateWindow("Minefield",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS);
    if(win == NULL){
        printf("SDL_Window could not initialize\n: %s", SDL_GetError());
        exit(-1);
    }

    SDL_Renderer *canva = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if(canva == NULL){
        printf("SDL_Renderer could not initialize\n: %s", SDL_GetError());
        exit(-1);
    }

    while(running){
        startGame(canva, event);
    }

    SDL_DestroyRenderer(canva);
    SDL_DestroyWindow(win);
    SDL_Quit();
}