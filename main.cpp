#include <iostream>
#include <SDL3/SDL.h>



int main(int argc, char *argv[]) {

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Viola", 800, 600, 0);

    if (window == nullptr) {
        std::cerr << "Failed to create window" << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(window, NULL);
    if (ren == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    SDL_Event event;
    bool quit = false;

    SDL_FRect green_square {200, 100, 300, 200};

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            SDL_SetRenderDrawColor(ren, 0, 0 ,0, 255);
            SDL_RenderClear(ren);

            SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
            SDL_RenderFillRect(ren, &green_square);

            SDL_RenderPresent(ren);
        }
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();


    std::cout << "Hello World" << std::endl;
    return 0;

}
