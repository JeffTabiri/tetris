#include "tetris/game.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <iterator>
#include <random>


// game setting
constexpr Uint32 SDL_FLAGS { SDL_INIT_VIDEO };
constexpr int CELL_SIZE { 30 };
constexpr int FIELD_WIDTH { 10 };
constexpr int FIELD_HEIGHT { 20 };
constexpr int SPAWN_POINT[2] { 0, 6 };

// window setting
constexpr int WINDOW_WIDTH = { FIELD_WIDTH * CELL_SIZE };
constexpr int WINDOW_HEIGHT = { FIELD_HEIGHT * CELL_SIZE };

typedef struct TETRIMINOES {
  int positions[4][8];
};

const TETRIMINOES tetriminoes[7] = {
  // I piece
  {
    {
      {1,0, 1,1, 1,2, 1,3},
      {0,2, 1,2, 2,2, 3,2},
      {2,0, 2,1, 2,2, 2,3},
      {0,1, 1,1, 2,1, 3,1},
    }
  },

  // O piece (same rotation for all)
  {
          {
            {0,0, 1,0, 0,1, 1,1},
            {0,0, 1,0, 0,1, 1,1},
            {0,0, 1,0, 0,1, 1,1},
            {0,0, 1,0, 0,1, 1,1},
        }
  },

  // T piece
  {
          {
            {1,0, 1,1, 1,2, 0,1},   // Up
            {0,1, 1,1, 2,1, 1,2},   // Right
            {1,0, 1,1, 1,2, 2,1},  // Down
            {1,1, 2,1, 3,1, 2,0},  // Left
        }
  },

  // J piece
  {
          {
            {0,0, 1,0, 1,1, 1,2},   // Up
            {0,1, 1,1, 2,1, 0,2},   // Right
            {1,0, 1,1, 1,2, 2,2},   // Down
            {0,1, 1,1, 2,1, 2,0},   // Left
        }
  },

  // L piece
  {
          {
            {1,0, 1,1, 1,2, 0,2},     // Up
            {0,1, 1,1, 2,1, 2,2},    // Right
            {1,0, 1,1, 1,2, 2,0},   // Down
            {0,0, 0,1, 1,1, 2,1},    // Left
        }
  },

  // S piece
  {
          {
            {1,0, 1,1, 0,1, 0,2},    // Up
            {0,1, 1,1, 1,2, 2,2},    // Right
            {2,0, 2,1, 1,1, 1,2},    // Down (same)
            {0,0, 1,0, 1,1, 2,1},    // Left (same)
        }
  },

  // Z piece
  {
          {
            {0,0, 0,1, 1,1, 1,2},    // Up
            {0,2, 1,2, 1,1, 2,1},    // Right
            {1,0, 1,1, 2,1, 2,2},    // Down (same)
            {0,2, 1,2, 1,1, 2,1},    // Left (same)
        }
  },
};

int current_bag_idx = 0;
int bag_sequence[7] = {0, 1, 2, 3, 4, 5, 6};

enum GameStates {
  PAUSE,
  NEW_BLOCK,
  PLAY,
  FINISHED,
};
GameStates current_game_state;

// player settings
int current_x = 0;
int current_y = 0;
int current_block[8] = {};
int current_rotation = 0;
int drop_rate = 200;

Game::Game() {}

void Game::check_random_bag() {
    if (current_bag_idx == 6) {
      current_bag_idx = 0;
      std::shuffle(std::begin(bag_sequence), std::end(bag_sequence),
                   std::default_random_engine(2)); // NOLINT
    } else {
      current_bag_idx++;
    }
}

void Game::spawn_block() {
  clear_blocks();

  current_y = 0;
  current_rotation = 0;
  current_x = FIELD_WIDTH / 2 - 2;  //center horizontally

  current_bag_idx = rand() % 7;

  for (int i = 0; i < 8; ++i) {
    current_block[i] = tetriminoes[bag_sequence[current_bag_idx]].positions[current_rotation][i];
  }


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
      } else if (event.key.scancode == SDL_SCANCODE_UP) {
        rotate_block();
      } else if (event.key.scancode == SDL_SCANCODE_SPACE) {

      } else if (event.key.scancode == SDL_SCANCODE_DOWN) {
        drop_rate = 50;
      }

      break;
    case SDL_EVENT_KEY_UP:
      if (event.key.scancode == SDL_SCANCODE_DOWN) {
        drop_rate = 200;
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
    if (cx < 0 || cx >= FIELD_WIDTH || cy < 0 || cy >= FIELD_HEIGHT) {
      return true;
    }
  }
  return false;
}

void Game::rotate_block() {
  for (int i = 0; i < 8; i += 2)
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = false;

  current_rotation = (current_rotation + 1) % 4;



  // Get the rotated block
  for (int i = 0; i < 8; i++) {
    current_block[i] = tetriminoes[bag_sequence[current_bag_idx]].positions[current_rotation][i];
  }

  for (int i = 0; i < 8; i += 2)
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = true;
}


void Game::update() {
}

void Game::clear_blocks() {

  for (int i = FIELD_HEIGHT - 1; i >= 0; i--) {

    int count = 0;
    for (int j = 0; j < FIELD_WIDTH; j++) {
      if (grid[i][j]) count++;
    }

    if (count == FIELD_WIDTH) {

      // Move all rows above down by 1
      for (int row = i; row > 0; row--) {
        for (int col = 0; col < FIELD_WIDTH; col++) {
          grid[row][col] = grid[row - 1][col];
        }
      }

      // Clear the top row
      for (int col = 0; col < FIELD_WIDTH; col++) {
        grid[0][col] = false;
      }

      // IMPORTANT: check the *same* row index again
      i++;
    }
  }
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

  // remove the blocks from screen
  for (int i = 0; i < 8; i += 2)
    grid[current_y + current_block[i]][current_x + current_block[i + 1]] = false;

  current_x++;

  // place the blocks on screen
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
}

bool Game::check_collisions(int new_y, int new_x) {

    for (int i = 0; i < 8; i += 2) {
      int cy = new_y + current_block[i];
      // Out of bounds
      if (cy < 0 || cy >= FIELD_HEIGHT) {
        return true;
      }
      }


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
    if (SDL_GetTicks() - frame_start >= drop_rate) {
      move_down();
      frame_start = SDL_GetTicks();
    }

    // update
    update();

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
