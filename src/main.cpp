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

static constexpr int NUM = 8;

static constexpr double SCREEN_LEN = 480.0;

static constexpr double pi = 3.141592653589793;

static constexpr double LEN_PER_GRID = SCREEN_LEN / NUM;

static constexpr double LEN_PER_CELL = LEN_PER_GRID * 0.75;

static constexpr double OFFSET = (LEN_PER_GRID - LEN_PER_CELL) / 2;



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
        .x = static_cast<int>((global_x - OFFSET) / LEN_PER_GRID), 
        .y = static_cast<int>((global_y - OFFSET) / LEN_PER_GRID)
    };
}

GlobalVector2 gridToGlobal(GridVector2 grid_vec){
    int grid_x = grid_vec.x;
    int grid_y = grid_vec.y;
    return GlobalVector2{
        .x = static_cast<double>((grid_x * LEN_PER_GRID + OFFSET)),
        .y = static_cast<double>((grid_y * LEN_PER_GRID + OFFSET))
    };
}

void CheckCell(
    char cell_content,
    char myself,
    GridVector2 checking_cell,
    int& buf_counter,
    std::vector<GridVector2>& buf,
    bool& buffering_flag
){
    if (cell_content == 'n'){
        buffering_flag = false;
        for (int _ = 0; _ < buf_counter; ++_){
            if (!buf.empty()){
                buf.pop_back();
            }
        }
        buf_counter = 0;
        return;
    } else if (cell_content == myself && !buffering_flag){
        buffering_flag = true;
        return;
    } else if (cell_content == myself && buffering_flag){
        buffering_flag = false;
        buf_counter = 0;
        return;
    } else if (cell_content != myself && buffering_flag){
        ++buf_counter;
        buf.emplace_back(checking_cell);
    } 
}

std::vector<GridVector2> ScanSurroundings(
    GridVector2 places_vec, 
    char myself, 
    const std::array<std::array<char, 8>, 8>& game_grid
){
    int placed_x = places_vec.x;
    int placed_y = places_vec.y;
    std::vector<GridVector2> buf{};
    bool buffering_flag{false};
    int buf_counter{0};
    // check row
    for (int _x = 0; _x < NUM; ++_x){
        CheckCell(
            game_grid[_x][placed_y],
            myself,
            GridVector2{.x = _x, .y = placed_y},
            buf_counter,
            buf,
            buffering_flag
        );
    }
    buffering_flag = false;

    // check col
    for (int _y = 0; _y < NUM; ++_y){
        CheckCell(
            game_grid[placed_x][_y],
            myself,
            GridVector2{.x = placed_x, .y = _y},
            buf_counter,
            buf,
            buffering_flag
        );
    }
    buffering_flag = false;

    // check upleft to downright
    std::array<int, 2> stating_point{};

    int y_intercepting_x = placed_x > placed_y ? 7 : 0;
    int y_intercept = (y_intercepting_x - placed_x) + placed_y;
    int iter_count = placed_x > placed_y ? y_intercept + 1 : (7 - y_intercept) + 1;
    stating_point = placed_x > placed_y ? std::array<int, 2>{y_intercepting_x - y_intercept, 0} : std::array<int, 2>{0, y_intercept};

    for (int c = 0; c < iter_count; ++c){
        CheckCell(
            game_grid[stating_point[0] + c][stating_point[1] + c],
            myself,
            GridVector2{.x = stating_point[0] + c, .y = stating_point[1] + c},
            buf_counter,
            buf,
            buffering_flag
        );
    }
    buffering_flag = false;

    // check upright to downleft

    y_intercepting_x = placed_x + placed_y > 7 ? 7 : 0;
    y_intercept = -1 * (y_intercepting_x - placed_x) + placed_y;
    iter_count = placed_x + placed_y > 7 ? (7 - y_intercept) + 1 : y_intercept + 1;
    stating_point = placed_x + placed_y > 7 ? std::array<int, 2>{7, y_intercept} : std::array<int, 2>{y_intercept, 0};

    for (int c = 0; c < iter_count; ++c){
        CheckCell(
            game_grid[stating_point[0] - c][stating_point[1] + c],
            myself,
            GridVector2{.x = stating_point[0] - c, .y = stating_point[1] + c},
            buf_counter,
            buf,
            buffering_flag
        );
    }
    buffering_flag = false;

    return buf;
}

// template<typename T>
// bool DoContain(const std::vector<T>& list, T target){
//     return std::find(list.begin(), list.end(), target) != list.end();
// }

// template<typename T>
// bool DoContain(const std::array<T, 2>& list, T target){
//     return std::find(list.begin(), list.end(), target) != list.end();
// }


