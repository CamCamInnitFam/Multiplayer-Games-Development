#include "SDL_net.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <Windows.h>


#include "MyGame.h"

using namespace std;

const char* IP_NAME = "localhost";
const Uint16 PORT = 55555;

bool is_running = true;
bool game_started = false;
bool is_connecting = true;
bool has_made_connection = false;
int numConnections = 1;
bool gameActive = false;

struct connectionData 
{
    boolean success;
    const char* message;

    connectionData(boolean a, const char* b) : success(a), message(b) {}
};

MyGame* game = new MyGame();
TCPsocket overallSocket;

static int on_receive(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    const int message_length = 1024;

    char message[message_length];
    int received;

    // TODO: while(), rather than do
    do {
        received = SDLNet_TCP_Recv(socket, message, message_length);
        message[received] = '\0';

        char* pch = strtok(message, ",");

        // get the command, which is the first string in the message
        string cmd(pch);

        // then get the arguments to the command
        vector<string> args;

        while (pch != NULL) {
            pch = strtok(NULL, ",");

            if (pch != NULL) {
                args.push_back(string(pch));
            }
        }

        if (cmd == "exit")
            break;

        if (cmd == "CONNECTEVENT") {
            numConnections = stoi(args.at(0));
        }

        if (cmd == "GAMESTATE" || cmd == "GAMESTATEGAME_DATA") { //Sometimes server gets confused. This is a hack.
                std::cout << "GameState Recieved" << std::endl;
                gameActive = true; //allows starting in lobby                                                    
        }          
        
        if (cmd == "GAME_START") 
            game_started = true; //exits lobby loop, lets program know to kill threads when game is closed

        game->on_receive(cmd, args);

        if (cmd == "INITIAL_DATA")
            numConnections = game->numConnectedClients;
                    
    } while (received > 0 && is_running);

    return 0;
}

static int on_send(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    while (is_running) {
        if (game->messages.size() > 0) {
            string message = "CLIENT_DATA";

            for (auto m : game->messages) {
                message += "," + m;
            }

            game->messages.clear();

            cout << "Sending_TCP: " << message << endl;

            SDLNet_TCP_Send(socket, message.c_str(), message.length());
        }

        SDL_Delay(1);
    }

    return 0;
}

void loop(SDL_Renderer* renderer) {
    SDL_Event event;

    while (is_running) {
        // input
        while (SDL_PollEvent(&event)) {
            if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONDOWN) && event.key.repeat == 0) {
                game->input(event);

                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        is_running = false;
                        break;

                    default:
                        break;
                }
            }

            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        game->update();

        game->render(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(17);
    }
}

int run_game() {
    SDL_Window* window = SDL_CreateWindow(
        "Multiplayer Tankz Client",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1200, 840,
        SDL_WINDOW_SHOWN
    );

    if (nullptr == window) {
        std::cout << "Failed to create window" << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (nullptr == renderer) {
        std::cout << "Failed to create renderer" << SDL_GetError() << std::endl;
        return -1;
    }

    loop(renderer);

    return 0;
}

// Function to render text
SDL_Texture* renderText(const char* message, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

connectionData tryConnection(char* p_ip, char* port) {
    IPaddress ip;

    //char * to string
    std::string newStr = std::string(port);
    //string to int
    int newInt = stoi(newStr);
    //int to Uint16
    Uint16 newUint16 = static_cast<UINT16>(newInt);

    // Resolve host (ip name + port) into an IPaddress type
    if (SDLNet_ResolveHost(&ip, p_ip, newUint16) == -1) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        printf("No Server found.");
        return connectionData(false, SDLNet_GetError());       
    }

    //Open the connection to the server
    overallSocket = SDLNet_TCP_Open(&ip);

    if (!overallSocket) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return connectionData(false, SDLNet_GetError());
    }

    return connectionData(true, " ");
}

