#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <SDL_ttf.h>


//Block dimension constants
const int BLOCK_WIDTH = 30;
const int BLOCK_HEIGHT = 30;

//Screen dimension constants
const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 900;

//starting point
const int START_X = 180;
const int START_Y = 150;

struct Block {
	int x; //x coordinate
	int y; //y..
};

struct Body {
	struct Block part_pos;
	struct Body* next_body;
};

struct Direction {
	int right;
	int down;
	int left;
	int up;
};

struct Snake {
	struct Direction snake_direction;
	struct Body* head;
};

struct Body *create_body(struct Block pos);
void add_to_tail(struct Snake *game_snake);
struct Snake create_snake();
void render_snake(SDL_Renderer* renderer, struct Snake game_snake);
void move_snake(struct Snake *game_snake);
void get_input(struct Snake *game_snake, SDL_Event event);
struct Block* generate_apple(struct Snake game_snake);
void draw_apple(struct Block* apple, SDL_Renderer* renderer);
struct Block* check_if_score(struct Snake* game_snake, struct Block* apple);
int check_if_lost(struct Snake* snake);
void draw_grid(SDL_Renderer* renderer);
void game_over(SDL_Renderer* renderer, struct Snake* snake);


int main() {

    RESTART:

    int restart = 0;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* casioTexture = NULL;

    int quit = 0;
    SDL_Event event;

    int result = SDL_Init(SDL_INIT_EVERYTHING);
    int result1 = TTF_Init();

    window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
        goto CLEANUP_AND_QUIT;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        goto CLEANUP_AND_QUIT;

    struct Snake game_snake = create_snake();
    struct Block* apple = generate_apple(game_snake);
    
    printf("x-> %d\ny-> %d", apple->x, apple->y);
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_KEYUP:
                get_input(&game_snake, event);
                break;
            }
        }

        if (check_if_lost(&game_snake)) {
            game_over(renderer, &game_snake);
            restart = 1;
            goto CLEANUP_AND_QUIT;
        }
        SDL_RenderClear(renderer);
        draw_grid(renderer);

        render_snake(renderer, game_snake);
        move_snake(&game_snake);
        draw_apple(apple, renderer);
        apple = check_if_score(&game_snake, apple);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);
        Sleep(200);
    }

CLEANUP_AND_QUIT:

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    if (restart) {
        goto RESTART;
    }
    return 0;
}

struct Body *create_body(struct Block pos) {
	struct Body* new_body = (struct Body*)malloc(sizeof(struct Body));
	if (new_body == NULL) {
		perror("couldn't create body");
		exit(1);
	}
		new_body->part_pos.x = pos.x;
		new_body->part_pos.y = pos.y;

		new_body->next_body = NULL;

		return new_body;
}

void add_to_tail(struct Snake *game_snake) {
    struct Body* current = game_snake->head;
    if (current != NULL) { // head already exists
            for (; current->next_body != NULL; current = current->next_body);

            struct Block add_block = { .x = current->part_pos.x, .y = current->part_pos.y + BLOCK_HEIGHT };
            current->next_body = create_body(add_block);
    }
    else { // create head
        struct Block add_block = { .x = START_X, .y = START_Y };
        game_snake->head = create_body(add_block);
    }
}

void draw_grid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    int gridSize = BLOCK_WIDTH;  

    for (int x = 0; x < SCREEN_WIDTH; x += gridSize) {
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
    }

    for (int y = 0; y < SCREEN_HEIGHT; y += gridSize) {
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }
}

struct Snake create_snake() {
	struct Snake game_snake;
	struct Direction init_direction = { 1, 0, 0, 0 }; // right direction flag is on
	game_snake.snake_direction = init_direction;
    game_snake.head = NULL;
    add_to_tail(&game_snake);
    add_to_tail(&game_snake);
    add_to_tail(&game_snake);

	return game_snake;
}

void render_snake(SDL_Renderer* renderer, struct Snake game_snake) {

    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    SDL_Rect body_block;
    body_block.w = BLOCK_WIDTH;
    body_block.h = BLOCK_HEIGHT;


    while (game_snake.head != NULL) {
        body_block.x = game_snake.head->part_pos.x;
        body_block.y = game_snake.head->part_pos.y;
        SDL_RenderFillRect(renderer, &body_block);

        //SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
        //SDL_Rect borderRect = { (body_block.x)/2, (body_block.y)/2, (body_block.w)/5, body_block.h};
        //SDL_RenderDrawRect(renderer, &body_block);

        game_snake.head = game_snake.head->next_body;
    }

}

