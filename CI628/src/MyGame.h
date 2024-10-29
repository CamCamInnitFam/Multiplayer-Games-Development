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
    int ballX = 0;
    int ballY = 0;
} game_data;

class MyGame {

    private:
        SDL_Rect player1 = { 0, 0, 60, 60 };
        SDL_Rect player2 = { 0, 0, 60, 60 };
        SDL_Rect ball = { 0, 0, 5, 5 };
        SDL_Rect block1 = { 600, 340, 60, 120 };
        SDL_Rect block2 = { 420,220,60, 120};

    public:
        std::vector<std::string> messages;

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update();
        void render(SDL_Renderer* renderer);
};

#endif