void load_connection_screen(SDL_Renderer* renderer, TTF_Font* font) 
{
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color green = { 0, 255, 0, 255 };
    SDL_Color red = { 255, 0 , 0, 255 };

    char ipBuffer[256] = "localhost"; 
    char portBuffer[10] = "55555";
    int ipLength = strlen(ipBuffer); //max 256
    int portLength = strlen(portBuffer); //max 10
    bool typingIP = true;  // Start by typing the IP
    bool hasErrored = false;
    std::string connectionString = " ";

    SDL_StartTextInput(); //allow text input from SDL

    bool quit = false;
    SDL_Event e;

    while (!quit) 
    {
        //Take input
        while (SDL_PollEvent(&e)) 
        {
            if (e.type == SDL_QUIT) {
                quit = true;
                break;
            }
            
            if (e.type == SDL_TEXTINPUT) {
                if (typingIP && ipLength < sizeof(ipBuffer) - 1) {
                    strcat(ipBuffer, e.text.text);
                    ipLength += strlen(e.text.text);
                }
                else if (!typingIP && portLength < sizeof(portBuffer) - 1) {
                    strcat(portBuffer, e.text.text);
                    portLength += strlen(e.text.text);
                }
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    if (typingIP && ipLength > 0) {
                        ipBuffer[--ipLength] = '\0';
                    }
                    else if (!typingIP && portLength > 0) {
                        portBuffer[--portLength] = '\0';
                    }
                }
                if (e.key.keysym.sym == SDLK_TAB) {
                    typingIP = !typingIP;
                }
                if (e.key.keysym.sym == SDLK_RETURN) {
                    // Attempt connection with entered IP
                    connectionData result = tryConnection(ipBuffer, portBuffer);
                    if (result.success) {                       
                        quit = true;
                        hasErrored = false;
                        connectionString = "Success! Connecting now...";
                        has_made_connection = true;
                    }
                    else {
                        hasErrored = true;
                        connectionString = result.message;
                        if (connectionString.empty())
                            connectionString = "No server found!";
                    }                        
                }
            }
        }

        // Render the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Display the IP and Port input fields
        SDL_Texture* ipLabel = renderText("IP Address ", font, white, renderer);
        SDL_Texture* ipText = renderText(ipBuffer, font, typingIP ? green : white, renderer);
        SDL_Texture* portLabel = renderText("Port ", font, white, renderer);
        SDL_Texture* portText = renderText(portBuffer, font, typingIP ? white : green, renderer);
        SDL_Texture* connectText = renderText("Press Enter to Connect", font, green, renderer);
        SDL_Texture* errorText = renderText(connectionString.c_str(), font, hasErrored ? red : green, renderer);

        //int widthAddress = 175;
        //int widthPort = 150;

        //Change width of text boxes based on input length
        int widthAddress = 0;
        int widthPort = 0;

        for (int i = 0; i < strlen(ipBuffer); i++) {
            widthAddress += 15;
        }

        for (int i = 0; i < strlen(portBuffer); i++) {
            widthPort += 20;
        }      

        SDL_Rect ipLabelRect = { 100, 100, 200, 50 };
        SDL_Rect ipTextRect = { 350, 100, widthAddress, 50 };
        SDL_Rect portLabelRect = { 100, 200, 100, 50 };
        SDL_Rect portTextRect = { 350, 200, widthPort, 50 };
        SDL_Rect connectRect = { 100, 300, 400, 50 };
        SDL_Rect errorRect = { 100, 400, 350, 60 };

        SDL_RenderCopy(renderer, ipLabel, NULL, &ipLabelRect);
        SDL_RenderCopy(renderer, ipText, NULL, &ipTextRect);
        SDL_RenderCopy(renderer, portLabel, NULL, &portLabelRect);
        SDL_RenderCopy(renderer, portText, NULL, &portTextRect);
        SDL_RenderCopy(renderer, connectText, NULL, &connectRect);
        SDL_RenderCopy(renderer, errorText, NULL, &errorRect);

        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(ipLabel);
        SDL_DestroyTexture(ipText);
        SDL_DestroyTexture(portLabel);
        SDL_DestroyTexture(portText);
        SDL_DestroyTexture(connectText);
        SDL_DestroyTexture(errorText);
    }

    SDL_StopTextInput();  // Stop text input  
}

