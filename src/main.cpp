#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

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

static constexpr double SCREEN_LEN = 800.0;

static constexpr double pi = 3.141592653589793;

static constexpr double LEN_PER_GRID = SCREEN_LEN / NUM;

static constexpr double LEN_PER_CELL = LEN_PER_GRID * 0.75;

static constexpr double OFFSET = (LEN_PER_GRID - LEN_PER_CELL) / 2;



struct GlobalVector2{
    double x; double y;
    
    bool operator==(const GlobalVector2& other) const {
        return x == other.x && y == other.y;
    }
};

struct GridVector2{
    int x; int y;

    bool operator==(const GridVector2& other) const {
        return x == other.x && y == other.y;
    }
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

std::vector<GridVector2> FilterSelectables(
    char myself,
    const std::array<std::array<char, 8>, 8>& game_grid
){
    std::vector<GridVector2> rtn{};
    char her = myself == 'b' ? 'w' : 'b';
    for (int _x = 0; _x < NUM; ++_x){
        for (int _y = 0; _y < NUM; ++_y){
            std::array<int, 3> offsets{-1, 0, 1};
            for (int offset_x : offsets){
                for (int offset_y : offsets){
                    if (_x + offset_x < 0
                        || _x + offset_x >= NUM
                        || _y + offset_y < 0
                        || _y + offset_y >= NUM
                        || game_grid[_x + offset_x][_y + offset_y] != her
                    ){ continue; }
                    GridVector2 to_append{
                        .x = _x + offset_x,
                        .y = _y + offset_y
                    };
                    if (std::find(rtn.begin(), rtn.end(), to_append) == rtn.end()){
                        rtn.emplace_back(to_append);
                    }
                }
            }
        }
    }
    return rtn;
}


std::vector<GridVector2> ScanSurroundings(
    GridVector2 places_vec, 
    char myself, 
    const std::array<std::array<char, 8>, 8>& game_grid
){
    int placed_x = places_vec.x;
    int placed_y = places_vec.y;
    char her = myself == 'b' ? 'w' : 'b';
    std::vector<GridVector2> buf{};
    std::array<int, 3> offsets{-1, 0, 1};

    for (int offset_x : offsets){
        for (int offset_y : offsets){
            if (placed_x + offset_x < 0
                || placed_x + offset_x >= NUM
                || placed_y + offset_y < 0
                || placed_y + offset_y >= NUM
                || (offset_x == 0 && offset_y == 0)
                || game_grid[placed_x + offset_x][placed_y + offset_y] != her
            ){ continue; }

            int buffering_count = 0;
            for (int i = 1; i < NUM; ++i){
                if (placed_x + (offset_x * i) < 0
                    || placed_x + (offset_x * i) >= NUM
                    || placed_y + (offset_y * i) < 0
                    || placed_y + (offset_y * i) >= NUM
                ){ 
                    for (int _ = 0; _ < buffering_count; ++_){
                        buf.pop_back();
                    }
                    break; 
                }
                char checking = game_grid[placed_x + (offset_x * i)][placed_y + (offset_y * i)];
                if (checking == her){
                    buf.emplace_back(
                        GridVector2{
                            .x = placed_x + (offset_x * i),
                            .y = placed_y + (offset_y * i)
                        }
                    );
                    ++buffering_count;
                } else if (checking == myself){
                    break;
                } else{
                    for (int _ = 0; _ < buffering_count; ++_){
                        buf.pop_back();
                    }
                    break;
                }
            }
        }
    }
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
        "Othello", SCREEN_LEN, SCREEN_LEN, SDL_WINDOW_RESIZABLE,
        &window, &renderer
    )){
        SDL_Log("SDL Window creation failed");
        SDL_Quit();
        return 1;
    }

    SDL_Texture* suisei_bg_tex = IMG_LoadTexture(renderer, "../suisei_bg.png");
    if (!suisei_bg_tex) {
        std::cerr << "Suisei BG img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* azki_bg_tex = IMG_LoadTexture(renderer, "../azki_bg.png");
    if (!azki_bg_tex) {
        std::cerr << "AZKi BG img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* suisei_tex = IMG_LoadTexture(renderer, "../suisei.png");
    if (!suisei_tex) {
        std::cerr << "Suisei img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* azki_tex = IMG_LoadTexture(renderer, "../azki.png");
    if (!azki_tex) {
        std::cerr << "AZKi img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* nil_tex = IMG_LoadTexture(renderer, "../nil.png");
    if (!nil_tex) {
        std::cerr << "Nil img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* selecting_tex = IMG_LoadTexture(renderer, "../selecting.png");
    if (!selecting_tex) {
        std::cerr << "Selecting img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* selecting_unable_tex = IMG_LoadTexture(renderer, "../selecting_unable.png");
    if (!selecting_unable_tex) {
        std::cerr << "SelectingUnable img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* selectable_tex = IMG_LoadTexture(renderer, "../selectable.png");
    if (!selectable_tex) {
        std::cerr << "Selectable img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    //white suisei  black azki


    bool running{true};
    SDL_Event event;
    
    std::array<std::array<char, 8>, 8> game_grid{{}}; // n = nothing w = white b = black
    for (std::array<char, 8>& k : game_grid){ k.fill('n'); }
    game_grid[3][3] = 'w';
    game_grid[4][4] = 'w';
    game_grid[3][4] = 'b';
    game_grid[4][3] = 'b';

    std::array<std::array<SDL_FRect, 8>, 8> game_frects{{}};
    for (std::array<SDL_FRect, 8>& k : game_frects){ 
        k.fill(
            SDL_FRect{
                50.0f, 50.0f, 100.0f, 100.0f
            }
        ); 
    }

    bool is_blacks_turn{true};
    bool is_game_ended{false};
    bool do_show_hints{false};
    GridVector2 current_on{GridVector2{.x = 5, .y = 4}};
    std::vector<GridVector2> selectable_cells{
        GridVector2{.x = 5, .y = 4},
        GridVector2{.x = 4, .y = 5},
        GridVector2{.x = 2, .y = 3},
        GridVector2{.x = 3, .y = 2}
    };
    std::vector<GridVector2> turnables_buf;
    char myself = is_blacks_turn ? 'b' : 'w';
    game_grid[current_on.x][current_on.y] = myself;
    turnables_buf = ScanSurroundings(current_on, myself, game_grid);
    game_grid[current_on.x][current_on.y] = 'n';

    while (running)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) { 
                running = false;
            } 
            if (event.type == SDL_EVENT_KEY_DOWN){
                if (event.key.scancode == SDL_SCANCODE_LEFT 
                    || event.key.scancode == SDL_SCANCODE_RIGHT 
                    || event.key.scancode == SDL_SCANCODE_UP
                    || event.key.scancode == SDL_SCANCODE_DOWN
                ){
                    if (event.key.scancode == SDL_SCANCODE_LEFT){
                        if (current_on.x - 1 >= 0){ -- current_on.x; }
                    } else if (event.key.scancode == SDL_SCANCODE_RIGHT){
                        if (current_on.x + 1 <= 7){ ++ current_on.x; }
                    } else if (event.key.scancode == SDL_SCANCODE_UP){
                    if (current_on.y - 1 >= 0){ -- current_on.y; }
                    } else if (event.key.scancode == SDL_SCANCODE_DOWN){
                        if (current_on.y + 1 <= 7){ ++ current_on.y; }
                    }
                    if (game_grid[current_on.x][current_on.y] != 'n'){ 
                        turnables_buf.clear();
                    } else {
                        char myself = is_blacks_turn ? 'b' : 'w';
                        turnables_buf = ScanSurroundings(current_on, myself, game_grid);
                    }
                } 
                else if (event.key.scancode == SDL_SCANCODE_RETURN){
                    if (turnables_buf.empty()){ break; }
                    char myself = is_blacks_turn ? 'b' : 'w';
                    game_grid[current_on.x][current_on.y] = myself;
                    for (GridVector2 turnable : turnables_buf){ game_grid[turnable.x][turnable.y] = myself; }
                    turnables_buf.clear();

                    do_show_hints = false;
                    selectable_cells.clear();
                    myself = !is_blacks_turn ? 'b' : 'w';
                    for (int _x = 0; _x < NUM; ++_x){
                        for (int _y = 0; _y < NUM; ++_y){
                            if (game_grid[_x][_y] != 'n') { continue; }
                            std::vector<GridVector2> _turnables = ScanSurroundings(GridVector2{.x = _x, .y = _y}, myself, game_grid);
                            if (!_turnables.empty()){
                                selectable_cells.emplace_back(GridVector2{.x = _x, .y = _y});
                            }
                        }
                    }
                    if (!selectable_cells.empty()){
                        is_blacks_turn = !is_blacks_turn;
                    } else{
                        selectable_cells.clear();
                        myself = is_blacks_turn ? 'b' : 'w';
                        for (int _x = 0; _x < NUM; ++_x){
                            for (int _y = 0; _y < NUM; ++_y){
                                if (game_grid[_x][_y] != 'n') { continue; }
                                std::vector<GridVector2> _turnables = ScanSurroundings(GridVector2{.x = _x, .y = _y}, myself, game_grid);
                                if (!_turnables.empty()){
                                    selectable_cells.emplace_back(GridVector2{.x = _x, .y = _y});
                                }
                            }
                        }
                        if (!selectable_cells.empty()){
                            is_blacks_turn = is_blacks_turn;
                        } else {
                            is_game_ended = true;
                            break;
                        }
                    }

                } 
                else if (event.key.scancode == SDL_SCANCODE_SPACE){
                    if (is_game_ended) {
                        running = false;
                        break;
                    }
                }
                else if (event.key.scancode == SDL_SCANCODE_H){
                    do_show_hints = !do_show_hints;
                }
            }
        }
        if (!running) { break; }

        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255.0, 255.0, 255.0, 255.0);
        SDL_RenderTexture(
            renderer, 
            is_blacks_turn ? azki_bg_tex : suisei_bg_tex, 
            nullptr, 
            nullptr
        );
        
        for (int _x = 0; _x < NUM; ++_x){
            for (int _y = 0; _y < NUM; ++_y){
                SDL_Texture* texture_to_use;
                if (game_grid[_x][_y] == 'b'){
                    texture_to_use = azki_tex;
                } else if (game_grid[_x][_y] == 'w'){
                    texture_to_use = suisei_tex;
                } else {
                    if (do_show_hints
                        && std::find(selectable_cells.begin(), selectable_cells.end(), GridVector2{.x = _x, .y = _y}) != selectable_cells.end()
                    ){
                        texture_to_use = selectable_tex;
                    } else{
                        texture_to_use = nil_tex;
                    }
                }

                GlobalVector2 global_vec = gridToGlobal(
                    GridVector2{.x = _x, .y = _y}
                );
                game_frects[_x][_y].h = static_cast<float>(LEN_PER_CELL);
                game_frects[_x][_y].w = static_cast<float>(LEN_PER_CELL);
                game_frects[_x][_y].x = static_cast<float>(global_vec.x);
                game_frects[_x][_y].y = static_cast<float>(global_vec.y);
                SDL_RenderTexture(
                    renderer,
                    texture_to_use,
                    nullptr,
                    &game_frects[_x][_y]
                );
            }
        }
        GlobalVector2 global_vec_selecting = gridToGlobal(current_on);
        SDL_FRect selecting_overlay{
            .h = static_cast<float>(LEN_PER_CELL),
            .w = static_cast<float>(LEN_PER_CELL),
            .x = static_cast<float>(global_vec_selecting.x),
            .y = static_cast<float>(global_vec_selecting.y)
        };
        SDL_RenderTexture(
            renderer,
            !turnables_buf.empty() ? selecting_tex : selecting_unable_tex,
            nullptr,
            &selecting_overlay
        );

        // SDL_SetRenderDrawColor(renderer, 255.0, 0.0, 0.0, 255.0);
        // const char* loc_mark = is_blacks_turn ? "B" : "W";
        // GlobalVector2 current_on_global = gridToGlobal(current_on);
        // SDL_RenderDebugText(
        //     renderer,
        //     static_cast<float>(current_on_global.x + (LEN_PER_CELL / 2)), 
        //     static_cast<float>(current_on_global.y + (LEN_PER_CELL / 2)), 
        //     loc_mark
        // );

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