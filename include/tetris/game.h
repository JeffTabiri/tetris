  #ifndef TETRIS_GAME_H
  #define TETRIS_GAME_H
  #include <SDL3/SDL_render.h>


  class Game {
    struct Tetrimino {

    };

    struct Cell {
      bool is_active = {false};
      Uint8 r = {0};
      Uint8 g = {0};
      Uint8 b = {0};
    };

    private:
      SDL_Window *window = nullptr;
      SDL_Renderer *renderer = nullptr;
      SDL_Event event = {};
      bool is_running = {true};
      int grid[20][10] = {};
      Cell new_grid[20][10] = {};
      int x_position = {0};
      int y_position = {0};

      bool check_boundary(int new_y, int new_x);
      void draw_field();
      void move_down();
      void move(int direction);
      void update(uint32_t *time);
      void render();
      void process_input();
      void spawn_block();
      void rotate_block();
      void clear_blocks();
      bool check_rotations();
      void reset();
      void check_random_bag();
      bool check_collisions(int new_y, int new_x);


    public:
      Game();
      ~Game();
      bool init();
      void run();
      void quit();
  };
  #endif