void load_lobby(SDL_Renderer* renderer, TTF_Font* font) 
{
    std::cout << "in lobby";

    // Render the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Event e;
    boolean inLobby = true;

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color green = { 0, 255, 0, 255 };
    SDL_Color red = { 255, 0 , 0, 255 };
    SDL_Color brown = { 139, 69, 19, 255 };
   
    while (inLobby && !game_started)
    {
        game->HeartBeat();
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) {
                inLobby = false;
                break;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN)
                {
                    //Exit lobby if 2 or more connections. Only enabled for p1.
                    if ((numConnections >= 2 && game->getPlayerId() == 0) || gameActive)
                    {                       
                        if(!gameActive)
                            game->send("EXIT_LOBBY");

                        inLobby = false;
                        game_started = true;
                    }
                }
            }
        }

        //Game vars
        int numPlayers = numConnections;
        int numActivePlayers = 1;
        bool ready = false;
        if (numPlayers > 1)
            numActivePlayers = 2;
        int numSpectators = numPlayers - numActivePlayers;

        std::string startGameText;
        if (numPlayers >= 2 && game->getPlayerId() == 0)
            startGameText = "Ready to start. Press Enter to begin game!";
        else if (numPlayers >= 2)
            startGameText = "Ready to start. Waiting on player 1.";
        else
            startGameText = "Waiting for more players to begin...";
        
        ready = numPlayers >= 2;

        if (gameActive) {
            startGameText = "Game in progress. Press Enter to join.";
            ready = true;
        }

        // Render the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        //Create Textures
        SDL_Texture* lobbyLabel = renderText("Tankz Lobby:", font, brown, renderer);
        SDL_Texture* numPlayersLabel = renderText(("Number of players: " + std::to_string(numPlayers)).c_str(), font, white, renderer);
        SDL_Texture* numSpectatorsLabel = renderText(numSpectators > 0 ? ("Number of Spectators: " + std::to_string(numSpectators)).c_str() : "Number of Spectators: O", font, white, renderer);
        SDL_Texture* player1Label = renderText("Player 1", font, game->getPlayerId() == 0 ? green : white, renderer);
        SDL_Texture* player2Label = renderText(numPlayers > 1 ? "Player 2" : " ", font, game->getPlayerId() == 1 ? green : white, renderer);
        SDL_Texture* spectatorLabel = renderText(game->getPlayerId() == -1 ? "You are spectating" : " ", font, white, renderer);
        SDL_Texture* startGameLabel = renderText(startGameText.c_str(), font, ready ? green : red, renderer);

        //Create Rects
        SDL_Rect lobbyLabelRect =   { 100, 50, 200,  50 };
        SDL_Rect numPlayersRect =   { 100, 100, 250, 50 };
        SDL_Rect playerRect =       { 100, 150, 110, 50 };
        SDL_Rect player2Rect =      { 100, 200, 110, 50 };
        SDL_Rect numSpectatorRect = { 100, 250, 275, 50 };
        SDL_Rect spectatorRect =    {100, 300, 150, 50};
        SDL_Rect startGameRect =    { 100, 350, 400, 50 };

        //Render   
        SDL_RenderCopy(renderer, lobbyLabel, NULL, &lobbyLabelRect);
        SDL_RenderCopy(renderer, numPlayersLabel, NULL, &numPlayersRect);
        SDL_RenderCopy(renderer, player1Label, NULL, &playerRect);
        SDL_RenderCopy(renderer, player2Label, NULL, &player2Rect);
        SDL_RenderCopy(renderer, numSpectatorsLabel, NULL, &numSpectatorRect);
        SDL_RenderCopy(renderer, spectatorLabel, NULL, &spectatorRect);
        SDL_RenderCopy(renderer, startGameLabel, NULL, &startGameRect);
        SDL_RenderPresent(renderer);

        //Destroy (this is done to prevent memory leaks - textures will keep getting created and stored in RAM)
        SDL_DestroyTexture(lobbyLabel);
        SDL_DestroyTexture(numPlayersLabel);
        SDL_DestroyTexture(player1Label);
        SDL_DestroyTexture(player2Label);
        SDL_DestroyTexture(numSpectatorsLabel);
        SDL_DestroyTexture(spectatorLabel);
        SDL_DestroyTexture(startGameLabel);
    }
}


int main(int argc, char** argv) {

    // Initialize SDL
    if (SDL_Init(0) == -1) {
        printf("SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Initialize SDL_net
    if (SDLNet_Init() == -1) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }

    //Initialise SDL_ttf
    if (TTF_Init() == 1) {
        printf("SDL_ttf could not initialise! TTF_Error: %s\n", SDL_GetError());
        exit(3);
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Connection Screen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 500, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(4);
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(5);
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("../assets/fonts/Hey Comic.ttf", 25);
    if (!font) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        exit(6);
    }
   
    load_connection_screen(renderer, font);

    if (has_made_connection) 
    {       
        SDL_Thread* recvThread = SDL_CreateThread(on_receive, "ConnectionReceiveThread", (void*)overallSocket);
        SDL_Thread* sendThread = SDL_CreateThread(on_send, "ConnectionSendThread", (void*)overallSocket);

        SDL_SetWindowTitle(window, "Lobby");
        load_lobby(renderer, font);

        if (game_started) {
            SDL_DestroyWindow(window); //destroy screen for game screen
            SDL_DestroyRenderer(renderer); //destroy renderer as new one is created in run_game
            run_game();
        }

        is_running = false;
        int threadReturnValue;
        std::cout << "Waiting for threads to exit...";
        SDL_WaitThread(recvThread, &threadReturnValue);
        SDL_WaitThread(sendThread, &threadReturnValue);
    }
   
    delete game;
      
    // Close connection to the server
    SDLNet_TCP_Close(overallSocket);
    
    // Shutdown SDL_net
    SDLNet_Quit();

    // Shutdown SDL
    SDL_Quit();

    return 0;
}