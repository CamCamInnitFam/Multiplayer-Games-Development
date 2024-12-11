#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <iostream>
#include <vector>
#include <string>

#include "SDL.h"
#include <SDL_ttf.h>

static struct GameData {
    int player1Y = 0;
    int player1X = 0;
    int player2Y = 0;
    int player2X = 0;
    int bulletX = 0;
    int bulletY = 0;
    int cursorX = 0;
    int cursorY = 0;
    int id = -1;
    float bulletVelocityX;
    float bulletVelocityY;
    float barrelRotation = 0;
    int p1Score = 0;
    int p2Score = 0;
} game_data;

static struct BulletData {
    SDL_Rect rect = { 0,0, 18,18 };
    int prevX = 0;
    int prevY = 0;
    float angle = 0;
    float prevAngle = 0;
} bullet_data;

static struct ConnectData {
    std::string connectMessage = "";
    std::string oldConnectMessage = "";
    int connectionID;
} connectData;

class MyGame {

    private:
        SDL_Rect player1 = { 0, 0, 60, 60 };
        SDL_Rect player1Barrel = {0,0, 50, 30};
        SDL_Rect player2Barrel = { 0, 0, 50, 30 };
        SDL_Rect player2 = { 0, 0, 60, 60 };
        SDL_Rect bullet = { 0, 0, 8, 8 };
        SDL_Rect block1 = { 600, 380, 60, 120 };
        SDL_Rect block2 = { 420, 80, 60, 120};
        SDL_Rect block3 = { 780, 80,  60, 120};
        SDL_Rect block4 = { 420, 620, 60, 120 };
        SDL_Rect block5 = { 780, 620, 60, 120 };
        SDL_Rect background = { 0,0,1200,840 };

        //Textures
        SDL_Texture* wallTexture;
        SDL_Texture* walls2Texture;
        SDL_Texture* tankTexture;
        SDL_Texture* barrelTexture;
        SDL_Texture* backgroundTexture;
        SDL_Texture* bulletTexture;

        //Font
        TTF_Font* font;
        TTF_Font* freshmanFont;

        int maxMoves = 12; // divide by 2 for 6 max moves
        
    public:
        std::vector<std::string> messages;
        int prevMouseX;
        int prevMouseY;
        float predictedX;
        float predictedY;
        bool bulletOnScreen = false;
        Uint32 nextSendTime = SDL_GetTicks();
        Uint32 lastFrameTime = SDL_GetTicks();
        Uint32 lastMessageTime;
        int currentTurn = 0; //TODO have timer for turn and will auto send (END_TURN) to server
        float interpolationTime = 0.0f;
        bool initialSync = true;
        int numConnectedClients = 1;
        bool hasLoadedTextures = false;
        float p1BarrelAngle = 0;
        float p2BarrelAngle = 0;
        Uint32 checkConnectTextTime;
        bool hasShot = false;
        int numMoves = 0;
        bool isServerActive = true;

        MyGame();
        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update();
        void render(SDL_Renderer* renderer);
        void HeartBeat();
        int getCurrentTurn();
        void setCurrentTurn(int);
        bool isCurrentTurn();
        void PredictBulletPosition(float delta);
        void Interpolate(float delta);
        void syncPlayerPos();
        int getPlayerId();
        void loadAssets(SDL_Renderer* renderer);
        void destroyTextures();
        SDL_Texture* renderText(const char* message, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer);
        void setServerActive(bool isActive);
};

#endif