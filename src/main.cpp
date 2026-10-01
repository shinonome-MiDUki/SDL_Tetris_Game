#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <vector>
#include <cstdlib>
#include <random>

int main(int argc, char* argv[]){
    if (!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("SDL Loading failed");
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
        "MySdlGame", 800, 600, SDL_WINDOW_RESIZABLE,
        &window, &renderer
    )){
        SDL_Log("SDL Window creation failed");
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    Uint64 lastTicks = SDL_GetTicks();
    SDL_FRect diver = {380.0f, 500.0f, 40.0f, 40.0f};
    float speed = 260.0f;

    struct Rock
    {
        SDL_FRect rect;
        float speed;
        bool is_hit;
        bool is_posion;
    };
    std::vector<Rock> rocks;
    Uint64 lastSpawn = 0;
    int score = 100;

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

        const bool* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_LEFT]) {diver.x -= speed * dt;}
        if (keys[SDL_SCANCODE_RIGHT]) {diver.x += speed * dt;}
        if (keys[SDL_SCANCODE_UP]) {diver.y -= speed * dt;}
        if (keys[SDL_SCANCODE_DOWN]) {diver.y += speed * dt;}

        if (diver.x < 0) {diver.x = 0;}
        if (diver.x > 800 - diver.w) {diver.x = 800 - diver.w;}
        if (diver.y < 0) {diver.y = 0;}
        if (diver.y > 600 - diver.h) {diver.y = 600 - diver.h;}

        SDL_SetRenderDrawColor(renderer, 8, 28, 48, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 227, 162, 92, 255);
        SDL_RenderFillRect(renderer, &diver);

        std::random_device seed_gen;
        std::mt19937 engine(seed_gen());
        std::uniform_int_distribution<int> dist(0,2);
        if (nowTick - lastSpawn > 800){
            float x = static_cast<float>(std::rand() % (800 - 30));
            int rock_type = dist(engine);
            if (rock_type == 0 || rock_type == 1){
                rocks.push_back(Rock{
                    SDL_FRect{x, -30.0f, 30.0f, 30.0f}, 180.0f, false, false
                });
            } else{
                rocks.push_back(Rock{
                    SDL_FRect{x, -30.0f, 30.0f, 30.0f}, 180.0f, false, true
                });
            }
            lastSpawn = nowTick;
        }

        for (auto& rock : rocks){
            rock.rect.y += rock.speed * dt;
        }

        for (auto it = rocks.begin(); it != rocks.end();){
            if (SDL_HasRectIntersectionFloat(&diver, &it->rect) && !it->is_hit){
                if (it->is_posion){
                    score += 5;
                } else{
                    score -= 10;
                }
                it->is_hit = true;
                it = rocks.erase(it);
            }
            if (it->rect.y > 600){
                it = rocks.erase(it);
            } else{
                ++it;
            }
        }

        if (score <= 0){
            running = false;
        }

        SDL_SetRenderDrawColor(renderer, 120, 90, 70, 255);
        for (auto& rock : rocks){
            if (rock.is_posion) {continue;}
            SDL_RenderFillRect(renderer, &rock.rect);
        }
        SDL_SetRenderDrawColor(renderer, 200, 10, 30, 255);
        for (auto& rock : rocks){
            if (!rock.is_posion) {continue;}
            SDL_RenderFillRect(renderer, &rock.rect);
        }
        char scoreBuffer[64];
        SDL_snprintf(scoreBuffer, sizeof(scoreBuffer), "SCORE : %d", score);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 12.0f, 12.0f, scoreBuffer);
        
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