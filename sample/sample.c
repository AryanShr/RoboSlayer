#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#include "utilis.h"

void draw_random_points(int points, bool randomizedcolor, SDL_Renderer *renderer)
{
    for (int i = 0;i<points;++i)
    {
        if (randomizedcolor)
        {
            SDL_SetRenderDrawColor(renderer,randomcolor(),randomcolor(),randomcolor(),255);
        }
        SDL_RenderDrawPoint(renderer,randomNumber(0,640),randomNumber(0,480));
    }
}

void draw_random_lines(int lines, bool randomizedcolor, SDL_Renderer *renderer)
{
    for (int i = 0;i<lines;++i)
    {
        if (randomizedcolor)
        {
            SDL_SetRenderDrawColor(renderer,randomcolor(),randomcolor(),randomcolor(),255);
        }
        SDL_RenderDrawLine(renderer,randomNumber(0,640),randomNumber(0,480),randomNumber(0,640),randomNumber(0,480));
    }
}
int main(int argc, char **argv)
{
    /* Separating Joystick and Video initialization. */
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("A Window",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1400, 442,SDL_WINDOW_OPENGL);

    

    // SDL_SetRenderDrawColor(renderer,0,255,0,255);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    // Load an image file
    SDL_Surface *image = IMG_Load("img/bg.png");
    SDL_Texture *image_texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_FreeSurface(image);
    // define where on the screen we want to the texture
    SDL_Rect texture_destination;
    texture_destination.x = 0;
    texture_destination.y = 0;
    texture_destination.w = 1400;
    texture_destination.h = 442;

    bool running = true;

    SDL_Event event;

    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }   

        SDL_SetRenderDrawColor(renderer,0,0,0,255);

        //Clear Screen
        SDL_RenderClear(renderer);
         //Draw a point
        // SDL_SetRenderDrawColor(renderer,255,255,0,255);
        // draw_random_points(500,true,renderer);
        // draw_random_lines(500,true,renderer);
        
        SDL_RenderCopy(renderer,image_texture, NULL,&texture_destination);

        //Draw rectangle

        // SDL_Rect rect = {50,50,200,200};
        // SDL_RenderDrawRect(renderer,&rect);
        SDL_RenderPresent(renderer);
    }


    /* Release Resources */
    SDL_DestroyTexture(image_texture);
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
/*

15/02/2022
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1400
#define SCREEN_HEIGHT 442

#define ticks  SDL_GetTicks()
#define seconds  ticks/100
#define FPS 15
#define screenticks 1000/FPS
#define framelimit  seconds%6

#define ROW0 0
#define ROW1 185
#define ROW2 350
#define ROW3 500

#define PLAYER_SPEED 4
#define PLAYER_BULLET_SPEED 16

#define MAX_KEYBOARD_KEYS 350
typedef struct{
	void (*logic)(void);
	void (*draw)(void);
}Delegate;
typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
	Delegate delegate;
	int keyboard[MAX_KEYBOARD_KEYS];
    // int up;
    // int down;
    // int left;
    // int right;
	// int fire;
}App;

static App app;

typedef struct Entity{
	float x;
	float y;
	float dx;
	float dy;
	int w;
	int h;
	int frame;
	int health;
	int facing;
	int reload;
	// struct Entity *next;
	SDL_Texture *texture;
	struct Entity *next;
} Entity;

typedef struct {
	Entity fighterHead, *fighterTail;
	Entity bulletHead, *bulletTail;
}Stage;
static Stage stage;
static Entity *hero;
static Entity *bullet;
void initSDL()
{
	SDL_Init(SDL_INIT_VIDEO);
    int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	app.window = SDL_CreateWindow("GAME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

	if (!app.window)
	{
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

	if (!app.renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}
}
void doKeyDown(SDL_KeyboardEvent *event)
{
	if (event->repeat == 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS)
	{
		app.keyboard[event->keysym.scancode] = 1;
	}
}
void doKeyUp(SDL_KeyboardEvent *event)
{
	if (event->repeat == 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS)
	{
		app.keyboard[event->keysym.scancode] = 0;
	}
}
void doInput()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
				break;
            case SDL_KEYDOWN:
			{
				doKeyDown(&event.key);
			}
				break;

			case SDL_KEYUP:
				doKeyUp(&event.key);
				break;
			default:
				break;
		}
	}
}
SDL_Texture *generate_img(const char *IMAGE_PATH)
{
    // Load an image file
    SDL_Surface *image = IMG_Load(IMAGE_PATH);
    SDL_Texture *image_texture = SDL_CreateTextureFromSurface(app.renderer, image);
    SDL_FreeSurface(image);
    return image_texture;
}
static void doBullet()
{
	Entity *b,*prev;
	prev = &stage.bulletHead;
	for (b = stage.bulletHead.next ; b != NULL ; b = b->next)
	{
		b->x += b->dx;
		b->y += b->dy;

		if (b->x > SCREEN_WIDTH)
		{
			if (b == stage.bulletTail)
			{
				stage.bulletTail = prev;
			}

			prev->next = b->next;
			free(b);
			b = prev;
		}

		prev = b;
	}
}
void doPlayer()
{
	hero->dx = hero->dy = 0;
	if (hero->reload >0)
	{
		hero->reload--;
	}
	if (app.keyboard[SDL_SCANCODE_UP])
	{
		hero->dy = -PLAYER_SPEED;
	}
	if (app.keyboard[SDL_SCANCODE_DOWN])
	{
		hero->dy = PLAYER_SPEED;
	}
	if (app.keyboard[SDL_SCANCODE_LEFT])
	{
		hero->dx = -PLAYER_SPEED;
	}
	if (app.keyboard[SDL_SCANCODE_RIGHT])
	{
		hero->dy = PLAYER_SPEED;
	}
	if (app.keyboard[SDL_SCANCODE_LCTRL] && hero->reload == 0)
	{
		firebullet();
	}
	hero->x +=hero->dx;
	hero->y +=hero->dy;
}
static void logic()
{
	doPlayer();
	doBullet();
}
static void draw()
{
	drawPlayer();
	drawBullets();
}
void initStage()
{
	app.delegate.logic = logic;
	app.delegate.draw = draw;

	memset(&stage, 0, sizeof(Stage));
	stage.fighterTail = &stage.fighterHead;
	stage.bulletTail = &stage.bulletHead;

	initPlayer();

	// bullet.Texture = generate_img("img/bullet.png");
}
void drawPlayer()
{
	blit(hero->texture,hero->x,hero->y,ROW1);
}
void drawBullets()
{
	Entity *b;
	for (b = stage.bulletHead.next;b!=NULL;b=b->next)
	{
		blit(b->texture,b->x,b->y,0);
	}
}
void initPlayer()
{
	hero = malloc(sizeof(Entity));
	memset(hero,0,sizeof(Entity));
	stage.fighterTail->next = hero;
	stage.fighterTail = hero;
	hero->x = 10;
    hero->y = 278;
	hero->texture = generate_img("img/Characterspritesbw.png");
	SDL_QueryTexture(hero->texture, NULL,NULL,&hero->w,&hero->h);
}


void firebullet()
{
	Entity *bullet;
	bullet = malloc(sizeof(Entity));
	memset(bullet,0,sizeof(Entity));
	stage.fighterTail->next = bullet;
	stage.bulletTail = bullet;

	bullet->x = hero->x;
	bullet->y = hero->y;
	bullet->dx = PLAYER_BULLET_SPEED;
	bullet->health = 1;
	bullet->texture = generate_img("img/bullet.png");
	SDL_QueryTexture(bullet->texture,NULL,NULL,&bullet->w,&bullet->h);
	bullet->y += (hero->h/2)-(bullet->h/2);
	hero->reload = 8;
}

void blit (SDL_Texture * image_texture, int x, int y,int row)
{
    SDL_Rect texture_source ={hero->frame * 100,row,100,135};
    SDL_Rect texture_destination = {x,y,100,135};
    // SDL_QueryTexture(image_texture, NULL,NULL, &texture_destination.w, &texture_destination.h);
    SDL_RenderCopy(app.renderer,image_texture, &texture_source,&texture_destination);
}
void jump (SDL_Texture * image_texture, int x, int y,int jumpframe)
{
    SDL_Rect texture_source [] ={{0,ROW3,100,135},
								{100,ROW3,100,135},
								{120,ROW3,100,135},
								{130,ROW3,100,135},
								{120,ROW3,100,135},
								{100,ROW3,100,135}
								};
    SDL_Rect texture_destination = {x,y,100,135};
    // SDL_QueryTexture(image_texture, NULL,NULL, &texture_destination.w, &texture_destination.h);
    // int i =0;
	hero->x+=15;
	SDL_RenderCopyEx(app.renderer,image_texture, &texture_source[jumpframe],&texture_destination,0,NULL,0);
	// i++;
		
}

void prepareScene()
{
	// SDL_SetRenderDrawColor(app.renderer, 96, 128, 255, 255);
	SDL_Texture * background = generate_img("img/bg1.png");
	SDL_Rect texture_destination;
    texture_destination.x = 0;
    texture_destination.y = 0;
    texture_destination.w = 1400;
    texture_destination.h = 442;
    SDL_RenderCopy(app.renderer,background, NULL,&texture_destination);
	// SDL_RenderClear(app.renderer);
}
void presentScene()
{
	SDL_RenderPresent(app.renderer);
}

// void shoot()
// {

// }
void destroy()
{
	SDL_DestroyTexture(hero->texture);
	SDL_DestroyTexture(bullet->texture);
    IMG_Quit();
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
}
static void capFrameRate(long *then, float *remainder)
{
	long wait, frameTime;

	wait = 16 + *remainder;

	*remainder -= (int)*remainder;

	frameTime = SDL_GetTicks() - *then;

	wait -= frameTime;

	if (wait < 1)
	{
		wait = 1;
	}

	SDL_Delay(wait);

	*remainder += 0.667;

	*then = SDL_GetTicks();
}
// int main(int argc, char **argv)
// {

//     memset(&app,0,sizeof(app));
//     memset(&hero, 0, sizeof(Entity));
//     memset(&bullet, 0, sizeof(Entity));
//     initSDL();
//     hero->texture = generate_img("img/Characterspritesbw.png");
// 	SDL_Texture *bg = generate_img("img/bg1.png");
// 	bullet.texture = generate_img("img/bullet.png");
//     hero->x = 10;
//     hero->y = 278;
// 	hero->frame =0;
// 	// int runframe[] = {0,1,2,3,4,5,2,3,4,5};
// 	int row = ROW0;
//     // atexit(cleanup);
// 	int running =1;
//     while(running)
//     {
//     	prepareScene(bg);
// 	    doInput();

//         if (app.up)
// 		{
// 			int jumpframe = 0;
// 			row = ROW3;
// 			hero->y = 200;
// 			while(jumpframe<6)
// 			{
// 			SDL_Delay(screenticks);
// 			jump(hero->texture,hero->x,hero->y,jumpframe);
// 			jumpframe++;
// 			printf("jump\n");
// 			}// hero->frame%=6; 
// 		}

// 		// if (app.down)
// 		// {
// 		// 	hero->y += 4;
// 		// }
// 		// hero->y=278;
// 		if (app.left)
// 		{
// 			row=ROW1;
// 			hero->x -= 20;
// 			hero->frame++;
// 			hero->frame%=4+1;
// 		}

// 		if (app.right)
// 		{
// 			row=ROW1;
// 			hero->x += 20;
// 			SDL_Delay(screenticks);
// 			hero->frame++;
// 			if (hero->frame == 6)
// 			{
// 				hero->frame = 2;
// 			}
// 			printf("right\n");
// 			// hero->frame=hero->frame%6;
// 		}

// 		if (app.fire && bullet.health ==0)
// 		{
// 			bullet.x = hero->x;
// 			bullet.y = hero->y;
// 			bullet.dx = 16;
// 			bullet.dy = 0;
// 			bullet.health =1;
// 		}
// 		bullet.x +=bullet.dx;
// 		bullet.y +=bullet.dy;

// 		if (bullet.x>SCREEN_WIDTH)
// 		{
// 			bullet.health =0;
// 		}
//         if (bullet.health>0)
// 		{
// 			blit(bullet.texture,bullet.x,bullet.y,0);
// 		}
        
// 		blit(hero->texture,hero->x,hero->y,row);
// 		presentScene();
// 		SDL_RenderClear(app.renderer);
// 		SDL_Delay(16);
//     }
// 	SDL_DestroyTexture(hero->texture);
// 	SDL_DestroyTexture(bullet.texture);
//     IMG_Quit();
//     SDL_DestroyRenderer(app.renderer);
//     SDL_DestroyWindow(app.window);
//     SDL_Quit();


//     return 0;
// }

int main(int argc, char *argv[])
{
	long then;
	float remainder;

	memset(&app, 0, sizeof(App));

	initSDL();

	// atexit(cleanup);

	initStage();

	then = SDL_GetTicks();

	remainder = 0;

	while (1)
	{
		prepareScene();

		doInput();

		app.delegate.logic();

		app.delegate.draw();

		presentScene();

		capFrameRate(&then, &remainder);
	}

	return 0;
}
*/