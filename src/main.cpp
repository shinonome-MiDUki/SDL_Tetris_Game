#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <vector>
#include <array>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>
#include <string>

static const int NUM = 8;

static const double SCREEN_LEN = 480.0;

static const double pi = 3.141592653589793;

static constexpr double LEN_PER_GRID = SCREEN_LEN / NUM;


struct GlobalVector2{
    double x; double y;
};

struct GridVector2{
    int x; int y;
};

GridVector2 globalToGrid(GlobalVector2 global_vec){
    double global_x = global_vec.x;
    double global_y = global_vec.y;
    return GridVector2{
        .x = static_cast<int>((global_x - (LEN_PER_GRID / 2)) / LEN_PER_GRID), 
        .y = static_cast<int>((global_y - (LEN_PER_GRID / 2)) / LEN_PER_GRID)
    };
}

GlobalVector2 gridToGlobal(GridVector2 grid_vec){
    int grid_x = grid_vec.x;
    int grid_y = grid_vec.y;
    return GlobalVector2{
        .x = static_cast<double>((grid_x * LEN_PER_GRID + (LEN_PER_GRID / 2))),
        .y = static_cast<double>((grid_y * LEN_PER_GRID + (LEN_PER_GRID / 2)))
    };
}


int main(int argc, char* argv[]){
    if (!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("SDL Loading failed");
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
        "Trtris", SCREEN_LEN, SCREEN_LEN, SDL_WINDOW_RESIZABLE,
        &window, &renderer
    )){
        SDL_Log("SDL Window creation failed");
        SDL_Quit();
        return 1;
    }

    bool running{true};
    SDL_Event event;

    struct Osero
    {
        SDL_Rect osero_block;
        char who;
    }; //w = white b = black
    
    std::array<std::array<char, 8>, 8> game_grid{{ 'n' }}; // n = nothing w = white b = black
    game_grid[3][3] = 'w';
    game_grid[4][4] = 'w';
    game_grid[3][4] = 'b';
    game_grid[4][3] = 'b';

    while (running)
    {
        for (int _x = 0; _x < NUM; ++_x){
            for (int _y = 0; _y < NUM; ++_y){
                if (game_grid[_x][_y] == 'n') { continue; }
                double color = game_grid[_x][_y] == 'w' ? 255.0 : 0.0;
                GlobalVector2 global_vec = gridToGlobal(
                    GridVector2{.x = _x, .y = _y}
                );
                SDL_Rect osero{
                    .x = global_vec.x, .y = global_vec.y,
                    .w = LEN_PER_GRID * 0.75, .h = LEN_PER_GRID * 0.75
                };
                SDL_SetRenderDrawColor(renderer, color)
            }
        }
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}