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

#define SIDE_LENGTH 40

static const double pi = 3.141592653589793;

struct Vector2D
{
    float r;
    float c;

    bool operator==(const Vector2D& other) const {
        return r == other.r && c == other.c;
    }
};

std::vector<Vector2D> rotate_vec2d(
    double rotation, const std::vector<Vector2D>& vec_gp, const Vector2D& pivot_vec
){
    double cos_t = std::cos(rotation);
    double sin_t = std::sin(rotation);
    std::vector<Vector2D> rtn{};
    for (Vector2D v : vec_gp){
        Vector2D local{.r = v.r - pivot_vec.r, .c = v.c - pivot_vec.c};
        Vector2D local_new{
            .r = static_cast<float>(local.r * cos_t - local.c * sin_t), 
            .c = static_cast<float>(local.r * sin_t + local.c * cos_t)
        };
        Vector2D displacement{.r = local_new.r - local.r, .c = local_new.c - local.c};
        Vector2D global_new{.r = v.r + displacement.r, .c = v.c + displacement.c};
        rtn.emplace_back(global_new);
    }
    return rtn;
}

Vector2D grid_to_global(Vector2D grid_pos){
    Vector2D global_coord{.r = grid_pos.r * SIDE_LENGTH + 5.0f, .c = grid_pos.c * SIDE_LENGTH};
    return global_coord;
}

