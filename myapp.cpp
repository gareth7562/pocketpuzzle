#include <SDL/SDL.h>
#include <stdio.h>
#include <list>
#include <signal.h>
#include <string>
#include <SDL/SDL_ttf.h>

SDL_Surface *screen = NULL;
SDL_Surface *bg = NULL;
SDL_Rect bgRect;
int running = 0;
int called = 0;

#define width 7
#define height 16
#define NUM_BLOCKS 3

#define BUTTON_A SDLK_LCTRL
#define BUTTON_SELECT SDLK_ESCAPE
#define BUTTON_R SDLK_BACKSPACE

#define INITIAL_SPEED 300
#define FAST_SPEED 100
#define PADDING 20
#define VERSION_TEXT "POCKET PUZZLE V0.1"

const int screen_width = 320;
const int screen_height = 240;

int score = 0;
const int points_big = 10;
const int points_small = 4;

SDL_Surface *loadbmp(std::string file_name);
int blockCount();
void checkForLines();
void removeBlocks();

std::list<int> full_columns;

typedef struct
{
    std::string dirname = "assets/";
    std::string file_block_image_red = dirname + "redblock.bmp";
    std::string file_block_image_green = dirname + "greenblock.bmp";
    std::string file_block_image_blue = dirname + "blueblock.bmp";
    std::string file_block_image_yellow = dirname + "yellowblock.bmp";
    std::string file_block_image_orange = dirname + "orangeblock.bmp";
    std::string file_block_image_white = dirname + "whiteblock.bmp";

} resources;

resources Resource;

class Bitmaps
{
public:
    SDL_Surface *bitmap_yellow = loadbmp(Resource.file_block_image_yellow);
    SDL_Surface *bitmap_red = loadbmp(Resource.file_block_image_red);
    SDL_Surface *bitmap_blue = loadbmp(Resource.file_block_image_blue);
    SDL_Surface *bitmap_green = loadbmp(Resource.file_block_image_green);
    SDL_Surface *bitmap_orange = loadbmp(Resource.file_block_image_orange);
    SDL_Surface *bitmap_white = loadbmp(Resource.file_block_image_white);
};

Bitmaps bitmap_resources;

SDL_Surface *loadbmp(char *file_name);

class block
{
public:
    int color;
    int x;
    int y;
    bool placed = false;
    SDL_Surface *image;

    block()
    {
        x = 0;
        y = 0;
        color = 1;
        placed = false;
        image = loadbmp(Resource.file_block_image_yellow);
    }
};

typedef struct
{
    int x;
    int y;
    int value;

} array_index;

int playArea[width][height];
block newBlock[NUM_BLOCKS];

int shuffleCount = 0;

SDL_Surface *loadbmp(std::string file_name)
{
    SDL_Surface *image;

    /* Load the BMP file into a surface */
    image = SDL_LoadBMP(file_name.c_str());
    if (image == NULL)
    {
        fprintf(stderr, "Couldn't load %s: %s\n", file_name, SDL_GetError());
        return NULL;
    }

    return image;
}

int drawText(SDL_Surface *screen, TTF_Font *font, const char *text, int x, int y)
{
    SDL_Color color = {255, 255, 255};
    SDL_Surface *text_surface;
    SDL_Rect dst_rect;

    dst_rect.x = x;
    dst_rect.y = y;

    text_surface = TTF_RenderText_Solid(font, text, color);
    if (text_surface != NULL)
    {
        SDL_BlitSurface(text_surface, NULL, screen, &dst_rect);
        SDL_FreeSurface(text_surface);
        return 1;
    }
    else
    {
        // report error
        return 0;
    }
}

void initPlayArea()
{
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {

            playArea[x][y] = 0;
        }
    }
}

void createBlock(int xPos)
{

    SDL_Surface *bitmap = NULL;
    std::list<int> empty_columns;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {

        newBlock[i].color = (rand() % 5) + 1;

        switch (newBlock[i].color)
        {
        case 1:
            bitmap = bitmap_resources.bitmap_red;
            break;

        case 2:
            bitmap = bitmap_resources.bitmap_green;
            break;

        case 3:
            bitmap = bitmap_resources.bitmap_blue;
            break;

        case 4:
            bitmap = bitmap_resources.bitmap_yellow;
            break;

        case 5:
            bitmap = bitmap_resources.bitmap_orange;
            break;
        }

        newBlock[i].image = bitmap;

        newBlock[i].x = xPos;
        newBlock[i].y = i * 16 - NUM_BLOCKS * 16;
        newBlock[i].placed = false;
    }
}

void blockMoveLeft()
{

    for (int i = 0; i < NUM_BLOCKS; i++)
    {

        if (playArea[newBlock[2].x / 16 - 1][newBlock[2].y / 16] <= 0)
        {
            if (newBlock[i].x > 0)
                newBlock[i].x -= 16;
        }
    }
}

