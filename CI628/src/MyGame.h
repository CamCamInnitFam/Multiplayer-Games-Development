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
} game_data;

class MyGame {

    private:
        SDL_Rect player1 = { 0, 0, 60, 60 };
        SDL_Rect player2 = { 0, 0, 60, 60 };
        SDL_Rect bullet = { 0, 0, 8, 8 };
        SDL_Rect block1 = { 600, 340, 60, 120 };
        SDL_Rect block2 = { 420,220,60, 120};
        

    public:
        std::vector<std::string> messages;
        int prevX, prevY;
        bool bulletOnScreen = false;

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update();
        void render(SDL_Renderer* renderer);
};

#endif