Vector2D global_to_grid(Vector2D global_pos){
    Vector2D grid_coord{
        .r = static_cast<float>(global_pos.r / (float)SIDE_LENGTH), 
        .c = static_cast<float>(global_pos.c / (float)SIDE_LENGTH)
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
        "Trtris", 800, 800, SDL_WINDOW_RESIZABLE,
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
    std::array<bool, 2> can_move{true, true}; // {can_move_left, can_move_right}
    float previous_moved_tick{0.0f};
    

    while (running)
    {
        Uint64 nowTick = SDL_GetTicks();
        float dt = (nowTick - lastTicks) / 1000.0f;
        lastTicks = nowTick;
        previous_moved_tick += dt;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT){
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN){
                if (!active_blocks.empty())
                {
                    if(event.key.scancode == SDL_SCANCODE_RIGHT && can_move[1]){
                        for (Block& b : active_blocks){
                            SDL_FRect block = b.block;
                            float new_grid_r_pos = b.pos.r + 1;
                            Vector2D new_grid_pos{.r = new_grid_r_pos, .c = b.pos.c};
                            if ((int)new_grid_r_pos <= 19){
                                Vector2D new_global_pos = grid_to_global(new_grid_pos);
                                b.block.x = new_global_pos.r;
                                b.block.y = new_global_pos.c;
                                b.pos.r = new_grid_r_pos;
                            }
                        }
                    } else if(event.key.scancode == SDL_SCANCODE_LEFT && can_move[0]){
                        for (Block& b : active_blocks){
                            SDL_FRect block = b.block;
                            float new_grid_r_pos = b.pos.r - 1;
                            Vector2D new_grid_pos{.r = new_grid_r_pos, .c = b.pos.c};
                            if ((int)new_grid_r_pos >= 0){
                                Vector2D new_global_pos = grid_to_global(new_grid_pos);
                                b.block.x = new_global_pos.r;
                                b.block.y = new_global_pos.c;
                                b.pos.r = new_grid_r_pos;
                            }
                        }
                    }  else if(event.key.scancode == SDL_SCANCODE_SPACE){
                        Vector2D pivot_point = active_blocks[1].pos;
                        std::vector<Vector2D> active_blocks_vecs = {
                            Vector2D{.r = active_blocks[0].pos.r, .c = active_blocks[0].pos.c},
                            Vector2D{.r = active_blocks[1].pos.r, .c = active_blocks[1].pos.c},
                            Vector2D{.r = active_blocks[2].pos.r, .c = active_blocks[2].pos.c},
                            Vector2D{.r = active_blocks[3].pos.r, .c = active_blocks[3].pos.c}
                        };
                        std::vector<Vector2D> rotated_active_blocks_vecs = rotate_vec2d(
                            pi / 2, active_blocks_vecs, pivot_point
                        );

                        bool is_rotatable{true};
                        for (int i = 0; i < 4; i++){
                            if (rotated_active_blocks_vecs[i].r > 19
                                || rotated_active_blocks_vecs[i].r < 0
                                || rotated_active_blocks_vecs[i].c > 19
                                || rotated_active_blocks_vecs[i].c < 0
                                || game_grid[rotated_active_blocks_vecs[i].c][rotated_active_blocks_vecs[i].r]
                            ) { is_rotatable = false; }
                        }
                        if (is_rotatable){
                            for (int i = 0; i < 4; i++){
                                Block& b = active_blocks[i];
                                SDL_FRect block = b.block;
                                Vector2D new_grid_pos{
                                    .r = rotated_active_blocks_vecs[i].r, 
                                    .c = rotated_active_blocks_vecs[i].c
                                };
                                Vector2D new_global_pos = grid_to_global(new_grid_pos);
                                b.block.x = new_global_pos.r;
                                b.block.y = new_global_pos.c;
                                b.pos.r = new_grid_pos.r;
                                b.pos.c = new_grid_pos.c;
                            }
                        }
                    } else if(event.key.scancode == SDL_SCANCODE_RETURN){
                        std::array<std::array<int, 2>, 4> active_blocks_local{{ 0 }};

                        int zero_standard{0};
                        for (int i = 0; i < 4; i++){
                            Block b = active_blocks[i];
                            if (i == 0){
                                zero_standard = (int)b.pos.c;
                            }
                            active_blocks_local[i][0] = (int)b.pos.r;
                            active_blocks_local[i][1] = (int)b.pos.c - zero_standard;
                        }

                        int base_line{19};
                        for (int c = 0; c <= 19; c++){
                            bool is_decided{false};
                            std::vector<bool> check_list{};
                            for (std::array<int, 2> check : active_blocks_local){
                                if (c + check[1] < 0){
                                    break;
                                }
                                if (c + check[1] > 19){
                                    base_line = c - 1;
                                    is_decided = true;
                                    break;
                                }
                                check_list.emplace_back(game_grid[c + check[1]][check[0]]);
                                if (std::find(check_list.begin(), check_list.end(), true) != check_list.end()){
                                    base_line = c - 1;
                                    is_decided = true;
                                }
                            }
                            if (is_decided) { break; }
                        }
                        std::cout << "-----" << "\n";
                        for (int i = 0; i < 4; i++){
                            std::cout << active_blocks_local[i][0] << " & " << active_blocks_local[i][1] << "\n";
                        }
                        std::cout << base_line << "\n";
                        std::cout << "-----" << "\n";

                        for (int i = 0; i < 4; i++){
                            Block& b = active_blocks[i];
                            SDL_FRect block = b.block;
                            Vector2D new_grid_pos{
                                .r = static_cast<float>(active_blocks_local[i][0]), 
                                .c = static_cast<float>(base_line + active_blocks_local[i][1])
                            };
                            Vector2D new_global_pos = grid_to_global(new_grid_pos);
                            b.block.x = new_global_pos.r;
                            b.block.y = new_global_pos.c;
                            b.pos.r = new_grid_pos.r;
                            b.pos.c = new_grid_pos.c;
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 8, 28, 48, 255);
        SDL_RenderClear(renderer);

        for (int c = 0; c < 20; c++){
            for (int r = 0; r < 20; r++){
                if (!game_grid[c][r]){ continue; }
                Vector2D grid_pos{.r = static_cast<float>(r), .c = static_cast<float>(c)};
                Vector2D global_pos = grid_to_global(grid_pos);
                SDL_FRect new_block{global_pos.r, global_pos.c, 30, 30};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &new_block);
            }
        }

        for (Block& b : active_blocks){
            if (b.pos.c == 19
                || game_grid[(int)b.pos.c + 1][(int)b.pos.r]
            ){
                can_spawn = true;
                for (Block& b : active_blocks){
                    game_grid[(int)b.pos.c][(int)b.pos.r] = true;
                }
                break;
            }
        }

        
        if (can_spawn) {
            active_blocks.fill(Block{});
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::mt19937 gen(seed);
            std::uniform_int_distribution<> init_pos_dist(2, 17); 
            int init_pos_r = init_pos_dist(gen);

            Vector2D new_pos{.r = static_cast<float>(init_pos_r), .c = 1.0f};
            std::vector<Vector2D> occupied{};

            for (int i = 0; i < 4; i++) {
                Vector2D global_pos = grid_to_global(new_pos);
                active_blocks[i] = Block{.block = {global_pos.r, global_pos.c, 30, 30}, .pos = new_pos};
                occupied.push_back(new_pos);

                std::uniform_int_distribution<> next_dir_dist(0, 3);
                bool next_dir_decided = false;

                for (int count = 0; count < 50; ++count) {
                    int next_dir = next_dir_dist(gen);
                    int r_inc = (next_dir < 2) ? (next_dir == 0 ? 1 : -1) : 0;
                    int c_inc = (next_dir >= 2) ? (next_dir == 2 ? 1 : -1) : 0;

                    int next_r = new_pos.r + r_inc;
                    int next_c = new_pos.c + c_inc;

                    if (next_r >= 0 && next_r < 20 && next_c >= 0 && next_c < 20) {
                        Vector2D target{static_cast<float>(next_r), static_cast<float>(next_c)};
                        if (std::find(occupied.begin(), occupied.end(), target) == occupied.end() && !game_grid[next_c][next_r]) {
                            new_pos = target;
                            next_dir_decided = true;
                            break;
                        }
                    }
                }

                if (!next_dir_decided) {
                    new_pos.r += 1.0f;  
                }
            }
            can_spawn = false;
        }
        else {
            for (Block& b : active_blocks){
                SDL_FRect block = b.block;
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &block);
            }
        }

        for (Block& b : active_blocks){
            if (b.pos.r == 0){
                can_move[0] = false;
                break;
            } else if (b.pos.r == 19){
                can_move[1] = false;
                break;
            } else {
                can_move[0] = true;
                can_move[1] = true;
            }
        }

        if (std::find(game_grid[0].begin(), game_grid[0].end(), true) != game_grid[0].end()){
            running = false;
            SDL_RenderPresent(renderer);
            break;
        }

        if (previous_moved_tick > 0.85f)
        {
            for (Block& b : active_blocks){
                SDL_FRect block = b.block;
                float new_grid_c_pos = b.pos.c + 1;
                Vector2D new_grid_pos{.r = b.pos.r, .c = new_grid_c_pos};
                if ((int)new_grid_c_pos <= 19){
                    Vector2D new_global_pos = grid_to_global(new_grid_pos);
                    b.block.x = new_global_pos.r;
                    b.block.y = new_global_pos.c;
                    b.pos.c = new_grid_c_pos;
                }
            }
            previous_moved_tick = 0.0f;
        }

        int upper_most_full_line{20};
        int lower_most_full_line{20};
        bool is_full_line_detected{false};
        for (int c = 19; c >= 0; c--){
            if (std::find(game_grid[c].begin(), game_grid[c].end(), false) == game_grid[c].end()){
                if (!is_full_line_detected){
                    lower_most_full_line = c;
                };
                is_full_line_detected = true;
            } else if (is_full_line_detected){ 
                upper_most_full_line = c + 1;
                break; 
            }
        }
        if (upper_most_full_line < 20){
            int consective_full_lines_count = lower_most_full_line - upper_most_full_line + 1;
            for (int c = lower_most_full_line; c >= 0; c--){
                if (c - consective_full_lines_count >= 0){
                    game_grid[c] = game_grid[c - consective_full_lines_count];
                } else{
                    game_grid[c].fill(false);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 180); 
        for (int c = 0; c < 20; c++) {
            std::vector<int> drawn_r{};
            for (Block& b : active_blocks){
                int r = (int)b.pos.r;
                if (std::find(drawn_r.begin(), drawn_r.end(), r) != drawn_r.end()){
                    continue;
                }
                if (c < (int)b.pos.c) { continue; }
                drawn_r.emplace_back(r);
                Vector2D global_pos = grid_to_global(Vector2D{.r = static_cast<float>(r), .c = static_cast<float>(c)});
                const char* state_str = "|";
                SDL_RenderDebugText(renderer, global_pos.r + 10.0f, global_pos.c + 10.0f, state_str);
            }
        }

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