void blockMoveRight()
{

    for (int i = 0; i < NUM_BLOCKS; i++)
    {

        if (playArea[newBlock[2].x / 16 + 1][newBlock[2].y / 16] <= 0)
        {
            if (newBlock[i].x < width * 16 - 16)
                newBlock[i].x += 16;
        }
    }
}
void removeBlocks()
{
    int count = 0;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {

            if (y + 1 < height && playArea[x][y + 1] == -1)
            {

                playArea[x][y + 1] = playArea[x][y];
                if (playArea[x][y] > 0)
                {
                    count++;

                    playArea[x][y] = -1;
                }
            }
        }
    }
    if (count > 0)
    {
        removeBlocks();
    }
}

void checkForLines()
{

    for (int color = 1; color < 6; color++)
    {
        for (int x = 0; x < width; x++)
        {

            for (int y = 0; y < height; y++)
            {
                if (x + 2 < width && playArea[x][y] == color && playArea[x + 1][y] == color && playArea[x + 2][y] == color)
                {

                    playArea[x][y] = -1;
                    playArea[x + 1][y] = -1;
                    playArea[x + 2][y] = -1;
                    for (int i = 0; i < width; i++)
                    {
                        if (x + i < width && playArea[x + i][y] == color)
                        {
                            playArea[x + i][y] = -1;
                            score += 8;
                        }
                    }
                    score += 10;
                }

                if (y + 2 < height && playArea[x][y] == color && playArea[x][y + 1] == color && playArea[x][y + 2] == color)
                {

                    playArea[x][y] = -1;
                    playArea[x][y + 1] = -1;
                    playArea[x][y + 2] = -1;

                    for (int i = 3; i < height; i++)
                    {
                        if (y + i < height && playArea[x][y + i] == color)
                        {
                            playArea[x][y + i] = -1;
                            score += points_big;
                        }
                    }
                    score += points_small;
                }

                //diagonals
                if (x - 2 > 0 && y + 2 < height && playArea[x][y] == color && playArea[x - 1][y + 1] == color && playArea[x - 2][y + 2] == color)
                {

                    playArea[x][y] = -1;
                    playArea[x - 1][y + 1] = -1;
                    playArea[x - 2][y + 2] = -1;

                    score += points_big;
                }

                if (y - 2 > 0 && x + 2 < width && playArea[x][y] == color && playArea[x + 1][y - 1] == color && playArea[x + 2][y - 2] == color)
                {

                    playArea[x][y] = -1;
                    playArea[x + 1][y - 1] = -1;
                    playArea[x + 2][y - 2] = -1;

                    score += points_big;
                }

                if (x - 2 > 0 && y - 2 > 0 && playArea[x][y] == color && playArea[x - 1][y - 1] == color && playArea[x - 2][y - 2] == color)
                {

                    playArea[x][y] = -1;
                    playArea[x - 1][y - 1] = -1;
                    playArea[x - 2][y - 2] = -1;

                    score += points_big;
                }

                if (x + 2 < width && y + 2 < height && playArea[x][y] == color && playArea[x + 1][y + 1] == color && playArea[x + 2][y + 2] == color)
                {

                    playArea[x][y] = -1;
                    playArea[x + 1][y + 1] = -1;
                    playArea[x + 2][y + 2] = -1;

                    score += points_big;
                }
            }
        }
    }
}

void shuffleBlocks()
{

    SDL_Surface *temp = newBlock[0].image;
    newBlock[0].image = newBlock[1].image;
    newBlock[1].image = temp;

    int colorTemp = newBlock[0].color;
    newBlock[0].color = newBlock[1].color;
    newBlock[1].color = colorTemp;

    SDL_Surface *temp2 = newBlock[2].image;
    newBlock[2].image = newBlock[0].image;
    newBlock[0].image = temp2;

    int colorTemp2 = newBlock[2].color;
    newBlock[2].color = newBlock[0].color;
    newBlock[0].color = colorTemp2;
}

bool fullBoardCount()
{
    int count = 0;
    int y = 1;
    for (int x = 0; x < width; x++)
    {
            if (playArea[x][y] > 0)
            {
                count++;
            }

    }
    if (count >= 6)
        return true;

    return false;
}

int getFreePosition()
{

    int chosenPosition = (rand() % 6);

    if(playArea[chosenPosition][1] <= 0)
    {
        return chosenPosition*16;
    }
    for(int x = 0; x < width; x++)
    {
        if(playArea[x][1] == 0 || playArea[x][1] == -1)
        {
            return x*16;
        }
    }
    return (rand() % 6) * 16;
}

void moveBlocks()
{

    if (playArea[newBlock[2].x / 16][(newBlock[2].y / 16) + 1] <= 0 && newBlock[2].y < height * 16 - 32)
    {

        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            newBlock[i].y += 16;
        }
    }
    else
    {

        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            if (newBlock[2].y > -16)
            {

                if (newBlock[i].y > -16)
                {
                    playArea[newBlock[i].x / 16][newBlock[i].y / 16] = newBlock[i].color;
                    newBlock[i].placed = true;
                }
            }
        }
    }
}

