#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
// #include "declarations.h"

#define ROW0 0
#define ROW1 185
#define ROW2 350
#define ROW3 500

#define SCREEN_WIDTH 1400
#define SCREEN_HEIGHT 442
#define MAX_KEYBOARD_KEYS 350
#define MAX_MOUSE_KEYS 350
#define BULLET_SPEED 30
#define GRAVITY 1.2f
#define MAX_BULLETS 100
#define MAX_ENEMY 2

int globaltime = 0;
int kills = 0;
int status = 0;
/*For mouse reading*/
typedef struct Mouse
{
	int button[MAX_MOUSE_KEYS];
	float angle;
} mouse;
typedef struct
{
	SDL_Renderer *renderer;
	SDL_Window *window;
	int keyboard[MAX_KEYBOARD_KEYS];
	mouse mouse;
	TTF_Font *font;
	SDL_Texture *label;
	SDL_Texture *labelkill;
	SDL_Texture *heart;
	int labelw, labelh;
	int labelkillw, labelkillh;
	int musicchannel;
	Mix_Music *bgmusic;
} App;
static App app;
/*All Entities declaration*/
typedef struct Entity
{
	float x;
	float y;
	float dx;
	float dy;
	short lives;
	int onground;
	int isdead;
	SDL_Texture *charachter;
	int facingleft;
	int frames;
	int slowingdown;
	int row;
	Mix_Chunk *shoot;
	Mix_Chunk *shootmetal;
	Mix_Chunk *hurt;
} Entity;
Entity hero;
Entity *enemy;
Entity *enemy1;

/*For bullets*/
typedef struct Bullet
{
	float x, y, dx, dy;
} Bullet;
Bullet *bullets[MAX_BULLETS] = {NULL};

typedef struct alive
{
	int entityalive;
} alive;
alive enemyalive;
/*For Adding Bullets*/
SDL_Texture *bullettexture;
SDL_Texture *enemybullettexture;
SDL_Texture *enemybullettexture1;
void addbullet(float x, float y, float dx, float dy)
{
	int found = -1;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (bullets[i] == NULL)
		{
			found = i;
			break;
		}
	}
	if (found >= 0)
	{
		int i = found;
		bullets[i] = malloc(sizeof(Bullet));
		bullets[i]->x = x;
		bullets[i]->y = y;
		bullets[i]->dx = dx;
		bullets[i]->dy = dy;
	}
}
/*Free up space by removing used bullets*/
void removebullets(int i)
{
	if (bullets[i])
	{
		free(bullets[i]);
		bullets[i] = NULL;
	}
}
Bullet *enemyBullets[MAX_BULLETS] = {NULL};
Bullet *enemyBullets1[MAX_BULLETS] = {NULL};
void addenemybullet(float x, float y, float dx, float dy)
{
	int found = -1;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (enemyBullets[i] == NULL)
		{
			found = i;
			break;
		}
	}
	if (found >= 0)
	{
		int i = found;
		enemyBullets[i] = malloc(sizeof(Bullet));
		enemyBullets[i]->x = x;
		enemyBullets[i]->y = y;
		enemyBullets[i]->dx = dx;
		enemyBullets[i]->dy = dy;
	}
}
void addenemybulletleft(float x, float y, float dx, float dy)
{
	int found = -1;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (enemyBullets1[i] == NULL)
		{
			found = i;
			break;
		}
	}
	if (found >= 0)
	{
		int i = found;
		enemyBullets1[i] = malloc(sizeof(Bullet));
		enemyBullets1[i]->x = x;
		enemyBullets1[i]->y = y;
		enemyBullets1[i]->dx = dx;
		enemyBullets1[i]->dy = dy;
	}
}
void removeenemybullets(int i)
{
	if (enemyBullets[i])
	{
		free(enemyBullets[i]);
		enemyBullets[i] = NULL;
	}
}
void removeenemybulletsleft(int i)
{
	if (enemyBullets1[i])
	{
		free(enemyBullets1[i]);
		enemyBullets1[i] = NULL;
	}
}
/*Initialising SDL Windows*/
void initSDL()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

	windowFlags = 0;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
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

	// SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

	if (!app.renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}

	TTF_Init();
	if (TTF_Init() == -1)
	{
		printf("Init not loaded\n %s\n", TTF_GetError());
	}

	// load support for the OGG and MOD sample/music formats
	int flags = MIX_INIT_OGG | MIX_INIT_MOD | MIX_INIT_MP3;
	Mix_Init(flags);
	if (Mix_Init(flags) == 0)
	{
		printf("Mix_Init: Failed to init required ogg and mod support!\n");
		printf("Mix_Init: %s\n", Mix_GetError());
		// handle error
	}

	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("Error in opening audio\n");
	}
}

