#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <stdlib.h>

#define MAX_ENEMIES 100
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Hero class code
typedef struct
{
    int x, y;
    int life;
    char *name;
} Hero;

Hero *createHero()
{
    Hero *hero = calloc(1, sizeof(hero));
    hero->x = 350;
    hero->y = 250;
    hero->life = 3;
    return hero;
}

// Enemy class code
typedef struct
{
    int activated;
    int x, y;
} Enemy;

Enemy *createEnemies(int nEnemies)
{
    // Activatied field initialised to 0 using calloc
    Enemy *enemies = calloc(nEnemies, sizeof(Enemy));
    return enemies;
}

// Future proofing function
void activateEnemies(int total, Enemy *enemies)
{
    int i = 0;
    int activated = 0;
    while (activated < total && i < MAX_ENEMIES)
    {
        if (!enemies[i].activated)
        {
            activated++;
            enemies[i].activated = 1;
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

void spawnEnemy(Enemy *enemies, int round)
{
    if (rand() % (120 / round) == 0)
    {
        activateEnemies(1, enemies);
    }
}

void moveEnemies(Hero *hero, Enemy *enemies, int speed)
{
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
                double dx = xDiff / magnitude;
                double dy = yDiff / magnitude;
                enemies[i].x += speed * dx;
                enemies[i].y += speed * dy;
            }
        }
    }
}

int processEvents(SDL_Window *window, Hero *hero)
{

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

void doRender(SDL_Renderer *renderer, Hero *hero, Enemy *enemies)
{
    // Render display
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    // clear screen to blue
    SDL_RenderClear(renderer);

    // set drawing colour to red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect rect = {hero->x, hero->y, 100, 100};
    SDL_RenderFillRect(renderer, &rect);

    // set drawing colour to green
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].activated)
        {
            SDL_Rect rect = {enemies[i].x, enemies[i].y, 20, 20};
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

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Hero *hero = createHero();
    Enemy *enemies = createEnemies(MAX_ENEMIES);

    // Event loop
    int done = 0;
    int round = 1;
    int enemySpeed = 10;
    while (!done)
    {
        // check for events
        done = processEvents(window, hero);

        moveEnemies(hero, enemies, enemySpeed);

        spawnEnemy(enemies, round);

        // render display
        doRender(renderer, hero, enemies);

        // limit fps
        SDL_Delay(20);
    }

    // Free all memory structures
    free(hero);
    free(enemies);

    // close and destroy window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
