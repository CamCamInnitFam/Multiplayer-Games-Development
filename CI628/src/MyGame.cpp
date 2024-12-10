#include "MyGame.h"
#include <iostream>
#include <SDL_image.h>
#include <SDL_ttf.h>


MyGame::MyGame() {

}

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

    else if (cmd == "BARREL_ROTATION") {
        game_data.barrelRotation = stof(args.at(0));      
    }

    else if (cmd == "SCORES") {
        game_data.p1Score = stoi(args.at(0));
        game_data.p2Score = stoi(args.at(1));
    }
    else if (cmd == "CONNECTEVENT") {
        //numConnectedClients = stoi(args.at(0));
        std::cout << "Recieved ConnectEvent!" << std::endl;
        connectData.connectionID = stoi(args.at(2));
        checkConnectTextTime = SDL_GetTicks();
        if (args.at(1) == "CONNECT")           
            connectData.connectMessage = "Player " + std::to_string(connectData.connectionID +1)  + " Connected!";
                   
        else if (args.at(1) == "DISCONNECT")
            connectData.connectMessage = "Player " + std::to_string(connectData.connectionID +1)  + " Disconnected!";
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


    //Calculate or set rotational values for tank barrels
    if (isCurrentTurn()) 
    {
        if (game_data.id == 0) 
        {
            angleInRadians = std::atan2f(game_data.cursorY - player1Barrel.y, game_data.cursorX - player1Barrel.x);

            p1BarrelAngle = 180 * angleInRadians / 3.14f;
            p1BarrelAngle = p1BarrelAngle - 12; //hack to make more accurate         
        }
        else {
            angleInRadians = std::atan2f(game_data.cursorY - player2Barrel.y, game_data.cursorX - player2Barrel.x);

            p2BarrelAngle = 180 * angleInRadians / 3.14f;
            p2BarrelAngle = p2BarrelAngle - 12; //hack to make more accurate    
        }
    }
    else {
        if (game_data.id == 0) {
            p2BarrelAngle = game_data.barrelRotation;
        }
        else if (game_data.id == 1) {
            p1BarrelAngle = game_data.barrelRotation;
        }
        else if (game_data.id == -1) {
            if (getCurrentTurn() == 0)
                p1BarrelAngle = game_data.barrelRotation;
            else
                p2BarrelAngle = game_data.barrelRotation;
        }
    }
        
    
    //p1BarrelAngle = p1BarrelAngle - 180;
    
    bullet.x = game_data.bulletX;
    bullet.y = game_data.bulletY;
}

void MyGame::render(SDL_Renderer* renderer) {   
    
    loadAssets(renderer);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color green = { 0, 255, 0, 255 };
    SDL_Color red = { 255, 0 , 0, 255 };
   
    //Render

    //background
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &background); //render first as other assets go on top

    //bullet
    if (bulletOnScreen)
        SDL_RenderCopyEx(renderer, bulletTexture, NULL, &bullet_data.rect, bullet_data.angle, NULL, SDL_FLIP_NONE);
       
    //walls
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block1);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block2);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block3);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block4);
    SDL_RenderCopy(renderer, walls2Texture, NULL, &block5);
       
    //tanks
    SDL_RenderCopy(renderer, tankTexture, NULL, &player1);
    SDL_RenderCopy(renderer, tankTexture, NULL, &player2);
    SDL_RenderCopyEx(renderer, barrelTexture, NULL, &player1Barrel, p1BarrelAngle, NULL, SDL_FLIP_NONE);    
    SDL_RenderCopyEx(renderer, barrelTexture, NULL, &player2Barrel, p2BarrelAngle, NULL, SDL_FLIP_NONE);

    //Game Text
    std::string p1Text = "PLAYER 1 SCORE  I  O"; //I is seperator, O is 0
    std::string p2Text = "PLAYER 2 SCORE  I  O";
    if (SDL_GetTicks() > checkConnectTextTime + 3000)
        connectData.connectMessage = "";
            
    SDL_Rect p1ScoreRect = { 100,0,250,60 };
    SDL_Rect p2ScoreRect = { 800,0,250,60 };
    SDL_Rect connectRect = { 20,100,250,40};

    //Textures (text)
    bool connected = connectData.connectMessage.find("Connected") != std::string::npos;
    SDL_Texture* p1Label = renderText(game_data.p1Score > 0 ? ("PLAYER 1 SCORE  I  " + std::to_string(game_data.p1Score)).c_str() : p1Text.c_str(), font, white, renderer);
    SDL_Texture* p2Label = renderText(game_data.p2Score > 0 ? ("PLAYER 2 SCORE  I  " + std::to_string(game_data.p2Score)).c_str() : p2Text.c_str(), font, white, renderer);
    SDL_Texture* connectLabel = renderText(connectData.connectMessage.c_str(), font, connected ? green : red, renderer);

    
    //RenderCopy Text
    SDL_RenderCopy(renderer, p1Label, NULL, &p1ScoreRect);
    SDL_RenderCopy(renderer, p2Label, NULL, &p2ScoreRect);
    SDL_RenderCopy(renderer, connectLabel, NULL, &connectRect);
    
    //Destroy
    SDL_DestroyTexture(p1Label);
    SDL_DestroyTexture(p2Label);
    SDL_DestroyTexture(connectLabel);    
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

void MyGame::loadAssets(SDL_Renderer* renderer) {
    if (!hasLoadedTextures) {
        wallTexture = IMG_LoadTexture(renderer, "../assets/walls.jpg");
        walls2Texture = IMG_LoadTexture(renderer, "../assets/walls2.jpg");
        tankTexture = IMG_LoadTexture(renderer, "../assets/testTank2.png");
        barrelTexture = IMG_LoadTexture(renderer, "../assets/barrel.png");
        backgroundTexture = IMG_LoadTexture(renderer, "../assets/background2.jpg");
        bulletTexture = IMG_LoadTexture(renderer, "../assets/bullet.png");
        font = TTF_OpenFont("../assets/fonts/Hey Comic.ttf", 25);
        if (!font) {
            printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
            exit(6);
        }
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

SDL_Texture* MyGame:: renderText(const char* message, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}