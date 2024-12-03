#include "MyGame.h"
#include <iostream>
#include <SDL_image.h>

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    lastMessageTime = SDL_GetTicks();
    
    if (cmd == "GAME_DATA") {
        if (args.size() == 4) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player1X = stoi(args.at(1));
            game_data.player2Y = stoi(args.at(2));
            game_data.player2X = stoi(args.at(3));
        }

        if (args.size() == 6) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player1X = stoi(args.at(1));
            game_data.player2Y = stoi(args.at(2));
            game_data.player2X = stoi(args.at(3));
            game_data.bulletX = stof(args.at(4));
            game_data.bulletY = stof(args.at(5));
        }

        if (args.size() == 8) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player1X = stoi(args.at(1));
            game_data.player2Y = stoi(args.at(2));
            game_data.player2X = stoi(args.at(3));

            game_data.bulletVelocityX = stof(args.at(6));
            game_data.bulletVelocityY = stof(args.at(7));

            //bullet_data.rect.x =  stoi(args.at(4));
           // bullet_data.rect.y = stoi(args.at(5));

            game_data.bulletX = stof(args.at(4));
            game_data.bulletY = stof(args.at(5));
        }
    }

    else if (cmd == "ID")
        game_data.id = stoi(args.at(0));

    else if (cmd == "INITIAL_DATA") {
        std::cout << "Recieved Initial Data" << std::endl;
        if (args.size() == 3) {
            game_data.id = stoi(args.at(0));
            numConnectedClients = stoi(args.at(1));
            currentTurn = stoi(args.at(2));
        }
        else {
            std::cout << "not size 3! " << args.size() <<  std::endl;
            send("INIT_DATA_REQ");
        }
    }

    else if (cmd == "TURN_CHANGE") {
        setCurrentTurn(stoi(args.at(0)));
        bulletOnScreen = false;
    }
         
    else if (cmd == "BULLET_SPAWN") {
        bulletOnScreen = true;
        game_data.bulletVelocityX = stof(args.at(0));
        game_data.bulletVelocityY = stof(args.at(1));
        game_data.bulletX = stof(args.at(2));
        game_data.bulletY = stof(args.at(3));
    }
        
    else if (cmd == "BULLET_DESPAWN") {
        bulletOnScreen = false;
        std::cout << "Recieved: " << cmd << std::endl;
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

        //up and down (archived)
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
    
    //TODO - calc own position +/- 60px per move
    //TODO - turn limits
    //TODO - winning and losing
    //TODO - client side prediction

    bullet_data.prevX = bullet_data.rect.x;
    bullet_data.rect.x = game_data.bulletX;

    bullet_data.prevY = bullet_data.rect.y;
    bullet_data.rect.y = game_data.bulletY;
    
    bullet_data.prevAngle = bullet_data.angle;

    float velocityX = bullet_data.rect.x - bullet_data.prevX;
    float velocityY = bullet_data.rect.y - bullet_data.prevY;

    float angleInRadians = std::atan2f(velocityY , velocityX); // radians
    bullet_data.angle = 180 * angleInRadians / 3.14f; //degrees

    if (bullet_data.angle == 0)
        bullet_data.angle = bullet_data.prevAngle;

    player1.y = game_data.player1Y;
    player1.x = game_data.player1X;
    player2.y = game_data.player2Y;
    player2.x = game_data.player2X;

    //Calc Barrel positions
    player1Barrel.x = player1.x + (player1.w - player1Barrel.w) / 2;
    player1Barrel.y = player1.y + (player1.h - player1Barrel.h) / 2;

    player2Barrel.x = player2.x + (player2.w - player2Barrel.w) / 2;
    player2Barrel.y = player2.y + (player2.h - player2Barrel.h) / 2;
      
    prevMouseX = game_data.cursorX;
    prevMouseY = game_data.cursorY;

    //std::cout << bullet_data.angle << std::endl;
    
    SDL_GetMouseState(&game_data.cursorX, &game_data.cursorY);

    //only send if changed (minimise traffic) and is current turn
    if ((game_data.cursorX != prevMouseX || game_data.cursorY != prevMouseY) && isCurrentTurn())    
        send("MP(" + std::to_string(game_data.cursorX) + "." + std::to_string(game_data.cursorY) + ")");
       
    
    angleInRadians = std::atan2f(game_data.cursorX - player1.x, game_data.cursorY - player1.y);
    p1BarrelAngle = 180 * angleInRadians / 3.14f;
    //p1BarrelAngle = p1BarrelAngle - 180;
    
    bullet.x = game_data.bulletX;
    bullet.y = game_data.bulletY;
}

void MyGame::render(SDL_Renderer* renderer) {   
    
    loadTextures(renderer);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   
    //Render

    SDL_RenderCopy(renderer, backgroundTexture, NULL, &background); //render first as other assets go on top

    if (bulletOnScreen)
        //SDL_RenderDrawRect(renderer, &bullet);
        SDL_RenderCopyEx(renderer, bulletTexture, NULL, &bullet_data.rect, bullet_data.angle, NULL, SDL_FLIP_NONE);
    
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block1);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block2);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block3);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block4);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block5);
    
    SDL_RenderCopy(renderer, tankTexture, NULL, &player1);
    SDL_RenderCopy(renderer, tankTexture, NULL, &player2);
   // SDL_RenderCopy(renderer, barrelTexture, NULL, &player1Barrel);
    SDL_RenderCopy(renderer, barrelTexture, NULL, &player2Barrel);
    
    //SDL_RenderCopy(renderer, barrelTexture, NULL, &player2Barrel);
    SDL_RenderCopyEx(renderer, barrelTexture, NULL, &player1Barrel, p1BarrelAngle, NULL, SDL_FLIP_NONE);    
    //SDL_RenderCopyEx(renderer, tankTexture, NULL, &player1, N, (NULL), SDL_FLIP_NONE);
    
}

void MyGame::PredictBulletPosition(float delta) {
    //Last known recieved from server
    int serverPositionX = game_data.bulletX;
    int serverPositionY = game_data.bulletY;
    predictedX = serverPositionX + game_data.bulletVelocityX ;
    predictedY = serverPositionY + game_data.bulletVelocityY ;

    //predictedX = bullet.x + vx * delta ??
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

void MyGame::loadTextures(SDL_Renderer* renderer) {
    if (!hasLoadedTextures) {
        wallTexture = IMG_LoadTexture(renderer, "../assets/walls.jpg");
        walls2Texture = IMG_LoadTexture(renderer, "../assets/walls2.jpg");
        tankTexture = IMG_LoadTexture(renderer, "../assets/testTank2.png");
        barrelTexture = IMG_LoadTexture(renderer, "../assets/barrel.png");
        backgroundTexture = IMG_LoadTexture(renderer, "../assets/background2.jpg");
        bulletTexture = IMG_LoadTexture(renderer, "../assets/bullet.png");
        hasLoadedTextures = true;
    }
   
}
void MyGame::destroyTextures() {
    //Destroy Textures // Stop memory leaks
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyTexture(walls2Texture);
    SDL_DestroyTexture(tankTexture);
    SDL_DestroyTexture(barrelTexture);
}