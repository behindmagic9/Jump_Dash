#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <ctime>

enum GameStates
{
    START,    
    PLAYING,  
    GAMEOVER, 
};


const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int TILE_WIDTH = 200;
const int TILE_GAP = 180;
const int PASSAGE_GAP = 250;
int score = 0;

int getRandomHeight()
{
    return (rand() % 200) + 100;
}

struct Car
{

    Car(SDL_Renderer *render)
        : renderer(render), time(0.0f), amplitude(100.0f), frequency(0.1f), speed(2.0f),
          x(0), y(SCREEN_HEIGHT / 2), isJumping(false), jumpVelocity(0.0f),
          gravity(0.5f), rotation(0.0f), texture(nullptr)
    {
        car_rect = {static_cast<int>(x), static_cast<int>(y), 80, 80}; // Set default size
    }

    void reset()
    {
        x = 0;                 
        y = SCREEN_HEIGHT / 2; 

    }

    void Load(const std::string &path)
    {
        SDL_Surface *car_surface = SDL_LoadBMP(path.c_str());
        if(!car_surface){
            std::cout << "surface not loaded" << std::endl;
        }
        SDL_SetColorKey(car_surface, SDL_TRUE, SDL_MapRGB(car_surface->format, 0x00, 0x00, 0x00));
        texture = SDL_CreateTextureFromSurface(renderer, car_surface);
        if (!texture)
        {
            std::cout << "texture not laoded" << std::endl;
        }
        SDL_FreeSurface(car_surface);
    }

    void UpdateY(int y_cord)
    {
        car_rect.y += y_cord;
    }

    void Update()
    {
        if (isJumping)
        {
            y += jumpVelocity;
            jumpVelocity += gravity;

            rotation = jumpVelocity * 2;

            if (y >= 620)
            {
                y = 620;
                isJumping = false;
                rotation = 0;
            }
        }
        else
        {
            rotation = 0;
        }

        
        x += speed;

        if (y <= 1)
        {
            y = 1;
        }

        if (x > 320)
        {
            x = 320; // fixeing in one postion
        }

        car_rect.x = static_cast<int>(x);
        car_rect.y = static_cast<int>(y);
    }

    void Jump()
    {
        isJumping = true;
        jumpVelocity = -7.0f; 
    }

    SDL_Rect getRect()
    {
        return car_rect;
    }

    void Render()
    {
        SDL_RenderCopyEx(renderer, texture, nullptr, &car_rect, rotation, nullptr, SDL_FLIP_NONE);
    }

    SDL_Rect car_rect;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    bool isJumping;
    float jumpVelocity;

    float time;

private:
    const float amplitude;
    const float frequency;
    const float speed;
    float x;
    float y;
    const float gravity;
    float rotation;
};

struct Tiles
{
    Tiles(SDL_Renderer *render, int x, int y, int h, bool isTopTile)
        : texture(nullptr), renderer(render), time(0.0f), baseY(y), height(h), isTop(isTopTile)
    {
        rect = {x, y, TILE_WIDTH, h};
        baseHeight = h;
    }

    void Load(std::string path)
    {
        SDL_Surface *tile_surface = SDL_LoadBMP(path.c_str());
        texture = SDL_CreateTextureFromSurface(renderer, tile_surface);
        if (!texture)
        {
            std::cout << "texture not laoded" << std::endl;
        }
        SDL_FreeSurface(tile_surface);
    }

    void updatePosition(int offsetX)
    {
        rect.x -= offsetX;
        x = rect.x;
    }

    void resetTime()
    {
        time = 0;
    }

    void updateVerticalPosition(float deltaTime)
    {
        if (deltaTime > 2000)
        {                          
            float speed = 0.1f;      
            float amplitude = 100.0f; 

            float sineOffset = amplitude * sin(time * speed);

            time += 0.1f;

            // another on
            if (isTop)
            {
                rect.h = baseHeight + sineOffset; 
            }
            else
            {
                rect.y = baseY + sineOffset;      
                rect.h = baseHeight - sineOffset; 
            }
        }
    }

    void Render()
    {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    }

    SDL_Rect getRect()
    {
        return rect;
    }

    bool Collide(SDL_Rect rect1)
    {
        return SDL_HasIntersection(&rect, &rect1);
    }

    int x;
    int height;
    int baseY;  
    bool isTop;
    float time; 
    int baseHeight;

    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Rect rect;
};

void RenderText(SDL_Renderer *renderer, const std::string &message, int x, int y, TTF_Font *font)
{
    SDL_Color color = {0x5A, 0xb7, 0x87}; // White color // 5ab787
    SDL_Surface *surfaceMessage = TTF_RenderText_Solid(font, message.c_str(), color);
    SDL_Texture *messageTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect messageRect;
    messageRect.x = x;
    messageRect.y = y;
    messageRect.w = surfaceMessage->w;
    messageRect.h = surfaceMessage->h;

    SDL_RenderCopy(renderer, messageTexture, nullptr, &messageRect);

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(messageTexture);
}