SDL_Texture *generate_img(const char *IMAGE_PATH)
{
	// Load an image file
	SDL_Surface *image = IMG_Load(IMAGE_PATH);
	if (image == NULL)
	{
		printf("error loading surface %s\n", IMAGE_PATH);
		exit(1);
	}
	/*Copying texture form surface then free up the Surface*/
	SDL_Texture *image_texture = SDL_CreateTextureFromSurface(app.renderer, image);
	if (image_texture == NULL)
	{
		printf("error loading texture %s\n", IMAGE_PATH);
		exit(1);
	}
	SDL_FreeSurface(image);
	return image_texture;
}

// To generate background of the game
void generatebg(SDL_Texture *image)
{
	if (image == NULL)
	{
		printf("error loading background\n");
		exit(1);
	}
	SDL_Rect texture_destination;
	texture_destination.x = 0;
	texture_destination.y = 0;
	texture_destination.w = SCREEN_WIDTH;
	texture_destination.h = SCREEN_HEIGHT;
	SDL_RenderCopy(app.renderer, image, NULL, &texture_destination);

	app.font = TTF_OpenFont("fonts/miss you.ttf", 48);
	if (!app.font)
	{
		printf("Cannot find font file\n\n");
		SDL_Quit();
		exit(1);
	}
	char life_status[128] = "";
	char kill_status[128] = "";
	sprintf(life_status, "x %d", hero.lives);
	sprintf(kill_status, "KILLS =  %d", kills);
	SDL_Color black = {0, 0, 0, 255};
	SDL_Color red = {255, 0, 0, 255};
	SDL_Surface *tmp = TTF_RenderText_Blended(app.font, life_status, black);
	app.labelh = tmp->h;
	app.labelw = tmp->w;
	app.label = SDL_CreateTextureFromSurface(app.renderer, tmp);
	SDL_FreeSurface(tmp);
	tmp = TTF_RenderText_Blended(app.font, kill_status, red);
	app.labelkillh = tmp->h;
	app.labelkillw = tmp->w;
	app.labelkill = SDL_CreateTextureFromSurface(app.renderer, tmp);
	SDL_FreeSurface(tmp);
	SDL_Rect rect = {68, 10, 48, 48};
	SDL_Rect rectkill = {1200, 10, 120, 60};
	SDL_RenderCopy(app.renderer, app.label, NULL, &rect);
	SDL_RenderCopy(app.renderer, app.labelkill, NULL, &rectkill);
	app.heart = generate_img("img/heart.png");
	SDL_Rect heart_loc = {10, 10, 48, 48};
	SDL_RenderCopy(app.renderer, app.heart, NULL, &heart_loc);
}

