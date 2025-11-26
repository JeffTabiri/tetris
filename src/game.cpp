#include "tetris/game.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <iterator>
#include <random>

// rate settings
constexpr int DROP_RATE{1000 / 60};

// game setting
constexpr Uint32 SDL_FLAGS = SDL_INIT_VIDEO;
constexpr int CELL_SIZE { 30 };
constexpr int FIELD_WIDTH { 10 };
constexpr int FIELD_HEIGHT { 20 };
constexpr int SPAWN_POINT[2] { 0, 6 };

// window setting
constexpr int WINDOW_WIDTH = { FIELD_WIDTH * CELL_SIZE };
constexpr int WINDOW_HEIGHT = { FIELD_HEIGHT * CELL_SIZE };

const int TETRIMINOES[7][8] = {
  // I
  {0,0, 0,1, 0,2, 0,3},

  // O
  {0,0, 0,1, 1,0, 1,1},

  // S
  {0,1, 0,2, 1,0, 1,1},

  // Z
  {0,0, 0,1, 1,1, 1,2},

  // L
  {0,0, 1,0, 2,0, 2,1},

  // J
  {0,1, 1,1, 2,1, 2,0},

  // T
  {0,0, 0,1, 0,2, 1,1}
};

int current_bag_idx = 0;
int bag_sequence[7] = {0, 1, 2, 3, 4, 5, 6};

enum GameStates {
  PAUSE,
  NEW_BLOCK,
  PLAY,
  FINISHED,
};

// player settings
int current_x = 0;
int current_y = 0;
int current_block[8] = {};

GameStates current_game_state;

Game::Game() {}

void Game::check_random_bag() {
    if (current_bag_idx == 7) {
      current_bag_idx = 0;
      std::shuffle(std::begin(bag_sequence), std::end(bag_sequence),
                   std::default_random_engine(2)); // NOLINT
    }
}

void Game::spawn_block() {

  for (int i = 0; i < 8; ++i) {
    current_block[i] = TETRIMINOES[bag_sequence[current_bag_idx]][i];
  }

  current_bag_idx++;

  current_y = 0;
  current_x = FIELD_WIDTH / 2 - 2; //center horizontally

  // create blocks
  for (int i = 0; i < 8; i += 2) {
    int y = current_y + current_block[i];
    int x = current_x + current_block[i + 1];
    grid[y][x] = true;
  }

  current_game_state = PLAY;

}

void Game::render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  // draw the field
  draw_field();
  // swap buffers and present
  SDL_RenderPresent(renderer);
}

void Game::process_input() {
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
      if (event.key.scancode == SDL_SCANCODE_LEFT) {
        move_left();
      } else if (event.key.scancode == SDL_SCANCODE_RIGHT) {
        move_right();
      }
      break;
    case SDL_EVENT_QUIT:
      is_running = false;
      break;
    default:
      break;
    }
  }
}


bool Game::check_boundary(int new_y, int new_x) {
  for (int i = 0; i < 8; i += 2) {
    int cy = new_y + current_block[i];
    int cx = new_x + current_block[i + 1];

    // Out of bounds
    if (cx < 0 || cx >= FIELD_WIDTH ||
        cy < 0 || cy >= FIELD_HEIGHT) {
      return true;}

  }
  return false;
}


void Game::draw_field() {

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

  for (int i = 1; i <= FIELD_WIDTH; ++i) {
    const int x = i * CELL_SIZE;
    SDL_RenderLine(renderer, x, 0, x, WINDOW_HEIGHT);
  }

  for (int i = 1; i <= FIELD_HEIGHT; ++i) {
    const int y = i * CELL_SIZE;
    SDL_RenderLine(renderer, 0, y, WINDOW_WIDTH, y);
  }

  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    for (int j = 0; j < FIELD_WIDTH; ++j) {
      if (grid[i][j]) {
        SDL_FRect rect = {j * 30.0f, i * 30.0f, CELL_SIZE, CELL_SIZE};
        SDL_RenderFillRect(renderer, &rect);
      }
    }
  }
}

bool Game::init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!window) {
    std::cerr << "SDL Error: " << "Error creating window:\n"
              << SDL_GetError() << std::endl;
    return false;
  }

  renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    std::cerr << "SDL Error: " << "Error creating renderer:\n"
              << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}

void Game::move_right() {
  if (check_boundary(current_y, current_x + 1)) return;
  if (check_collisions(current_y, current_x + 1)) return;

  for (int i = 0; i < 8; i += 2)
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = false;

  current_x++;

  for (int i = 0; i < 8; i += 2)
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = true;
}

void Game::move_left() {
  if (check_boundary(current_y, current_x - 1)) return;
  if (check_collisions(current_y, current_x - 1)) return;

  for (int i = 0; i < 8; i += 2)
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = false;

  current_x--;

  for (int i = 0; i < 8; i += 2)
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = true;
}


void Game::move_down() {
  // test next position
  printf("%d, %d", current_y, current_x);
  printf("\n");
  if (check_boundary(current_y + 1, current_x)) {
    current_game_state = NEW_BLOCK;  // lock piece
    return;
  }

  if (check_collisions(current_y + 1, current_x)) {
    for (int i = 0; i < 8; i += 2) {
      grid[current_y + current_block[i]][current_x + current_block[i + 1]] = true;
    }
    current_game_state = NEW_BLOCK;
    return;
  }

  // erase current
  for (int i = 0; i < 8; i += 2) {
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = false;
  }

  current_y++;

  // draw at new position
  for (int i = 0; i < 8; i += 2) {
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = true;
  }

  printf("%d, %d", current_y, current_x);
  printf("\n");
}

bool Game::check_collisions(int new_y, int new_x) {

  for (int i = 0; i < 8; i += 2) {
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = false;
  }

  for (int i = 0; i < 8; i += 2) {
    int cy = new_y + current_block[i];
    int cx = new_x + current_block[i + 1];

    // collided with an element.
    if (grid[cy][cx] == true) {
      return true;
    }
  }

  return false;
}

void Game::reset() {
  current_game_state = NEW_BLOCK;
  current_x = 0;
  current_y = 0;
}

void Game::run() {
  Uint32 frame_start = SDL_GetTicks();
  std::shuffle(std::begin(bag_sequence), std::end(bag_sequence),
                  std::default_random_engine(2)); // NOLINT
  spawn_block();

  while (is_running) {

    // input
    process_input();

    if (current_game_state == NEW_BLOCK) {
      spawn_block();
    }

    // handle gravity every second
    if (SDL_GetTicks() - frame_start >= 200) {
      move_down();
      frame_start = SDL_GetTicks();
    }

    // update

    // render
    render();
  }
}

void Game::quit() {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }

  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }

  SDL_Quit();
}

Game::~Game() {
  printf("Quitting Game ...\n");
  quit();
}
