#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <vector>
#include <array>
#include <cstdlib>
#include <chrono>
#include <random>

#define SIDE_LENGTH 40

struct Vector2D
{
    float r;
    float c;
};

std::vector<Vector2D> rotate_vec2d(
    int rotation, const std::vector<Vector2D>& global_vec, const Vector2D& pivot_vec
){
    double cos_t = std::cos(rotation);
    double sin_t = std::sin(rotation);
    std::vector<Vector2D> rtn{};
    for (Vector2D v : global_vec){
        Vector2D local{.r = v.r - pivot_vec.r, .c = v.c - pivot_vec.c};
        Vector2D local_new{.r = local.r * cos_t - local.c * sin_t, .c = local.r * sin_t + local.c * cos_t};
        Vector2D displacement{.r = local_new.r - local.r, .c = local_new.c - local.c};
        Vector2D global_new{.r = v.r + displacement.r, .c = v.c + displacement.c};
        rtn.emplace_back(global_new);
    }
    return rtn;
}

Vector2D grid_to_global(Vector2D grid_pos){
    Vector2D global_coord{.r = grid_pos.r * SIDE_LENGTH, .c = grid_pos.c * SIDE_LENGTH};
    return global_coord;
}

Vector2D global_to_grid(Vector2D global_pos){
    Vector2D grid_coord{
        .r = static_cast<int>(global_pos.r / (float)SIDE_LENGTH), 
        .c = static_cast<int>(global_pos.c / (float)SIDE_LENGTH)
    };
    return grid_coord;
}

int main(int argc, char* argv[]){
    if (!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("SDL Loading failed");
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
        "MySdlGame", 800, 800, SDL_WINDOW_RESIZABLE,
        &window, &renderer
    )){
        SDL_Log("SDL Window creation failed");
        SDL_Quit();
        return 1;
    }

    bool running{true};
    SDL_Event event;

    Uint64 lastTicks = SDL_GetTicks();
    float speed = 260.0f;

    struct Block
    {
        SDL_FRect block;
        Vector2D pos;
    };

    std::array<std::array<bool, 20>, 20> game_grid = {{ false }};
    std::array<Block, 4> active_blocks{};
    bool can_spawn{true};
    

    while (running)
    {
        Uint64 nowTick = SDL_GetTicks();
        float dt = (nowTick - lastTicks) / 1000.0f;
        lastTicks = nowTick;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT){
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 8, 28, 48, 255);
        SDL_RenderClear(renderer);

        for (int c = 0; c < 20; c++){
            for (int r = 0; r < 20; r++){
                if (!game_grid[c][r]){ continue; }
                Vector2D grid_pos{.r = r, .c = c};
                Vector2D global_pos = grid_to_global(grid_pos);
                SDL_FRect new_block{global_pos.r, global_pos.c, SIDE_LENGTH, SIDE_LENGTH};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &new_block);
            }
        }

        
        if (can_spawn){
            active_blocks.fill(Block{});
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::mt19937 gen(seed);
            std::uniform_int_distribution<> init_pos_dist(0, 19);
            int init_pos_r = init_pos_dist(gen);
            Vector2D previous_pos{.r = init_pos_r, .c = 1};
            Vector2D new_pos{.r = init_pos_r, .c = 1};
            for (int i = 0; i < 4; i++){
                Vector2D global_pos = grid_to_global(new_pos);
                SDL_FRect new_block{global_pos.r, global_pos.c, SIDE_LENGTH, SIDE_LENGTH};
                Block new_block_block{
                    .block = new_block, .pos = new_pos
                };
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &new_block);
                active_blocks[i] = new_block_block;

                previous_pos = new_pos;
                std::uniform_int_distribution<> next_dir_dist(0, 3);
                bool next_dir_decided{false};
                while (!next_dir_decided){
                    int next_dir = next_dir_dist(gen); //0 right / 1 left / 2 up / 3 down
                    if (next_dir == 0){
                        int next_r = previous_pos.r + 1;
                        if (next_r <= 19) {
                            new_pos.r = next_r;
                            new_pos.c = previous_pos.c;
                        }
                        next_dir_decided = true;
                    } else if (next_dir == 1){
                        int next_r = previous_pos.r - 1;
                        if (next_r >= 0) {
                            new_pos.r = next_r;
                            new_pos.c = previous_pos.c;
                        }
                        next_dir_decided = true;
                    } else if (next_dir == 2){
                        int next_c = previous_pos.r - 1;
                        if (next_c >= 0) {
                            new_pos.r = previous_pos.r;
                            new_pos.c = next_c;
                        }
                        next_dir_decided = true;
                    } else if (next_dir == 3){
                        int next_c = previous_pos.r + 1;
                        if (next_c <= 19) {
                            new_pos.r = previous_pos.r;
                            new_pos.c = next_c;
                        }
                        next_dir_decided = true;
                    } else{
                        continue;
                    }
                }
            
            }
            can_spawn = false;
        }

        const bool* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_LEFT]) {}
        if (keys[SDL_SCANCODE_RIGHT]) {}
        if (keys[SDL_SCANCODE_UP]) {}
        if (keys[SDL_SCANCODE_DOWN]) {}


        // char scoreBuffer[64];
        // SDL_snprintf(scoreBuffer, sizeof(scoreBuffer), "SCORE : %d", score);
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_RenderDebugText(renderer, 12.0f, 12.0f, scoreBuffer);
        
        SDL_RenderPresent(renderer);
    }

    bool is_game_over_scene = true;
    while (is_game_over_scene){
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT){
                running = false;
                is_game_over_scene = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN){
                if (event.key.scancode == SDL_SCANCODE_SPACE){
                    is_game_over_scene = false;
                }
            }
            SDL_SetRenderDrawColor(renderer, 8, 28, 48, 255); 
            SDL_RenderClear(renderer);

            char scoreBuffer[64];
            SDL_snprintf(scoreBuffer, sizeof(scoreBuffer), "GAMEOVER");
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(renderer, 400.0f, 300.0f, scoreBuffer);
            
            SDL_RenderPresent(renderer);
        }
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}