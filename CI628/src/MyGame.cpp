#include "MyGame.h"
#include <iostream>

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    if (cmd == "GAME_DATA") {        
        if (args.size() == 4) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player1X = stoi(args.at(1));
            game_data.player2Y = stoi(args.at(2));
            game_data.player2X = stoi(args.at(3));
            //game_data.ballX = stoi(args.at(4));
            //game_data.ballY = stoi(args.at(5));
        }
    } else {
        std::cout << "Received: " << cmd << std::endl;
    }
}

void MyGame::send(std::string message) {
    messages.push_back(message);
}

void MyGame::input(SDL_Event& event) 
{   
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) 
    {
        if (event.button.button == 1) { //left click
            send(event.type == SDL_MOUSEBUTTONDOWN ? "LMB_DOWN" : "LMB_UP");
            return;
        }
    }
           
    switch (event.key.keysym.sym) {
        //USER1

        //up and down
    case SDLK_w:
        send(event.type == SDL_KEYDOWN ? "W_DOWN" : "W_UP");
        break;
    case SDLK_s:
        send(event.type == SDL_KEYDOWN ? "S_DOWN" : "S_UP");
        break;

        //move left and right
    case SDLK_a:
        send(event.type == SDL_KEYDOWN ? "A_DOWN" : "A_UP");
        break;
    case SDLK_d:
        send(event.type == SDL_KEYDOWN ? "D_DOWN" : "D_UP");
        break;

        //USER 2

        //up and down
    case SDLK_i:
        send(event.type == SDL_KEYDOWN ? "I_DOWN" : "I_UP");
        break;
    case SDLK_k:
        send(event.type == SDL_KEYDOWN ? "K_DOWN" : "K_UP");
        break;
    case SDLK_j:
        send(event.type == SDL_KEYDOWN ? "J_DOWN" : "J_UP");
        break;
    case SDLK_l:
        send(event.type == SDL_KEYDOWN ? "L_DOWN" : "L_UP");
        break;      
    }
}

void MyGame::update() {
    player1.y = game_data.player1Y;
    player1.x = game_data.player1X;
    player2.y = game_data.player2Y;
    player2.x = game_data.player2X;
   // ball.y = game_data.ballY;
   // ball.x = game_data.ballX;
}

void MyGame::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &player1);
    SDL_RenderDrawRect(renderer, &player2);
    //SDL_RenderDrawRect(renderer, &ball);
    SDL_RenderDrawRect(renderer, &block1);
    SDL_RenderDrawRect(renderer, &block2);
}