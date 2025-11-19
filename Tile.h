#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define TILE_SIZE 32

enum{
    BOMB = -2,
    NORMAL = -1,
    FREE = 0    
};

typedef struct {
    SDL_Texture *tex[3];
    SDL_Texture *NumberOfMines;
    SDL_Rect rect;
    int closeMines;
    bool mine;
    int state;

}Tile;

void DetectMines(int width, int height, Tile **tiles, SDL_Renderer **canva);

// TODO: width and height should not be parameters
Tile **CreateTiles(SDL_Renderer **canva, int width, int height, int *AmountOfFreeTiles){

    
    SDL_Surface *tile = IMG_Load("Tile.png");
    SDL_Surface *tile2 = IMG_Load("TileBomb.png");
    SDL_Surface *tile3 = IMG_Load("TileFree.png");

    if(!(tile || tile2 || tile3)){
        printf("Missing images");
        exit(1);
    }

    SDL_Texture *tileTex[3] = {
        SDL_CreateTextureFromSurface(*canva, tile),
        SDL_CreateTextureFromSurface(*canva, tile2),
        SDL_CreateTextureFromSurface(*canva, tile3)
    };
    
    SDL_FreeSurface(tile);
    SDL_FreeSurface(tile2);
    SDL_FreeSurface(tile3);

    Tile **tiles = (Tile**)malloc(sizeof(Tile*)*(width/TILE_SIZE));

    for(int i = 0; i < width/TILE_SIZE; i++){
        
        tiles[i] = (Tile*)malloc(sizeof(Tile)*(height/TILE_SIZE));
        
        for(int e = 0; e < height/TILE_SIZE; e++){
            SDL_Rect rect;

            int randMine =  rand() % 100;

            rect.x = i*TILE_SIZE;
            rect.y = e*TILE_SIZE;
            rect.h = TILE_SIZE;
            rect.w = TILE_SIZE;

            tiles[i][e].rect = rect;
            tiles[i][e].state = NORMAL;
            tiles[i][e].NumberOfMines = NULL;
            tiles[i][e].mine = false;
            tiles[i][e].closeMines = 0;
            
            if(randMine < 20){
                tiles[i][e].mine = true;
            }else if(e != 0 && i != 0){
                (*AmountOfFreeTiles)++;
            }

            // Saves texture on the struct
            tiles[i][e].tex[0] = tileTex[0];
            tiles[i][e].tex[1] = tileTex[1];
            tiles[i][e].tex[2] = tileTex[2];     
        }
    }

    // Detects mines close to this tile and put the quantity in the closeMines variable
    DetectMines(width, height, tiles, canva);

    return tiles;
}

void DetectMines(int width, int height, Tile **tiles, SDL_Renderer **canva)
{
    TTF_Font *font = TTF_OpenFont("Tiny5-Regular.ttf", 128);
    SDL_Color BLACK = {0, 0, 150, 255};

    char Text[4];

    for (int i = 0; i < width / TILE_SIZE; i++)
    {
        for (int e = 0; e < height / TILE_SIZE; e++)
        {
             tiles[i][e].closeMines = 0;
            int ClosePosX[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
            int ClosePosY[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
            for (int o = 0; o < 8; o++)
            {

                int detectingPosX = i + ClosePosX[o];
                int detectingPosY = e + ClosePosY[o];

                if ((detectingPosX > 0 && detectingPosY > 0 && detectingPosX < (width / TILE_SIZE) && detectingPosY < (height / TILE_SIZE)))
                {

                    if (tiles[detectingPosX][detectingPosY].mine)
                    {
                        tiles[i][e].closeMines += 1;
                    }
                }
            }

           // Segmentation Fault
            if(tiles[i][e].NumberOfMines){
                SDL_DestroyTexture(tiles[i][e].NumberOfMines);
            }

            if (tiles[i][e].closeMines != 0)
            {
                snprintf(Text, 4, "%d", tiles[i][e].closeMines);
                SDL_Surface *number = TTF_RenderText_Solid(font, Text, BLACK);
                tiles[i][e].NumberOfMines = SDL_CreateTextureFromSurface(*canva, number);
                SDL_FreeSurface(number);
            }
            else
            {
                tiles[i][e].NumberOfMines = NULL;
            }
        }
    }

    TTF_CloseFont(font);
}

void RenderTiles(SDL_Renderer **canva,Tile **tiles, int width, int height){
    for(int i = 1; i < width/TILE_SIZE; i++){
        for(int e = 1; e < height/TILE_SIZE; e++){
            switch (tiles[i][e].state)
            {
                case BOMB:
                    SDL_RenderCopy(*canva, tiles[i][e].tex[1], NULL, &tiles[i][e].rect);
                    break;
                case FREE:
                    SDL_RenderCopy(*canva, tiles[i][e].tex[2], NULL, &tiles[i][e].rect);
                    SDL_RenderCopy(*canva, tiles[i][e].NumberOfMines, NULL, &tiles[i][e].rect);
                    break;
                case NORMAL:
                    SDL_RenderCopy(*canva, tiles[i][e].tex[0], NULL, &tiles[i][e].rect);
                    break;
            }
        }
    }
}

void RevealTiles(Tile **tiles, int x, int y, int width, int height){
    for(int i = x; i < width/TILE_SIZE; i++){
        for(int e = y; e < height/TILE_SIZE; e++){
            if(tiles[i][e].mine){
                tiles[i][e].state = BOMB;
            }else{
                tiles[i][e].state = FREE;
            }
        }
    }
}
