#include "MyGame.h"
#include <iostream>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>


MyGame::MyGame() {

}

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {  
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

            if (predicting) {
                if (game_data.bulletX - bullet_data.rect.x > 2)
                    bullet_data.rect.x = game_data.bulletX;
                if (game_data.bulletY - bullet_data.rect.y > 2)
                    bullet_data.rect.y = game_data.bulletY;
            }                    
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
        std::cout << "BULLET SPAWN" << std::endl;
        bulletOnScreen = true;
        game_data.bulletVelocityX = stof(args.at(0));
        game_data.bulletVelocityY = stof(args.at(1));
        game_data.bulletX = stof(args.at(2));
        game_data.bulletY = stof(args.at(3));
        bullet_data.rect.x = game_data.bulletX;
        bullet_data.rect.y = game_data.bulletY;

        //play sound
        Mix_PlayChannel(-1, bulletSound, 0);
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
        std::cout << "Recieved Connect Event" << std::endl;
        if (args.size() > 1) {
            connectData.connectionID = stoi(args.at(2));
            checkConnectTextTime = SDL_GetTicks();

            if (args.at(1) == "CONNECT") {
                if (connectData.connectionID != -1)
                    connectData.connectMessage = "Player " + std::to_string(connectData.connectionID + 1) + " Connected!";
                else
                    connectData.connectMessage = "Spectator Connected!";
            }

            else if (args.at(1) == "DISCONNECT") {
                if (connectData.connectionID != -1)
                    connectData.connectMessage = "Player " + std::to_string(connectData.connectionID + 1) + " Disconnected!";
                else
                    connectData.connectMessage = "Spectator Disconnected!";
            }
        }               
    }

    else if (cmd == "GAMEWIN" || cmd == "GAMEWINGAME_DATA") {
        std::cout << "Recieved game win" << std::endl;
        game_data.winner = stoi(args.at(0));
        gameWon = true;
        if (game_data.winner == game_data.id) {
            isWinner = true;
        }          
    }

    else if (cmd == "RESTART") 
        restartGame();
                             
    else 
        std::cout << "Received: " << cmd << std::endl;
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
        if (event.button.button == 1 && !hasShot) { //left click                        
            //Send Input                                    
            send("LMB_DOWN");
            SDL_Delay(100);
            send("LMB_UP");
            hasShot = true;
        }
        return;
    }
           
    switch (event.key.keysym.sym) {
    case(SDLK_RETURN):
        if (!gameWon)
            break;
        if (gameWon && isWinner) 
        {
            send("RESTART_GAME");
        }
           
        break;
        //up and down
    case SDLK_w:
        if (numMoves < maxMoves) {
            send(event.type == SDL_KEYDOWN ? "W_DOWN" : "W_UP");
            numMoves++;
        }       
        break;
    case SDLK_s:
        if (numMoves < maxMoves){
            send(event.type == SDL_KEYDOWN ? "S_DOWN" : "S_UP");
            numMoves++;
        }
        
        break;

        //move left and right
    case SDLK_a:
        if (numMoves < maxMoves) {
            send(event.type == SDL_KEYDOWN ? "A_DOWN" : "A_UP");
            numMoves++;
        }    
        break;
    case SDLK_d:
        if (numMoves < maxMoves) {
            send(event.type == SDL_KEYDOWN ? "D_DOWN" : "D_UP");
            numMoves++;
        }      
        break;

        //Turn end
    case SDLK_SPACE:
        //Send to Server 
        send("TURN_END");

        //Change Turn Locally
        currentTurn = currentTurn == 0 ? 1 : 0;

        //Reset Turn info
        numMoves = 0;
        hasShot = false;

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
    
    if (!isServerActive) return;

    float getTicksf = SDL_GetTicks();
   
    float deltaTime = (getTicksf - lastMessageTime) / 1000;
    lastMessageTime = getTicksf;
    
    //Send heartbeat to server
    HeartBeat();

    if (hasShot && numMoves >= maxMoves && !bulletOnScreen) {
        SDL_Delay(100);
        send("TURN_END");
        hasShot = false;
        numMoves = 0;
        currentTurn = currentTurn = 0 ? 1 : 0;
    }
    
    //TODO - calc own position +/- 60px per move

    float velocityX;
    float velocityY;

    if (predicting) {

        //Grab last known velocity from server
        //Necessary as velocity changes due to bouncing
        velocityX = game_data.bulletVelocityX;
        velocityY = game_data.bulletVelocityY;

        //Predict the position
        bullet_data.rect.x += velocityX * deltaTime;
        bullet_data.rect.y += velocityY * deltaTime;

        //Ensure in bounds
        if (bullet_data.rect.x + bullet_data.rect.w > 1200)
            bullet_data.rect.x = 1200 - bullet_data.rect.w;
        if (bullet_data.rect.y + bullet_data.rect.h > 840)
            bullet_data.rect.y = 840 - bullet_data.rect.h;
    }
    else {
        bullet_data.prevX = bullet_data.rect.x;
        bullet_data.rect.x = game_data.bulletX;

        bullet_data.prevY = bullet_data.rect.y;
        bullet_data.rect.y = game_data.bulletY;

        velocityX = bullet_data.rect.x - bullet_data.prevX;
        velocityY = bullet_data.rect.y - bullet_data.prevY;
    }

    bullet_data.prevAngle = bullet_data.angle;
    float angleInRadians = std::atan2f(velocityY , velocityX); // radians
    bullet_data.angle = 180 * angleInRadians / 3.14f; //degrees

    //Ensure angle is never 0 value
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
    std::string p1Text = "PLAYER 1 SCORE  O"; //I is seperator, O is 0
    std::string p2Text = "PLAYER 2 SCORE  O";
    std::string movesLeftText = "Moves Left   " + (maxMoves-numMoves == 0 ? "O" : std::to_string((maxMoves - numMoves)/2));
    std::string bulletsLeftText = "Bullets Left   " + std::string(hasShot ? "O" : "1");
    std::string playerTurnText = "Player Turn ";
    std::string turnNumberText = std::to_string(getCurrentTurn() + 1);
    
    if (SDL_GetTicks() > checkConnectTextTime + 3000)
        connectData.connectMessage = "";

    if (game_data.id == -1) //spectator
    {
        movesLeftText = "";
        bulletsLeftText = "";
    }
               
    SDL_Rect p1ScoreRect = { 100,0,250,50 };
    SDL_Rect p2ScoreRect = { 850,0,250,50 };  
    SDL_Rect playerTurnRect = { 20,70,200,40 };
    SDL_Rect turnNumberRect = { 230,75,15,40 };
    SDL_Rect movesLeftRect = { 20,150,200,40 };
    SDL_Rect shotsLeftRect = { 20,200,205,40 };

    //Textures (text)
    bool connected = connectData.connectMessage.find("Connected") != std::string::npos;
    int connectRectWidth = 250;

    if (!isServerActive) {
        connectData.connectMessage = "Server Connection Closed. Exiting Application...";
        connected = false;
        connectRectWidth = connectRectWidth + 150;
    }

    if (gameWon) {
        if (isWinner) {
            connectData.connectMessage = "You Win! Press Enter to Restart.";
            connected = true;
            connectRectWidth = connectRectWidth + 100;
        }         
        else {
            connectData.connectMessage = "You Lose! Awaiting Restart.";
            connected = false;
            connectRectWidth = connectRectWidth + 75;
        }
    }

    SDL_Rect connectRect = { 20,780,connectRectWidth,50 };

    SDL_Texture* p1Label = renderText(game_data.p1Score > 0 ? ("PLAYER 1 SCORE   " + std::to_string(game_data.p1Score)).c_str() : p1Text.c_str(), font, white, renderer);
    SDL_Texture* p2Label = renderText(game_data.p2Score > 0 ? ("PLAYER 2 SCORE   " + std::to_string(game_data.p2Score)).c_str() : p2Text.c_str(), font, white, renderer);
    SDL_Texture* connectLabel = renderText(connectData.connectMessage.c_str(), font, connected ? green : red, renderer);
    SDL_Texture* turnLabel = renderText(playerTurnText.c_str(), font, white, renderer);
    SDL_Texture* turnNumberLabel = renderText(turnNumberText.c_str(), freshmanFont, isCurrentTurn() ? green : red, renderer);
    SDL_Texture* movesLeftLabel = renderText(movesLeftText.c_str(), font, white, renderer);
    SDL_Texture* shotsLeftLabel = renderText(bulletsLeftText.c_str(), font, white, renderer);
    
    //RenderCopy Text
    SDL_RenderCopy(renderer, p1Label, NULL, &p1ScoreRect);
    SDL_RenderCopy(renderer, p2Label, NULL, &p2ScoreRect);
    SDL_RenderCopy(renderer, connectLabel, NULL, &connectRect);
    SDL_RenderCopy(renderer, turnLabel, NULL, &playerTurnRect);
    SDL_RenderCopy(renderer, turnNumberLabel, NULL, &turnNumberRect);
    SDL_RenderCopy(renderer, movesLeftLabel, NULL, &movesLeftRect);
    SDL_RenderCopy(renderer, shotsLeftLabel, NULL, &shotsLeftRect);
    
    //Destroy
    SDL_DestroyTexture(p1Label);
    SDL_DestroyTexture(p2Label);
    SDL_DestroyTexture(connectLabel);    
    SDL_DestroyTexture(turnLabel);
    SDL_DestroyTexture(turnNumberLabel);
    SDL_DestroyTexture(movesLeftLabel);
    SDL_DestroyTexture(shotsLeftLabel);
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


int MyGame::getPlayerId() {
    return game_data.id;
}

void MyGame::loadAssets(SDL_Renderer* renderer) {
    if (!hasLoadedTextures) {

        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            printf("Failed to initialise Audio!", SDL_GetError());
            exit(6);
        }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            printf("SDL mixer could not initialise!", SDL_GetError());
            exit(6);
        }
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
        freshmanFont = TTF_OpenFont("../assets/fonts/Freshman.ttf", 15);
        if (!freshmanFont) {
            printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
            exit(6);
        }
        bulletSound = Mix_LoadWAV("../assets/gunShot.wav");
        Mix_VolumeChunk(bulletSound, 10);


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
void MyGame::setServerActive(bool isActive) {
    isServerActive = isActive;
    if (!isServerActive)
        std::cout << "Server Inactive!" << std::endl;
}
void MyGame::restartGame() 
{
    isWinner = false;
    gameWon = false;
    game_data.p1Score = 0;
    game_data.p2Score = 0;
    hasShot = false;
    numMoves = 0;
    currentTurn = 0;
    bulletOnScreen = false;
    game_data.winner = 0;
}