// to destroy the SDL window
void destroy()
{
	SDL_DestroyRenderer(app.renderer);
	SDL_DestroyWindow(app.window);
	TTF_CloseFont(app.font);
	IMG_Quit();
	TTF_Quit();
	Mix_FreeChunk(hero.shoot);
	Mix_FreeChunk(hero.shootmetal);
	Mix_FreeMusic(app.bgmusic);
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();
	printf("SDL destroyed\n");
	printf("kills = %d\n", kills);
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
void doMouseButtonUp(SDL_MouseButtonEvent *event)
{
	app.mouse.button[event->button] = 0;
}

void doMouseButtonDown(SDL_MouseButtonEvent *event)
{
	app.mouse.button[event->button] = 1;
}
int input = 0;
void doInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		// printf("%s\n",event.type);
		case SDL_QUIT:
		{
			destroy();
			exit(0);
			break;
		}
		case SDL_KEYDOWN:
		{
			doKeyDown(&event.key);
			break;
			input = 1;
		}

		case SDL_KEYUP:
		{
			doKeyUp(&event.key);
			input = 0;
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			doMouseButtonDown(&event.button);
			input = 0;
			break;
		}

		case SDL_MOUSEBUTTONUP:
		{
			doMouseButtonUp(&event.button);
			input = 1;
			break;
		}
		default:
		{
			input = 0;
			break;
		}
		}
	}
}
typedef struct cross_hair
{
	SDL_Texture *Texture;
	float x, y, dx, dy;
} cross_hair;
cross_hair crosshair;
void initPlayer(SDL_Texture *player, int x, int y)
{
	if (player == NULL)
	{
		printf("error initiating player..\n");
		exit(1);
	}
	if (hero.lives > 0)
	{
		// printf("%d\n",hero.lives);
		SDL_Rect player_source = {hero.frames * 100, hero.row, 100, 135};
		SDL_Rect player_destination = {x, y, 100, 135};
		SDL_RenderCopyEx(app.renderer, player, &player_source, &player_destination, 0, NULL, hero.facingleft);
		for (int i = 0; i < MAX_BULLETS; i++)
			if (bullets[i])
			{
				SDL_Rect rect = {bullets[i]->x, bullets[i]->y, 18, 18};
				SDL_RenderCopy(app.renderer, bullettexture, NULL, &rect);
			}
		for (int i = 0; i < MAX_BULLETS; i++)
			if (enemyBullets[i])
			{
				SDL_Rect rect = {enemyBullets[i]->x, enemyBullets[i]->y, 30, 30};
				SDL_RenderCopyEx(app.renderer, enemybullettexture, NULL, &rect, 0, NULL, 1);
			}
		for (int i = 0; i < MAX_BULLETS; i++)
			if (enemyBullets1[i])
			{
				SDL_Rect rect = {enemyBullets1[i]->x, enemyBullets1[i]->y, 30, 30};
				SDL_RenderCopyEx(app.renderer, enemybullettexture1, NULL, &rect, 0, NULL, 0);
			}
	}
	else
	{
		if (hero.isdead == 0 && hero.onground == 1)
		{
			hero.row = ROW3;
			hero.frames = 7;
			hero.isdead = 1;
			hero.y += hero.dy;
		}
		SDL_Delay(30);
		SDL_Rect player_source = {hero.frames * 100, hero.row, 100, 135};
		SDL_Rect player_destination = {x, y, 100, 135};
		SDL_RenderCopyEx(app.renderer, player, &player_source, &player_destination, 0, NULL, hero.facingleft);
		hero.frames++;
		if (hero.frames == 20)
		{
			printf("Hero Died \n");
			status = 2;
			// destroy();
			// exit(0);
		}
		// printf("I am in 306\n");
		// if ()
	}
}