void drawBitmap(SDL_Surface *image, int x, int y)
{

    SDL_Rect dstRect;

    dstRect.x = x;
    dstRect.y = y;

    /* Blit onto the screen surface */
    if (SDL_BlitSurface(image, NULL, screen, &dstRect) < 0)
        fprintf(stderr, "BlitSurface error: %s\n", SDL_GetError());
}

void quit(int err)
{
    printf("Quitting.\n");
    system("sync");
    SDL_Quit();
    exit(err);
}

Uint32 removeBlocksCallback( Uint32 interval, void* param )
{
    removeBlocks();

    return 0;
}

int main(int argc, char *argv[])
{

    signal(SIGINT, &quit);
    signal(SIGSEGV, &quit);
    signal(SIGTERM, &quit);
    /* Initialize the SDL library */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr,
                "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() < 0)
    {
        fprintf(stderr,
                "Couldn't initialize SDL_TTF: %s\n");
        exit(1);
    }

    /* Clean up on exit */
    atexit(SDL_Quit);
    atexit(TTF_Quit);

    screen = SDL_SetVideoMode(screen_width, screen_height, 16, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "Couldn't set 320x240x16 video mode: %s\n",
                SDL_GetError());
        exit(1);
    }

    TTF_Font *font = TTF_OpenFont("assets/joystix.ttf", 10);

    SDL_Color fontColor = {255, 255, 255};
    bool restart = false;

    while (!restart)
    {


        initPlayArea();
        createBlock((rand() % 6) * 16);

        float ticks = SDL_GetTicks() + INITIAL_SPEED;

        bool fastmode = false;
        bool moveLeft = false;
        bool moveRight = false;

        SDL_ShowCursor(false);
        int speedms = INITIAL_SPEED;
        SDL_Event event;

        running = 1;
        SDL_Surface *textSurface = NULL;
        int text_w, text_h;



        score = 0;
        std::string scoreText = "SCORE: " + std::to_string(score);

        while (running)
        {
            if (fastmode)
            {
                speedms = FAST_SPEED;
            }
            else
            {
                speedms = INITIAL_SPEED;
            }

            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_LEFT:
                        blockMoveLeft();
                        moveLeft = true;
                        break;

                    case SDLK_RIGHT:
                        blockMoveRight();
                        moveRight = true;
                        break;

                    case SDLK_DOWN:
                        fastmode = true;
                        break;

                    case BUTTON_A:
                        shuffleBlocks();
                        break;
                    case BUTTON_SELECT:
                        running = 0;
                        break;
                    case BUTTON_R:
                        restart = true;
                        running = 0;
                        break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_DOWN:
                        fastmode = false;
                        break;
                    case SDLK_RIGHT:
                        moveRight = false;
                        break;

                    case SDLK_LEFT:
                        moveLeft = false;
                        break;
                    }
                    break;
                }
            }

            bgRect.x = PADDING;
            bgRect.y = 0;
            bgRect.w = 112;
            bgRect.h = 240;

            SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

            SDL_FillRect(screen, &bgRect, SDL_MapRGB(screen->format, 18, 14, 80));

            for (int i = 0; i < NUM_BLOCKS; i++)
            {
                drawBitmap(newBlock[i].image, PADDING + newBlock[i].x, newBlock[i].y);
            }

            SDL_Surface *bitmap = bitmap_resources.bitmap_red;
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    switch (playArea[x][y])
                    {

                    case -1:
                        bitmap = bitmap_resources.bitmap_white;
                        break;

                    case 1:
                        bitmap = bitmap_resources.bitmap_red;
                        break;

                    case 2:
                        bitmap = bitmap_resources.bitmap_green;
                        break;

                    case 3:
                        bitmap = bitmap_resources.bitmap_blue;
                        break;

                    case 4:
                        bitmap = bitmap_resources.bitmap_yellow;
                        break;

                    case 5:
                        bitmap = bitmap_resources.bitmap_orange;
                        break;
                    }
                    if (playArea[x][y] != 0)
                        drawBitmap(bitmap, PADDING + x * 16, y * 16);
                }

                TTF_SizeText(font, VERSION_TEXT, &text_w, &text_h);

                drawText(screen, font, VERSION_TEXT, (screen_width - text_w - PADDING), 10);

                TTF_SizeText(font, VERSION_TEXT, &text_w, &text_h);

                scoreText = "SCORE: " + std::to_string(score);
                drawText(screen, font, scoreText.c_str(), (screen_width - text_w - PADDING), 32);
            }

     

            if (SDL_GetTicks() >= ticks)
            {

                if(newBlock[2].placed)
                {
                    checkForLines();  

                    SDL_TimerID timerID = SDL_AddTimer(300, removeBlocksCallback, NULL);
                }
                if (moveLeft)
                {
                    blockMoveLeft();
                }
                if (moveRight)
                {
                    blockMoveRight();
                }

                if (fullBoardCount())
                {
                    running = 0;
                }

                if(newBlock[2].placed)
                {


                    createBlock(getFreePosition());
                    


                }

                ticks = SDL_GetTicks() + speedms;

                moveBlocks();
            }

            SDL_Flip(screen);
        }
    }
    quit(0);
}