int main(int argc, char* argv[]){
    if (!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("SDL Loading failed");
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
        "Osero", SCREEN_LEN, SCREEN_LEN, SDL_WINDOW_RESIZABLE,
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
    
    std::array<std::array<char, 8>, 8> game_grid{{}}; // n = nothing w = white b = black
    for (std::array<char, 8>& k : game_grid){ k.fill('n'); }
    game_grid[3][3] = 'w';
    game_grid[4][4] = 'w';
    game_grid[3][4] = 'b';
    game_grid[4][3] = 'b';

    bool is_blacks_turn{true};
    bool is_game_ended{false};

    GridVector2 current_on{GridVector2{.x = 5, .y = 4}};

    while (running)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) { 
                running = false;
            } 
            if (event.type == SDL_EVENT_KEY_DOWN){
                if (event.key.scancode == SDL_SCANCODE_LEFT){
                    if (current_on.x - 1 >= 0){ -- current_on.x; }
                } else if (event.key.scancode == SDL_SCANCODE_RIGHT){
                    if (current_on.x + 1 <= 7){ ++ current_on.x; }
                } else if (event.key.scancode == SDL_SCANCODE_UP){
                   if (current_on.y - 1 >= 0){ -- current_on.y; }
                } else if (event.key.scancode == SDL_SCANCODE_DOWN){
                    if (current_on.y + 1 <= 7){ ++ current_on.y; }
                } else if (event.key.scancode == SDL_SCANCODE_RETURN){
                    if (game_grid[current_on.x][current_on.y] != 'n'){ break; }
                    char myself = is_blacks_turn ? 'b' : 'w';
                    game_grid[current_on.x][current_on.y] = myself;
                    std::vector<GridVector2> turnables = ScanSurroundings(current_on, myself, game_grid);
                    if (turnables.empty()){
                        game_grid[current_on.x][current_on.y] = 'n';
                        break; 
                    }
                    for (GridVector2 turnable : turnables){
                        game_grid[turnable.x][turnable.y] = myself;
                    }
                    is_blacks_turn = !is_blacks_turn;
                } else if (event.key.scancode == SDL_SCANCODE_SPACE){
                    if (is_game_ended) {
                        running = false;
                        break;
                    }
                } else if (event.key.scancode == SDL_SCANCODE_D){
                    //DEBUG
                    std::cout << "----------" << "\n";
                    for (int _y = 0; _y < NUM; ++_y){
                        for (int _x = 0; _x < NUM; ++_x){
                            std::cout << game_grid[_x][_y] << " | ";
                        }
                        std::cout << "\n";
                    }
                    std::cout << "----------" << "\n";
                    //DEBUG
                }
            }
        }
        if (!running) { break; }

         SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 28, 58, 88, 255);
        SDL_RenderClear(renderer);
        
        for (int _x = 0; _x < NUM; ++_x){
            for (int _y = 0; _y < NUM; ++_y){
                double color{};
                double alpha{};
                if (game_grid[_x][_y] == 'n') {
                    color = 255.0;
                    alpha = 35.0;
                } else{
                    color = game_grid[_x][_y] == 'w' ? 255.0 : 0.0;
                    alpha = 255.0;
                }
                GlobalVector2 global_vec = gridToGlobal(
                    GridVector2{.x = _x, .y = _y}
                );
                SDL_FRect osero{
                    .x = static_cast<float>(global_vec.x), 
                    .y = static_cast<float>(global_vec.y),
                    .w = static_cast<float>(LEN_PER_CELL), 
                    .h = static_cast<float>(LEN_PER_CELL)
                };
                SDL_SetRenderDrawColor(renderer, color, color, color, alpha);
                SDL_RenderFillRect(renderer, &osero);
            }
        }

        SDL_SetRenderDrawColor(renderer, 255.0, 0.0, 0.0, 255.0);
        const char* loc_mark = is_blacks_turn ? "B" : "W";
        GlobalVector2 current_on_global = gridToGlobal(current_on);
        SDL_RenderDebugText(
            renderer,
            static_cast<float>(current_on_global.x + (LEN_PER_CELL / 2)), 
            static_cast<float>(current_on_global.y + (LEN_PER_CELL / 2)), 
            loc_mark
        );

        bool still_have_empties{true};
        for (const std::array<char, 8>& col : game_grid){
            if (std::find(col.begin(), col.end(), 'n') != col.end()){
                still_have_empties = true;
                break;
            } else {
                still_have_empties = false;
            }
        }
        if (!still_have_empties){
            std::array<int, 2> stat{}; //0 b 1 w
            for (const std::array<char, 8>& col : game_grid){
                for (int _y = 0; _y < NUM; ++_y){
                    if (col[_y] == 'b') { ++stat[0]; } else { ++stat[1]; }
                }
            }
            if (stat[0] != stat[1]){
                for (std::array<char, 8>& col : game_grid){
                    for (int _y = 0; _y < NUM; ++_y){
                        col[_y] = stat[0] > stat[1] ? 'b' : 'w';
                    }
                }
            }
            is_game_ended = true;
        }

        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}