int local_time = 0;
int firedbullets = 0;
void spawnenemyright()
{
	enemybullettexture = generate_img("img/bullet.png");
	enemy->charachter = generate_img("img/RoboEnemy.png");
	if (local_time % 10 == 0 && enemyBullets[firedbullets] == NULL)
	{
		// printf("I am in add eenemy bullet\n");
		addenemybullet(enemy->x, enemy->y + 70, enemy->dx * 5, 0);
	}
	if (enemyBullets[firedbullets])
	{

		enemyBullets[firedbullets]->x -= enemyBullets[firedbullets]->dx;
		if (enemyBullets[firedbullets]->x < 0 || enemy->isdead == 1)
		{
			removeenemybullets(firedbullets);
			// firedbullets ++;
		}
		else if (enemyBullets[firedbullets]->x < hero.x + 50 && enemyBullets[firedbullets]->x > hero.x && enemyBullets[firedbullets]->y > hero.y && enemyBullets[firedbullets]->y < hero.y + 135)
		{
			removeenemybullets(firedbullets);
			hero.lives--;
			Mix_PlayChannel(-1, hero.hurt, 0);
		}
		// printf("%d\n",firedbullets);
		// }
	}
	if (enemy->charachter == NULL)
	{
		printf("Check your Damn Path again\n");
		exit(1);
	}
	if (local_time % 3 == 0 && enemy->isdead == 0)
	{
		enemy->frames++;
		enemy->frames %= 9;
		enemy->x -= enemy->dx;
	}
	if (hero.x+70>=enemy->x&&hero.x<=enemy->x+50&&hero.y+120>=enemy->y)
	{
		enemy->dx = 0;
		enemy->frames = 7;
		hero.lives = 0;
	}
	else
	{
		enemy->dx = 5;
	}
	if (enemy->isdead == 1)
	{
		enemy->row = 0;
		enemy->dx = 0;
		if (local_time % 5 == 0 && enemy->frames < 6)
		{
			enemy->frames++;
		}
		// enemy->frames%=5;
	}
	SDL_Rect enemy_source = {enemy->frames * 95, enemy->row, 90, 100};
	SDL_Rect enemy_distination = {enemy->x, enemy->y, 120, 135};
	SDL_RenderCopyEx(app.renderer, enemy->charachter, &enemy_source, &enemy_distination, 0, NULL, 1);
	if (enemy->isdead == 1 && enemy->frames == 5 && enemy->row == 0)
	{
		kills++;
		enemy->isdead = 0;
		enemy->frames = 0;
		// enemyalive.entityalive = 0;
		enemy->lives = 3;
		enemy->row = 100;
		enemy->x = 1450;

		// free(enemy);
	}
	local_time = (local_time + 1) % 100;
}
int localtime1 = 0;
void spawnenemyleft()
{
	enemybullettexture1 = generate_img("img/bullet.png");
	enemy1->charachter = generate_img("img/RoboEnemy.png");
	if (localtime1 % 10 == 0 && enemyBullets1[firedbullets] == NULL)
	{
		// printf("I am in add eenemy bullet\n");
		addenemybulletleft(enemy1->x, enemy1->y + 70, enemy1->dx * 5, 0);
	}
	if (enemyBullets1[firedbullets])
	{

		enemyBullets1[firedbullets]->x += enemyBullets1[firedbullets]->dx;
		if (enemyBullets1[firedbullets]->x > 1300 || enemy1->isdead == 1)
		{
			removeenemybulletsleft(firedbullets);
			// firedbullets ++;
		}
		else if (enemyBullets1[firedbullets]->x < hero.x + 50 && enemyBullets1[firedbullets]->x > hero.x && enemyBullets1[firedbullets]->y > hero.y && enemyBullets1[firedbullets]->y < hero.y + 135)
		{
			removeenemybulletsleft(firedbullets);
			hero.lives--;
			Mix_PlayChannel(-1, hero.hurt, 0);
		}
		// printf("%d\n",firedbullets);
		// }
	}
	if (enemy1->charachter == NULL)
	{
		printf("Check your Damn Path again\n");
		exit(1);
	}
	if (localtime1 % 3 == 0 && enemy1->isdead == 0)
	{
		enemy1->frames++;
		enemy1->frames %= 9;
		enemy1->x += enemy1->dx;
	}
	if (enemy1->x >= hero.x)
	{
		enemy1->dx = 0;
		enemy1->frames = 7;
		hero.lives = 0;
	}
	else
	{
		enemy1->dx = 5;
	}
	if (enemy1->isdead == 1)
	{
		enemy1->row = 0;
		enemy1->dx = 0;
		if (localtime1 % 5 == 0 && enemy1->frames < 6)
		{
			enemy1->frames++;
		}
		// enemy->frames%=5;
	}
	SDL_Rect enemy_source = {enemy1->frames * 95, enemy1->row, 90, 100};
	SDL_Rect enemy_distination = {enemy1->x, enemy1->y, 120, 135};
	SDL_RenderCopyEx(app.renderer, enemy1->charachter, &enemy_source, &enemy_distination, 0, NULL, 0);
	if (enemy1->isdead == 1 && enemy1->frames == 5 && enemy1->row == 0)
	{
		kills++;
		enemy1->isdead = 0;
		enemy1->frames = 0;
		// enemyalive.entityalive = 0;
		enemy1->lives = 3;
		enemy1->row = 100;
		enemy1->x = -30;

		// free(enemy);
	}
	localtime1 = (localtime1 + 1) % 100;
}

