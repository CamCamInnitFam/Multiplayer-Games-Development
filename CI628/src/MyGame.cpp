#include "MyGame.h"
#include <iostream>

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    lastMessageTime = SDL_GetTicks();
    
    if (cmd == "GAME_DATA") {
        if (args.size() == 4) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player1X = stoi(args.at(1));
            game_data.player2Y = stoi(args.at(2));
            game_data.player2X = stoi(args.at(3));
            //game_data.bulletX = stoi(args.at(4));
            //game_data.bulletY = stoi(args.at(5));
        }
        if (args.size() == 6) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player1X = stoi(args.at(1));
            game_data.player2Y = stoi(args.at(2));
            game_data.player2X = stoi(args.at(3));
            game_data.bulletX = stof(args.at(4));
            game_data.bulletY = stof(args.at(5));
        }
    }

    else if (cmd == "ID")
        game_data.id = stoi(args.at(0));

    else if (cmd == "BULLET_SPAWN") {
        bulletOnScreen = true;
        game_data.bulletVelocityX = stof(args.at(0));
        game_data.bulletVelocityY = stof(args.at(1));
        game_data.bulletX = stof(args.at(2));
        game_data.bulletY = stof(args.at(3));
    }
        
    else if (cmd == "BULLET_DESPAWN")
        bulletOnScreen = false;

    else if (cmd == "*")
        setCurrentTurn(game_data.id);    

    else if (cmd == "CONNECTEVENT") {
        
    }
             
    else 
        std::cout << "Received: " << cmd << std::endl;

    //if cmd .includes ("PLAYER)"
    //id = args.stoi(*id index*)
    //player[id].NetworkUpdate(cmd, args) ?
}

void MyGame::send(std::string message) {
    messages.push_back(message);
}

void MyGame::HeartBeat() {
    if (SDL_GetTicks() > nextSendTime) {
        send("heartbeat, " + std::to_string(game_data.id));
        nextSendTime = SDL_GetTicks() + 1000;
    }
}

void MyGame::input(SDL_Event& event) 
{  
    //Ignore Input if not current turn
    if (!isCurrentTurn())
        return;

    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) 
    {
        if (event.button.button == 1) { //left click                        
            //Send Input                                    
            send("LMB_DOWN");
            SDL_Delay(100);
            send("LMB_UP");
            send(std::to_string(game_data.id));

            //send(event.type == SDL_MOUSEBUTTONDOWN ? "LMB_DOWN" : "LMB_UP");
        }
        return;
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

        //Turn end
    case SDLK_SPACE:
        send("TURN_END");
        if (game_data.id == 0 && isCurrentTurn())
            setCurrentTurn(1);
        if (game_data.id == 1 && isCurrentTurn())
            setCurrentTurn(0);
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
    
    float deltaTime = SDL_GetTicks() - lastMessageTime / 1000;
    lastMessageTime = SDL_GetTicks();
    //Send heartbeat to server
    HeartBeat();
    
    player1.y = game_data.player1Y;
    player1.x = game_data.player1X;
    player2.y = game_data.player2Y;
    player2.x = game_data.player2X;
    
    prevX = game_data.cursorX;
    prevY = game_data.cursorY;
    SDL_GetMouseState(&game_data.cursorX, &game_data.cursorY);

    //only send if changed (minimise traffic) and is current turn
    if ((game_data.cursorX != prevX || game_data.cursorY != prevY) && isCurrentTurn())    
        send("MP(" + std::to_string(game_data.cursorX) + "." + std::to_string(game_data.cursorY) + ")");
       
    //sync with server
    //if (std::sqrt((predictedX - game_data.bulletX) * (predictedX - game_data.bulletX)) + ((predictedY - game_data.bulletY) * (predictedY - game_data.bulletY)) > 0.01f)
   //    interpolationTime = 0.0f;

   ////if have not recieved update in 0.4 seconds
   //if (deltaTime > 0.4) {
   //    std::cout << deltaTime;
   //    PredictBulletPosition(deltaTime);
   //    bullet.y = predictedY;
   //    bullet.x = predictedX;
   //}
   //else {
   //    bullet.y = game_data.bulletY;
   //    bullet.x = game_data.bulletX;
   //}
   
    bullet.x = game_data.bulletX;
    bullet.y = game_data.bulletY;
}

void MyGame::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &player1);
    SDL_RenderDrawRect(renderer, &player2);
    SDL_RenderDrawRect(renderer, &block1);
    SDL_RenderDrawRect(renderer, &block2);
    SDL_RenderDrawRect(renderer, &block3);
    SDL_RenderDrawRect(renderer, &block4);
    SDL_RenderDrawRect(renderer, &block5);

    if (bulletOnScreen)
        SDL_RenderDrawRect(renderer, &bullet);
}

void MyGame::PredictBulletPosition(float delta) {
    //Last known recieved from server
    int serverPositionX = game_data.bulletX;
    int serverPositionY = game_data.bulletY;
    predictedX = serverPositionX + game_data.bulletVelocityX ;
    predictedY = serverPositionY + game_data.bulletVelocityY ;
}

void MyGame::Interpolate(float delta) {
    if (interpolationTime < 0.01f) {
        float interpolationFactor = interpolationTime / 0.01f;
        predictedX = predictedX * (1.0f - interpolationFactor) + game_data.bulletX * interpolationFactor;
        predictedY = predictedY * (1.0f - interpolationFactor) + game_data.bulletY * interpolationFactor;
        interpolationTime += delta;
    }
    else {
        predictedX = game_data.bulletX;
        predictedY = game_data.bulletY;
    }       
}

void MyGame::setCurrentTurn(int x) {
    currentTurn = x;
}

int MyGame::getCurrentTurn() {
    return currentTurn;
}

bool MyGame::isCurrentTurn() {
    if (currentTurn == game_data.id)
        return true;   
    return false;
}
void MyGame::syncPlayerPos() {

}

int MyGame::getPlayerId() {
    return game_data.id;
}