void move_snake(struct Snake *game_snake) {
    int iprev_x, prev_x, iprev_y, prev_y;
    iprev_x = game_snake->head->part_pos.x;
    iprev_y = game_snake->head->part_pos.y;

    if (game_snake->snake_direction.right) {
        game_snake->head->part_pos.x += BLOCK_WIDTH;
    }
    else if (game_snake->snake_direction.down) {
        game_snake->head->part_pos.y += BLOCK_HEIGHT;
    }
    else if (game_snake->snake_direction.left) {
        game_snake->head->part_pos.x -= BLOCK_WIDTH;
    }
    else if (game_snake->snake_direction.up) {
        game_snake->head->part_pos.y -= BLOCK_HEIGHT;
    }

    struct Body* current = game_snake->head;
    if (current->next_body != NULL)
    {
        current = current->next_body;
    }
    while (current != NULL) {
        prev_x = current->part_pos.x;
        prev_y = current->part_pos.y;

        current->part_pos.x = iprev_x;
        current->part_pos.y = iprev_y;

        iprev_x = prev_x;
        iprev_y = prev_y;

        current = current->next_body;
    }
}

void get_input(struct Snake* game_snake, SDL_Event event) {
    switch (event.key.keysym.sym) {
        case SDLK_UP:
            if (game_snake->snake_direction.up || game_snake->snake_direction.down) {
                return;
            }
            else {
                struct Direction new_dir = { .right = 0, .down = 0, .left = 0, .up = 1 };
                game_snake->snake_direction = new_dir;
            }
            break;
        case SDLK_DOWN:
            if (game_snake->snake_direction.up || game_snake->snake_direction.down) {
                return;
            }
            else {
                struct Direction new_dir = { .right = 0, .down = 1, .left = 0, .up = 0 };
                game_snake->snake_direction = new_dir;
            }
            break;
        case SDLK_RIGHT:
            if (game_snake->snake_direction.left || game_snake->snake_direction.right) {
                return;
            }
            else {
                struct Direction new_dir = { .right = 1, .down = 0, .left = 0, .up = 0 };
                game_snake->snake_direction = new_dir;
            }
            break;
        case SDLK_LEFT:
            if (game_snake->snake_direction.left || game_snake->snake_direction.right) {
                return;
            }
            else {
                struct Direction new_dir = { .right = 0, .down = 0, .left = 1, .up = 0 };
                game_snake->snake_direction = new_dir;
            }
            break;
        default:
            return;
    }
    return;
}

void draw_apple(struct Block* apple, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    SDL_Rect apple_block;
    apple_block.w = BLOCK_WIDTH;
    apple_block.h = BLOCK_HEIGHT;
    apple_block.x = apple->x;
    apple_block.y = apple->y;
    SDL_RenderFillRect(renderer, &apple_block);
}

int is_apple_on_snake(struct Snake game_snake, struct Block *apple) {

    while (game_snake.head != NULL) {
        if (game_snake.head->part_pos.x == apple->x && game_snake.head->part_pos.y == apple->y) {
            return 1; // on snake
        }
        game_snake.head = game_snake.head->next_body;
    }
    return 0; // not on snake
}

struct Block* generate_apple(struct Snake game_snake) {
    struct Block* apple = (struct Block*)malloc(sizeof(struct Block));
    if (apple == NULL) {
        perror("couldn't generate apple");
        exit(1);
    }
    srand(time(NULL));
    do {
        apple->x = rand() % SCREEN_WIDTH;
        apple->y = rand() % SCREEN_HEIGHT;
    } while (apple->x % BLOCK_WIDTH || apple->y % BLOCK_WIDTH || is_apple_on_snake(game_snake, apple));

    return apple;
}

struct Block* check_if_score(struct Snake* game_snake, struct Block* apple) {
    if (game_snake->head->part_pos.x == apple->x && game_snake->head->part_pos.y == apple->y) {
        //printf("in if lolz");
        add_to_tail(game_snake);
        free(apple);
        return generate_apple(*game_snake);
    }
    return apple;
}

void game_over(SDL_Renderer* renderer, struct Snake* snake2_cleanup) {
    while (snake2_cleanup->head != NULL) {
        struct Body* tmp = snake2_cleanup->head;
        snake2_cleanup->head = snake2_cleanup->head->next_body;
        free(tmp);
    }

    TTF_Font* font = TTF_OpenFont("font/usefont.ttf", 36);
    if (font == NULL) {
        printf("TTF Error: %s\n", TTF_GetError());
        exit(1);
    }

    SDL_Color textColor = { 255, 255, 255, 255 }; // White color (R, G, B, A)

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "YOU LOSE, PRESS SPACEBAR TO RESTART", textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect;
    textRect.x = (SCREEN_WIDTH - textSurface->w) / 2;
    textRect.y = (SCREEN_HEIGHT - textSurface->h) / 2;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;

    SDL_Event event;
    int quit = 0;
    printf("game over noob\n");
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                exit(0);
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_SPACE) {
                    quit = 1;
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
}

int check_if_lost(struct Snake* snake) {
    struct Body* current = snake->head;
    if (current == NULL) {
        perror("wtf bro snkae not created");
        exit(1);
    }
    while (current = current->next_body, current != NULL) {
        if ((current->part_pos.x == snake->head->part_pos.x && current->part_pos.y == snake->head->part_pos.y) || snake->head->part_pos.x > SCREEN_WIDTH ||
            snake->head->part_pos.x < 0 || snake->head->part_pos.y > SCREEN_HEIGHT || snake->head->part_pos.y < 0) {
            printf("gameover noob");
            return 1;
        }
    }
    return 0;
}