int collide2d(float x1, float y1, float x2, float y2, float wt1, float ht1, float wt2, float ht2)
{
	return (!((x1 > (x2 + wt2)) || (x2 > (x1 + wt1)) || (y1 > (y2 + ht2)) || (y2 > (y1 + ht1))));
}

int bulletdestroy = 0;
void doplayer()
{
	int pressed = 0;
	if (app.mouse.button[SDL_BUTTON_LEFT]) // try to use different time halting methods can define independently a variable only for shooting.
	{
		if (bulletdestroy < 3)
		{
			hero.row = ROW0;
			hero.frames = 4;
			int x, y;
			SDL_PumpEvents();
			SDL_GetMouseState(&x, &y);
			if (hero.facingleft == 0 && x > hero.x)
			{
				app.mouse.angle = atan2(y - hero.y + 70, x - hero.x + 50);
			}
			else if (hero.facingleft == 1 && x < hero.x)
			{
				app.mouse.angle = atan2(y - hero.y + 70, x - hero.x + 50);
			}
			if (app.mouse.angle > 90)
			{
				app.mouse.angle = 180 - app.mouse.angle;
			}
			if (globaltime % 6 == 0)
			{
				if (hero.facingleft == 0)
				{
					addbullet(hero.x, hero.y + 70, BULLET_SPEED * cos(app.mouse.angle), -BULLET_SPEED * sin(app.mouse.angle));
					bulletdestroy++;
					// bulletdestroy%=3;
				}
				else
				{
					addbullet(hero.x + 70, hero.y + 70, BULLET_SPEED * cos(app.mouse.angle), -BULLET_SPEED * sin(app.mouse.angle));
					bulletdestroy++;
					// bulletdestroy%=3;
				}
				hero.frames = hero.frames % 2 + 4;
			}
		}
	}
	if (app.mouse.button[SDL_BUTTON_RIGHT])
	{
		int x, y;
		SDL_PumpEvents();
		SDL_GetMouseState(&x, &y);
		float angle = atan2(y - hero.y + 70, x - hero.x + 50);
		if (angle > 90)
		{
			angle = 180 - angle;
		}
		// printf("%d %d\n",x,y);
		crosshair.Texture = generate_img("img/crosshair.png");
		// crosshair.x = x - hero.x+50;
		// crosshair.y = y - hero.y+70;
		// printf("%f %f\n",crosshair.x,crosshair.y);
		crosshair.x = 1.35 * BULLET_SPEED * BULLET_SPEED * sin(2 * angle) / GRAVITY;
		crosshair.y = hero.y + 20;
		SDL_Rect crosshair_source = {0, 0, 60, 62};
		// cross cordinates not finished
		SDL_Rect crosshair_destination = {crosshair.x, crosshair.y, 60, 62};
		SDL_RenderCopyEx(app.renderer, crosshair.Texture, &crosshair_source, &crosshair_destination, 0, NULL, 0);
	}
	if (app.keyboard[SDL_SCANCODE_ESCAPE])
	{
		destroy();
		exit(0);
	}
	if (hero.y < 260 && hero.onground == 0)
	{
		// hero.dy = 0;
		hero.dy += GRAVITY + 1;
		// hero.onground =1;
	}
	else
	{
		hero.onground = 1;
		hero.dy = 0;
		if (app.keyboard[SDL_SCANCODE_W])
		{
			if (hero.y < 420)
			{
				hero.row = ROW3;
				hero.frames = 1;
			}
			// else
			pressed = 1;
			hero.dy = -20;
			hero.onground = 0;
			printf("up\n");
		}
		// if (app.keyboard[SDL_SCANCODE_DOWN])
		// {
		// 	hero.dy = PLAYER_SPEED;
		// 	printf("down\n");
		// }
		else if (app.keyboard[SDL_SCANCODE_A])
		{
			hero.facingleft = 1;
			if (hero.x < -1)
			{
				hero.dx = 0;
			}
			else
			{
				hero.dx -= 0.5f;
				if (globaltime % 3 == 0)
				{
					hero.frames++;
					hero.frames %= 4;
					hero.row = ROW1;
				}
				if (hero.dx < -15)
				{
					hero.dx = -15;
					if (globaltime % 3 == 0)
					{
						hero.frames++;
						hero.frames %= 2;
						hero.row = ROW2;
					}
				}
				printf("left\n");
			}
		}
		else if (app.keyboard[SDL_SCANCODE_D])
		{
			hero.facingleft = 0;
			if (hero.x > 1300)
			{
				hero.dx = 0;
			}
			else
			{
				hero.dx += 0.5f;
				if (globaltime % 3 == 0)
				{
					hero.frames++;
					hero.frames %= 4;
					hero.row = ROW1;
				}
				if (hero.dx > 15)
				{
					hero.dx = 15;
					if (globaltime % 3 == 0)
					{
						hero.frames++;
						hero.frames %= 2;
						hero.row = ROW2;
					}
				}
				printf("right\n");
			}
		}
		else
		{
			hero.dx *= 0.8;
			if (globaltime % 10 == 0)
			{
				hero.row = ROW0;
				// hero.frames = 0;
				hero.frames = (hero.frames + 1) % 4;
			}
			if (fabsf(hero.x) < 0.1f)
			{
				hero.dx = 0;
			}
		}
	}
	if (app.keyboard[SDL_SCANCODE_W] || pressed == 1)
	{
		hero.dy -= 0.8f;
		// hero.onground = 0;
	}
	hero.x += hero.dx;
	hero.y += hero.dy;
	if (hero.dy == 0)
	{
		hero.onground = 1;
	}
	for (int i = 0; i < MAX_BULLETS; i++)
		if (bullets[i])
		{
			bullets[i]->x += bullets[i]->dx;
			bullets[i]->dy += GRAVITY;
			bullets[i]->y += bullets[i]->dy;
			if (fabs(bullets[i]->y) <= 0.1)
			{
				bullets[i]->dy = BULLET_SPEED * sin(app.mouse.angle);
			}
			// printf("%f %f\n",bullets[i]->dy,bullets[i]->dx);
			// for test purposes
			if (enemyalive.entityalive > 0)
			{
				if (bullets[i]->x > enemy->x && bullets[i]->x < enemy->x + 50 && bullets[i]->y > enemy->y && bullets[i]->y < enemy->y + 50)
				{
					enemy->lives--;
					Mix_PlayChannel(-1, hero.shoot, 0);
					if (enemy->lives == 0)
					{
						enemy->isdead = 1;
						enemy->frames = 0;
						// kills++;
					}
					removebullets(i);
					bulletdestroy--;
				}
				else if (bullets[i]->x > enemy->x && bullets[i]->x < enemy->x + 50 && bullets[i]->y > enemy->y && bullets[i]->y < enemy->y + 135)
				{
					Mix_PlayChannel(-1, hero.shootmetal, 0);
					removebullets(i);
					bulletdestroy--;
				}
				else if (bullets[i]->x > enemy1->x && bullets[i]->x < enemy1->x + 50 && bullets[i]->y > enemy1->y && bullets[i]->y < enemy1->y + 50)
				{
					enemy1->lives--;
					Mix_PlayChannel(-1, hero.shoot, 0);
					if (enemy1->lives == 0)
					{
						enemy1->isdead = 1;
						enemy1->frames = 0;
						// kills++;
					}
					removebullets(i);
					bulletdestroy--;
				}
				else if (bullets[i]->x > enemy1->x && bullets[i]->x < enemy1->x + 50 && bullets[i]->y > enemy1->y && bullets[i]->y < enemy1->y + 135)
				{
					Mix_PlayChannel(-1, hero.shootmetal, 0);
					removebullets(i);
					bulletdestroy--;
				}

				else if (bullets[i]->x > 1300 || bullets[i]->y < 10 || bullets[i]->y > 400)
				{
					removebullets(i);
					bulletdestroy--;
				}
			}
		}
	globaltime = (globaltime + 1) % 10;
}
void startscreen()
{
	SDL_Renderer *renderere = app.renderer;
	SDL_SetRenderDrawColor(renderere, 0, 0, 0, 255);
	SDL_RenderClear(renderere);
	char start[128] = "";
	sprintf(start, "GAME");
	TTF_Font *font = TTF_OpenFont("fonts/miss you.ttf", 72);
	SDL_Color red = {255, 0, 0, 255};
	// SDL_Color black ={0,0,0,255};
	SDL_Surface *tmp = TTF_RenderText_Solid(font, start, red);
	int starttextureH, starttextureW;
	starttextureH = tmp->h;
	starttextureW = tmp->w;
	SDL_Texture *starttexture = SDL_CreateTextureFromSurface(renderere, tmp);
	if (starttexture == NULL)
	{
		printf("Nothing is here in start screen texture\n");
	}
	SDL_Rect rect = {650, 150, starttextureW, starttextureH};
	SDL_RenderCopy(renderere, starttexture, NULL, &rect);
	SDL_FreeSurface(tmp);
	SDL_RenderPresent(renderere);
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			status = 1;
		}
		if (event.type == SDL_QUIT)
		{
			destroy();
			exit(1);
		}
	}
	// SDL_Delay(1000);
	TTF_CloseFont(font);
	SDL_DestroyTexture(starttexture);
}


