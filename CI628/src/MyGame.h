#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <iostream>
#include <vector>
#include <string>

#include "SDL.h"

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
} game_data;

class MyGame {

    private:
        SDL_Rect player1 = { 0, 0, 60, 60 };
        SDL_Rect player2 = { 0, 0, 60, 60 };
        SDL_Rect bullet = { 0, 0, 8, 8 };
        SDL_Rect block1 = { 600, 380, 60, 120 };
        SDL_Rect block2 = { 420, 80, 60, 120};
        SDL_Rect block3 = { 780, 80,  60, 120};
        SDL_Rect block4 = { 420, 620, 60, 120 };
        SDL_Rect block5 = { 780, 620, 60, 120 };
        

    public:
        std::vector<std::string> messages;
        int prevX, prevY;
        bool bulletOnScreen = false;
        Uint32 nextSendTime = SDL_GetTicks();
        int currentTurn = 0; //TODO have timer for turn and will auto send (END_TURN) to server

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update();
        void render(SDL_Renderer* renderer);
        void HeartBeat();
        int getCurrentTurn();
        void setCurrentTurn(int);
        bool isCurrentTurn();
};

#endif