void populateTiles(SDL_Renderer *renderer, std::vector<Tiles> &tiles_vector_top, std::vector<Tiles> &tiles_vector_bottom)
{
    tiles_vector_top.clear();
    tiles_vector_bottom.clear();

    int startX = 340; 
    for (int i = 0; i < 10; i++)
    {
        int gapY = getRandomHeight();
        int bottom_tile_height = SCREEN_HEIGHT - gapY - PASSAGE_GAP;
        int top_tile_height = gapY;

        //  bottom tiles
        Tiles bottom_tile(renderer, startX + i * (TILE_WIDTH + TILE_GAP), SCREEN_HEIGHT - bottom_tile_height, bottom_tile_height, false);
        bottom_tile.Load("images/wall1.bmp");
        bottom_tile.resetTime();
        tiles_vector_bottom.push_back(bottom_tile);

        //  top tiles
        Tiles top_tile(renderer, startX + i * (TILE_WIDTH + TILE_GAP), 0, top_tile_height, true);
        top_tile.Load("images/wall1.bmp");
        top_tile.resetTime();
        tiles_vector_top.push_back(top_tile);
    }
}

int main(int argc, char *argv[])
{

    srand(static_cast<unsigned>(time(0)));

    SDL_Init(SDL_INIT_VIDEO);
    if (TTF_Init() == -1)
    {
        std::cout << "Failed to initialize TTF: " << TTF_GetError() << std::endl;
        return -1;
    }

    TTF_Font *font = TTF_OpenFont("Fonts/LIVINGBY.TTF", 70);
    if (font == nullptr)
    {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
    }

    SDL_Window *window = SDL_CreateWindow("Wave Movement", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    GameStates gameState = START;

    Car car(renderer);
    car.Load("images/car.bmp");

    SDL_Surface *snow_surface = SDL_LoadBMP("images/snow2.bmp");
    SDL_SetColorKey(snow_surface, SDL_TRUE, SDL_MapRGB(snow_surface->format, 0x5A, 0xb7, 0x87)); // // 5ab787
    SDL_Texture *snow1 = SDL_CreateTextureFromSurface(renderer, snow_surface);
    SDL_FreeSurface(snow_surface);
    SDL_Rect snow_rect1 = {0, 0, 1280, 720};
    SDL_Rect snow_rect2 = {1280, 0, 1280, 720};

    SDL_Surface* background_surface = SDL_LoadBMP("images/background1.bmp");
    SDL_Texture* background = SDL_CreateTextureFromSurface(renderer, background_surface);
    SDL_FreeSurface(background_surface);   
    SDL_Rect background1 = {0,0,1280,720};


    SDL_Surface* start_surface = SDL_LoadBMP("images/startScreen.bmp");
    SDL_Texture* start_Screen = SDL_CreateTextureFromSurface(renderer, start_surface);
    SDL_FreeSurface(start_surface);
    SDL_Rect start_rect = {0,0,1280,720};


    SDL_Surface* End_surface = SDL_LoadBMP("images/GameOver.bmp");
    SDL_Texture* End_Screen = SDL_CreateTextureFromSurface(renderer, End_surface);
    SDL_FreeSurface(End_surface);
    SDL_Rect Over_rect = {0,0,1280,720};

    std::vector<Tiles> tiles_vector_top;
    std::vector<Tiles> tiles_vector_bottom;

    int startX = 340;

    for (int i = 0; i < 10; i++)
    {
        int gapY = getRandomHeight();
        int bottom_tile_height = SCREEN_HEIGHT - gapY - PASSAGE_GAP;
        int top_tile_height = gapY;

        // bottom tiles
        Tiles bottom_tile(renderer, startX + i * (TILE_WIDTH + TILE_GAP), SCREEN_HEIGHT - bottom_tile_height, bottom_tile_height, false);
        bottom_tile.Load("images/wall1.bmp");
        tiles_vector_bottom.push_back(bottom_tile);

        // top tiles
        Tiles top_tile(renderer, startX + i * (TILE_WIDTH + TILE_GAP), 0, top_tile_height, true);
        top_tile.Load("images/wall1.bmp");
        tiles_vector_top.push_back(top_tile);
    }

    bool GameIsRunning = true;
    SDL_Event event;
    Uint32 startTime = SDL_GetTicks();
    bool isFrozen = false;
    Uint32 freezeStartTime = 0;
    const Uint32 freezeDuration = 1000; 
    while (GameIsRunning)
    {
        Uint32 currenttime = SDL_GetTicks();
        float deltatime = (currenttime - startTime);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                GameIsRunning = false;
            }
            if (event.type == SDL_KEYDOWN)
            {
                if (gameState == START && event.key.keysym.sym == SDLK_RETURN && gameState != GAMEOVER)
                {
                    gameState = PLAYING;
                }
                else if (gameState == PLAYING && event.key.keysym.sym == SDLK_SPACE)
                {
                    car.Jump();
                }
                else if (gameState == GAMEOVER)
                {
                    if (event.key.keysym.sym == SDLK_q)
                    {
                        GameIsRunning = false;
                    }
                    if (event.key.keysym.sym == SDLK_RETURN && gameState != PLAYING)
                    {
                        score = 0;
                        car.reset();
                        populateTiles(renderer, tiles_vector_top, tiles_vector_bottom);
                        startTime = SDL_GetTicks();
                        deltatime = 0;
                        SDL_Delay(50);
                        gameState = PLAYING; 
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (gameState == PLAYING)
        {
            SDL_RenderCopy(renderer, background, nullptr, &background1);


            car.Update();
            car.Render();

            for (auto &tiles : tiles_vector_bottom)
            {
                tiles.updatePosition(2);
                tiles.updateVerticalPosition(deltatime);
                tiles.Render();
                if (tiles.Collide(car.getRect()))
                {
                    gameState = GAMEOVER; 
                    freezeStartTime = SDL_GetTicks();
                }

                if (tiles.rect.x + TILE_WIDTH == car.getRect().x)
                {
                    score++;
                    std::cout << "Score: " << score << std::endl;
                }
            }

            for (auto &tile : tiles_vector_top)
            {
                tile.updatePosition(2);
                tile.updateVerticalPosition(deltatime);
                tile.Render();
                if (tile.Collide(car.getRect()))
                {
                    gameState = GAMEOVER;
                    freezeStartTime = SDL_GetTicks();
                }
            }

            snow_rect1.x -= 8;

            if (snow_rect1.x + snow_rect1.w <= 0)
            {
                snow_rect1.x = snow_rect2.x + snow_rect2.w; 
            }

            snow_rect2.x -= 8; 

            if (snow_rect2.x + snow_rect2.w <= 0)
            {
                snow_rect2.x = snow_rect1.x + snow_rect1.w; 
            }


            if (!tiles_vector_top.empty() && tiles_vector_bottom.front().rect.x + TILE_WIDTH < 0)
            { 
                int gapY = getRandomHeight();
                int bottom_tile_height = SCREEN_HEIGHT - gapY - PASSAGE_GAP;
                std::cout << bottom_tile_height << std::endl;
                int top_tile_height = gapY;

                Tiles recycleTile(renderer, tiles_vector_bottom.back().rect.x + TILE_WIDTH + TILE_GAP, SCREEN_HEIGHT - bottom_tile_height, bottom_tile_height, false);
                recycleTile.Load("images/wall1.bmp");
                tiles_vector_bottom.push_back(recycleTile);
                tiles_vector_bottom.erase(tiles_vector_bottom.begin());

                Tiles top_recycle_tile(renderer, tiles_vector_top.back().rect.x + TILE_WIDTH + TILE_GAP, 0, top_tile_height, true);
                top_recycle_tile.Load("images/wall1.bmp");
                tiles_vector_top.push_back(top_recycle_tile);
                tiles_vector_top.erase(tiles_vector_top.begin());
            }
            SDL_RenderCopy(renderer, snow1, nullptr, &snow_rect1);
            SDL_RenderCopy(renderer, snow1, nullptr, &snow_rect2);

            std::string stringscore = std::to_string(score);
            RenderText(renderer, stringscore, 640, 50, font);
        }

        else if (gameState == START)
        {
            SDL_RenderCopy(renderer, start_Screen, nullptr, &start_rect);
        }

        else if (gameState == GAMEOVER)
        {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - freezeStartTime < freezeDuration)
            {
                car.UpdateY(8);
                car.Render();
                for (auto &tile : tiles_vector_bottom)
                    tile.Render();
                for (auto &tile : tiles_vector_top)
                    tile.Render();

                RenderText(renderer, "OOPs!", 500, 350, font);
            }
            else
            {
                SDL_RenderCopy(renderer, End_Screen, nullptr,&Over_rect);
                RenderText(renderer, "Game Over!", 200, 200, font);
                std::string stringscore = std::to_string(score);
                RenderText(renderer, "YOUR HIGHSCORE IS :  " + stringscore, 200, 400, font);
                RenderText(renderer, "Press ENTER to Restart or Q to Quit", 200, 600, font);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); 
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyTexture(car.texture);

    for (auto &tile : tiles_vector_top)
    {
        SDL_DestroyTexture(tile.texture);
    }

    for (auto &tile : tiles_vector_bottom)
    {
        SDL_DestroyTexture(tile.texture);
    }

    SDL_DestroyTexture(car.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