void endscreen()
{
	SDL_Renderer *renderere = app.renderer;
	SDL_SetRenderDrawColor(renderere, 0, 0, 0, 255);
	SDL_RenderClear(renderere);
	char start[128] = "";
	sprintf(start, "KILLS: %d",kills);
	TTF_Font *font = TTF_OpenFont("fonts/miss you.ttf", 72);
	SDL_Color red = {255, 0, 0, 255};
	// SDL_Color black ={0,0,0,255};
	SDL_Surface *tmp = TTF_RenderText_Solid(font, start, red);
	int starttextureH, starttextureW;
	starttextureH = tmp->h;
	starttextureW = tmp->w;
	SDL_Texture *starttexture = SDL_CreateTextureFromSurface(renderere, tmp);
	if (starttexture == NULL)
	{
		printf("Nothing is here in start screen texture\n");
	}
	SDL_Rect rect = {650, 150, starttextureW, starttextureH};
	SDL_RenderCopy(renderere, starttexture, NULL, &rect);
	SDL_FreeSurface(tmp);
	SDL_RenderPresent(renderere);
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			destroy();
			exit(1);
		}
		if (event.type == SDL_QUIT)
		{
			destroy();
			exit(1);
		}
	}
	// SDL_Delay(1000);
	TTF_CloseFont(font);
	SDL_DestroyTexture(starttexture);
}

