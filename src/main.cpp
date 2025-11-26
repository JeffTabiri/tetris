#include "tetris/game.h"
#include <SDL3/SDL_main.h>
#include <iostream>

int main(int argc, char *argv[]) {
  Game game;
  int exit_status = EXIT_FAILURE;
  if (game.init()) {
    game.run();
    exit_status = EXIT_SUCCESS;
  }

  return exit_status;
}
