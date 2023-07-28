#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>

#define MAX_ENEMIES 100
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define HERO_SIZE 50
#define ENEMY_SIZE 10

// Hero class code
typedef struct
{
    int x, y, w, h;
    int life;
    char *name;
} Hero;

Hero *createHero()
{
    Hero *hero = calloc(1, sizeof(hero));
    hero->x = 350;
    hero->y = 250;
    hero->w = HERO_SIZE;
    hero->h = HERO_SIZE;
    hero->life = 3;
    return hero;
}

// Enemy class code
typedef struct
{
    int activated;
    int x, y, w, h;
} Enemy;

Enemy *createEnemies(int nEnemies)
{
    // Activatied field initialised to 0 using calloc
    Enemy *enemies = calloc(nEnemies, sizeof(Enemy));
    return enemies;
}

typedef struct
{
    // Player
    Hero *hero;

    // Enemies
    Enemy *enemy;

    // Images
    SDL_Texture *sprite;

} GameState;

// Future proofing function
void activateEnemies(int total, GameState *game, int round)
{
    Enemy *enemies = game->enemy;
    int i = 0;
    int activated = 0;
    while (activated < total && i < MAX_ENEMIES)
    {
        if (!enemies[i].activated)
        {
            activated++;
            enemies[i].activated = 1;
            enemies[i].w = round * ENEMY_SIZE;
            enemies[i].h = round * ENEMY_SIZE;
            enum spawnPlace
            {
                TOP,
                BOTTOM,
                LEFT,
                RIGHT
            };
            switch (rand() % 4)
            {
            case TOP:
                enemies[i].x = rand() % WINDOW_WIDTH;
                enemies[i].y = 0;
                break;
            case BOTTOM:
                enemies[i].x = rand() % WINDOW_WIDTH;
                enemies[i].y = WINDOW_HEIGHT;
                break;
            case LEFT:
                enemies[i].x = 0;
                enemies[i].y = rand() % WINDOW_HEIGHT;
                break;
            case RIGHT:
                enemies[i].x = WINDOW_WIDTH;
                enemies[i].y = rand() % WINDOW_HEIGHT;
                break;
            default:
                printf("Invalid enemy spawn case");
                return;
            }
        }
        i++;
    }
}

void spawnEnemy(GameState *game, int round)
{
    if (rand() % (120 / round) == 0)
    {
        activateEnemies(1, game, round);
    }
}


int checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    int l = x1 > x2 + w2; // Left is right of right
    int r = x1 + w1 < x2; // Right is left of left
    int t = y1 > y2 + h2; // Top is below bottom
    int b = y1 + h2 < y2; // Bottom is below top
    if (l || r || t || b)
    {
        return 0;
    }
    return 1;
}

int pointCollision(int x1, int y1, int x2, int y2, int w2, int h2)
{
    int xCheck = x1 < x2 + w2 && x1 > x2;
    int yCheck = y1 < y2 + h2 && y1 > y2;
    return xCheck && yCheck;
}

void moveEnemies(GameState *game, int speed)
{
    Hero *hero = game->hero;
    Enemy *enemies = game->enemy;
    double xDiff, yDiff;
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].activated)
        {
            xDiff = hero->x - enemies[i].x;
            yDiff = hero->y - enemies[i].y;
            double magnitude = sqrt(xDiff * xDiff + yDiff * yDiff);
            if (magnitude != 0)
            {
                double dx = xDiff / magnitude; // Direction maintained because xDiff used in calc
                double dy = yDiff / magnitude;
                enemies[i].x += speed * dx;
                enemies[i].y += speed * dy;
            }
            if (checkCollision(hero->x, hero->y, hero->w, hero->y, enemies[i].x, enemies[i].y, enemies[i].w, enemies[i].h))
            {
                enemies[i].activated = 0;
            }
        }
    }
}

int processEvents(SDL_Window *window, GameState *game)
{
    Hero *hero = game->hero;

    int done = 0;
    SDL_Event event;

    // Check for event
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT_CLOSE:
        {
            if (window)
            {
                SDL_DestroyWindow(window);
                window = NULL;
                done = 1;
            }
        }
        break;
        case SDL_KEYDOWN:
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                done = 1;
                break;
            }
            break;
        }
        case SDL_QUIT:
            done = 1;
            break;

        default:
            break;
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    double heroSpeed = 20;
    if (state[SDL_SCANCODE_LEFT])
    {
        hero->x -= heroSpeed;
    }
    if (state[SDL_SCANCODE_RIGHT])
    {
        hero->x += heroSpeed;
    }
    if (state[SDL_SCANCODE_UP])
    {
        hero->y -= heroSpeed;
    }
    if (state[SDL_SCANCODE_DOWN])
    {
        hero->y += heroSpeed;
    }

    return done;
}

void doRender(SDL_Renderer *renderer, GameState *game)
{
    Hero *hero = game->hero;
    Enemy *enemies = game->enemy;
    // Render display
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    // clear screen to blue
    SDL_RenderClear(renderer);

    // set drawing colour to red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect rect = {hero->x, hero->y, hero->w, hero->h};
    SDL_RenderFillRect(renderer, &rect);

    // draw the sprite
    SDL_Rect spriteRect = {50, 50, 256, 256};
    SDL_RenderCopy(renderer, game->sprite, NULL, &spriteRect);

    // set drawing colour to green
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].activated)
        {
            SDL_Rect rect = {enemies[i].x, enemies[i].y, enemies[i].w, enemies[i].h};
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // done drawing, now presenting
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    { // Initialize SDL2
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Game Window",           // window title
                                          SDL_WINDOWPOS_UNDEFINED, // window x position
                                          SDL_WINDOWPOS_UNDEFINED, // window y position
                                          WINDOW_WIDTH,            // width, in pixels
                                          WINDOW_HEIGHT,           // height, in pixels
                                          SDL_WINDOW_SHOWN         // flags
    );
    if (!window)
    {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Hero *hero = createHero();
    Enemy *enemies = createEnemies(MAX_ENEMIES);
    SDL_Surface *spriteSurface = NULL;
    GameState game;
    game.hero = hero;
    game.enemy = enemies;

    spriteSurface = IMG_Load("heart_sprite.png");
    if (spriteSurface == NULL) {
        printf("Cannot find sprite.\n\n");
        SDL_Quit();
        return 1;
    }

    game.sprite = SDL_CreateTextureFromSurface(renderer, spriteSurface);
    SDL_FreeSurface(spriteSurface);

    // Event loop
    int done = 0;
    int round = 1;
    int enemySpeed = 10;
    while (!done)
    {
        // check for events
        done = processEvents(window, &game);

        moveEnemies(&game, enemySpeed);

        spawnEnemy(&game, round);

        // render display
        doRender(renderer, &game);

        // limit fps
        SDL_Delay(20);
    }

    // Free all memory structures
    free(hero);
    free(enemies);
    SDL_DestroyTexture(game.sprite);

    // close and destroy window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