int randomnum(int low,int high)
{
	return (rand()%(high-low+1))+low;
}
int main(int argc, char *argv[])
{
	memset(&app, 0, sizeof(App));
	initSDL();
	memset(&hero, 0, sizeof(Entity));
	enemy = malloc(sizeof(Entity));
	memset(enemy, 0, sizeof(Entity));
	enemy1 = malloc(sizeof(Entity));
	memset(enemy1, 0, sizeof(Entity));
	enemyalive.entityalive = 1;
	enemy->isdead = 0;
	enemy->row = 100;
	enemy->x = 1450;
	enemy->y = 260;
	enemy->dx = 5;
	enemy->frames = 0;
	enemy->lives = 3;
	enemy1->isdead = 0;
	enemy1->row = 100;
	enemy1->x = -30;
	enemy1->y = 260;
	enemy1->dx = 5;
	enemy1->frames = 0;
	enemy1->lives = 3;
	hero.x = 260;
	hero.y = 0;
	hero.onground = 0;
	hero.frames = 1;
	hero.row = ROW1;
	hero.lives = 5;
	hero.isdead = 0;
	bullettexture = generate_img("img/bullet.png");
	addenemybullet(enemy->x, enemy->y + 70, enemy->dx + 10, 0);
	app.bgmusic = Mix_LoadMUS("sounds/music.ogg");
	if (app.bgmusic != NULL)
	{
		Mix_PlayMusic(app.bgmusic, -1);
		Mix_VolumeMusic(64);
	}
	else
	{
		printf("Failed to load music\n");
	}
	if (Mix_PausedMusic())
	{
		printf("Music not playing\n");
	}
	hero.shoot = Mix_LoadWAV("sounds/glasshit.mp3");
	hero.shootmetal = Mix_LoadWAV("sounds/metalhit.mp3");
	hero.hurt = Mix_LoadWAV("sounds/hurt.mp3");
	Mix_VolumeChunk(hero.shoot, 24);
	Mix_VolumeChunk(hero.shootmetal, 24);
	int flag =0;
	while (1)
	{
		if (status == 1)
		{
			Mix_ResumeMusic();
			Mix_VolumeMusic(18);
			SDL_RenderClear(app.renderer);
			SDL_Texture *bg = generate_img("img/bg1.png");
			generatebg(bg);
			hero.charachter = generate_img("img/Characterspritesbw.png");
			initPlayer(hero.charachter, hero.x, hero.y);
			if (enemyalive.entityalive == 1)
			{
				spawnenemyright();
				if (kills%3 == 2||flag ==1)
				{
					flag =1;
					spawnenemyleft();
				}
			}
			if (hero.isdead == 0)
			{
				doInput();
				doplayer();
			}
			SDL_RenderPresent(app.renderer);
			SDL_Delay(4);
			// destroyenemytexture();
			SDL_DestroyTexture(hero.charachter);
			SDL_DestroyTexture(crosshair.Texture);
			SDL_DestroyTexture(enemy->charachter);
			SDL_DestroyTexture(app.heart);
			SDL_DestroyTexture(bg);
		}
		else if(status == 2)
		{
			Mix_PauseMusic();
			endscreen();
		}
		else
		{
			startscreen();
		}
	}
	return 0;
}

/*
remaing tasks:
	Check for all comments to see if any edit required
	crosshair location
	shoot animation fix
	optional:
		render cahrachter and gun seperately

*/
/*
	crosshair getting randomly located
*/
/*
	1st fail
	couldnt set crosshair to bullet projectile end*/
/*2nd fail
	couldn't spawn a number of enemies at once*/
/*
	1st Fail

	finally able to spawn more than one enemies
	but
	FPS drops significantly as the number of entites increases

	One possible solution:
		use the original spawn enemy algorithm and spawn one enemies at one then after killing reset is position of x*/
/**/
/**/