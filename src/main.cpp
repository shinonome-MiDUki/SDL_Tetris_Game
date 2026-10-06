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
#include <optional>

static constexpr int SCREEN_LEN = 980;
static constexpr int PLAYER_OBJ_W = 120;
static constexpr int PLAYER_OBJ_H = 20;
static constexpr int BALL_R = 20;
static constexpr double kPI = 3.1415926535;

struct Ball{
    SDL_FRect ball_frect;
    double center_x;
    double center_y;
    double v_mag;
    float v_ang;
    bool is_alive;
};

struct Collider{
    float a;
    float b;
    float c;

    float uponCollision(float colliding_angle){
        float my_norm_rad = std::atan2(-a, b);
        return 2 * my_norm_rad - colliding_angle - kPI;
    }

    bool operator==(const Collider& other) const {
        return b == other.a && b == other.b && c == other.c;
    }
};

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

    SDL_Texture* ball_tex = IMG_LoadTexture(renderer, "../ball.png");
    if (!ball_tex) {
        std::cerr << "Selectable img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Texture* bg_tex = IMG_LoadTexture(renderer, "../bg.png");
    if (!bg_tex) {
        std::cerr << "Selectable img loading failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_FRect player_obj = SDL_FRect{
        .h = PLAYER_OBJ_H, .w = PLAYER_OBJ_W, 
        .x = static_cast<int>((SCREEN_LEN / 2) - (PLAYER_OBJ_W / 2)), 
        .y = static_cast<int>(SCREEN_LEN * 0.9)
    };
    Ball ball = Ball{
        .ball_frect = SDL_FRect{
            .h = BALL_R * 2, .w = BALL_R * 2,
            .x = static_cast<int>(SCREEN_LEN / 2) - BALL_R, .y = static_cast<int>((SCREEN_LEN * 0.9) - (BALL_R * 2))
        },
        .center_x = static_cast<double>(SCREEN_LEN / 2),
        .center_y = static_cast<double>((SCREEN_LEN * 0.9)),
        .v_mag = 800.0,
        .v_ang = static_cast<float>(kPI / 4),
        .is_alive = true
    };
    std::vector<Collider> screen_boundaries{
        Collider{.a = 1.0, .b = 0.0, .c = 0.0},
        Collider{.a = 0.0, .b = 1.0, .c = 0.0},
        Collider{.a = 1.0, .b = 0.0, .c = -SCREEN_LEN},
        Collider{.a = 0.0, .b = 1.0, .c = -SCREEN_LEN}
    };
    Collider custom_collider = Collider{.a = 1.0, .b = 0.0, .c = 0.0};

    bool running{true};
    SDL_Event event;

    Uint64 last_tick = 0;
    bool is_drawing_line{false};
    float line_start_x{0.0f};
    float line_start_y{0.0f};


    while (running)
    {
        Uint64 current_take = SDL_GetTicks();
        float dt = (current_take - last_tick) / 1000.0;
        last_tick = current_take;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) { 
                running = false;
            } else if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                line_start_x = event.motion.x;
                line_start_y = event.motion.y;
                is_drawing_line = true;
            } else if(event.type == SDL_EVENT_MOUSE_BUTTON_UP){
                is_drawing_line = false;
            } else if(event.type == SDL_EVENT_MOUSE_MOTION){
                if (!is_drawing_line) { break; }
                float current_pos_x = event.motion.x;
                float current_pos_y = event.motion.y;
                Collider new_collider = Collider{
                    .a = current_pos_y - line_start_y,
                    .b = line_start_x - current_pos_x,
                    .c = ((current_pos_y - line_start_y) * line_start_x) + ((line_start_x - current_pos_x) * line_start_y)
                };
                custom_collider = new_collider;
            }
        }

    
        const bool* keys = SDL_GetKeyboardState(nullptr);

        if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) { 
            if (player_obj.x - 2 < 0){
                player_obj.x = 0;
            } else {
                player_obj.x -= 2;
            }
        }
        if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
            if (player_obj.x + PLAYER_OBJ_W + 2 > SCREEN_LEN){
                player_obj.x = SCREEN_LEN - PLAYER_OBJ_W;
            } else {
                player_obj.x += 2;
            }
        }

        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255.0, 255.0, 255.0, 255.0);
        SDL_RenderTexture(
            renderer, bg_tex, nullptr, nullptr
        );

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &player_obj);

        if (ball.is_alive){
            ball.ball_frect.y -= ball.v_mag * std::cos(ball.v_ang) * dt; 
            ball.ball_frect.x += ball.v_mag * std::sin(ball.v_ang) * dt; 
            SDL_RenderTexture(
                renderer,
                ball_tex,
                nullptr,
                &ball.ball_frect
            );
            ball.center_x = static_cast<double>(ball.ball_frect.x + (BALL_R / 2));
            ball.center_y = static_cast<double>(ball.ball_frect.y + (BALL_R / 2));
        }

        // float x_intercept = static_cast<float>((0 - custom_collider.c) / custom_collider.a);
        // float y_intercept = static_cast<float>((0 - custom_collider.c) / custom_collider.b);
        // SDL_SetRenderDrawColor(renderer, 255.0, 255.0, 255.0, 255.0);
        // SDL_RenderLine(renderer, x_intercept, 0.0f, 0.0f, y_intercept);

        for (Collider collider : screen_boundaries){
            double d = std::abs(collider.a * ball.center_x + collider.b * ball.center_y + collider.c) 
                / std::sqrt((collider.a * collider.a) + (collider.b * collider.b));
            if (d <= BALL_R){
                ball.v_ang = collider.uponCollision(ball.v_ang);
                break;
            }
        }

        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyTexture(bg_tex);
    SDL_DestroyTexture(ball_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}