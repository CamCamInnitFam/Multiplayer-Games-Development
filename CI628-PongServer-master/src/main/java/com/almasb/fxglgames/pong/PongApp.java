/*
 * The MIT License (MIT)
 *
 * FXGL - JavaFX Game Library
 *
 * Copyright (c) 2015-2017 AlmasB (almaslvl@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package com.almasb.fxglgames.pong;

import com.almasb.fxgl.animation.Interpolators;
import com.almasb.fxgl.app.ApplicationMode;
import com.almasb.fxgl.app.GameApplication;
import com.almasb.fxgl.app.GameSettings;
import com.almasb.fxgl.core.math.FXGLMath;
import com.almasb.fxgl.entity.Entity;
import com.almasb.fxgl.entity.SpawnData;
import com.almasb.fxgl.input.UserAction;
import com.almasb.fxgl.net.*;
import com.almasb.fxgl.physics.CollisionHandler;
import com.almasb.fxgl.physics.HitBox;
import com.almasb.fxgl.physics.PhysicsComponent;
import com.almasb.fxgl.ui.UI;
import javafx.scene.input.MouseButton;
import javafx.scene.paint.Color;
import javafx.util.Duration;

import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import static com.almasb.fxgl.dsl.FXGL.*;
import static com.almasb.fxglgames.pong.NetworkMessages.*;

/**
 * A simple clone of Pong.
 * Sounds from https://freesound.org/people/NoiseCollector/sounds/4391/ under CC BY 3.0.
 *
 * @author Almas Baimagambetov (AlmasB) (almaslvl@gmail.com)
 */
public class PongApp extends GameApplication implements MessageHandler<String> {

    @Override
    protected void initSettings(GameSettings settings) {
        settings.setTitle("Tankz");
        settings.setVersion("1.0");
        settings.setFontUI("inkTank.ttf");
        settings.setApplicationMode(ApplicationMode.DEBUG);
        settings.setWidth(1200);
        settings.setHeight(840);
    }

    private Entity player1;
    private Entity player2;
    private Entity bullet;
    private BatComponent player1Bat;
    private BarrelComponent p1barrelComponent;
    private BatComponent player2Bat;
    private BarrelComponent p2barrelComponent;
    private List<Entity> Players;
    private Entity block1;
    private Entity block2;
    private Entity block3;
    private Entity block4;
    private Entity block5;
    private Entity AIPlayer; //TODO
    private Server<String> server;

    //TODO in init game vars
    int mousePosX = 0;
    int mousePosY = 0;
    int activeTurn = 0; //0 or 1, -1 for game over?
    boolean gameOver = false;
    boolean isInLobby = true;

    @Override
    protected void initInput() {
        getInput().addAction(new UserAction("Shoot")
        {
            @Override
            protected void onActionBegin() {spawnBullet();}
        }, MouseButton.PRIMARY);
    }

    @Override
    protected void initGameVars(Map<String, Object> vars) {
        vars.put("player1score", 0);
        vars.put("player2score", 0);
        vars.put("numClientsConnected", 0);
    }


    @Override
    protected void initGame() {
        Writers.INSTANCE.addTCPWriter(String.class, outputStream -> new MessageWriterS(outputStream));
        Readers.INSTANCE.addTCPReader(String.class, in -> new MessageReaderS(in));
        Players = new ArrayList<>();

        //Set up world before server?

        getGameWorld().addEntityFactory(new PongFactory());
        getGameScene().setBackgroundColor(Color.DARKGREEN);

        initGameObjects();
        initScreenBounds();

        server = getNetService().newTCPServer(55555, new ServerConfig<>(String.class));

        server.setOnConnected(connection -> { //when a client connects
            boolean hasSetId = false;
            System.out.println("Connection Found.");

            //loop through all player Entities and check if connected
            //if not - assign id to client and entity
            for(int i = 0; i < Players.size(); i++){
                if(!Players.get(i).getComponent(BatComponent.class).connected){
                    connection.getLocalSessionData().setValue("ID", i);
                    connection.getLocalSessionData().setValue("HeartBeatTime", System.currentTimeMillis() + 2000);
                    connection.getLocalSessionData().setValue("Connected", true);
                    Players.get(i).getComponent(BatComponent.class).connected = true;
                    Players.get(i).getComponent(BatComponent.class).id = i;
                    hasSetId = true;
                    System.out.println("Player Connected!");
                    break;
                }
            }

            //make extras -1 (they can spectate) they are still connected
            if(!hasSetId){
                connection.getLocalSessionData().setValue("ID", -1);
                connection.getLocalSessionData().setValue("Connected", true);
                connection.getLocalSessionData().setValue("HeartBeatTime", System.currentTimeMillis() + 2000);
                System.out.println("Now Spectating...");
            }

            //Client is sent its own ID for it to handle
            //This will usually be 0 (p1), 1 (p2) or -1 (spectator).
            inc("numClientsConnected", +1);
            connection.addMessageHandlerFX(this);
            connection.send("ID," + connection.getLocalSessionData().getValue("ID"));

            //Use runOnce to send another message to the client
            runOnce(() -> {
                connection.send("CONNECTEVENT," + geti("numClientsConnected"));
            }, Duration.seconds(1));

            server.broadcast("CONNECTEVENT," + geti("numClientsConnected"));

            if(!isInLobby)
            {
                runOnce(() -> {
                   connection.send("GAMESTATE," + "Playing");
                }, Duration.seconds(1));
            }

        });

        server.setOnDisconnected(connection -> {
            System.out.println("CLIENT DISCONNECT");
            inc("numClientsConnected", -1);
            connection.send("CONNECTEVENT," + geti("numClientsConnected"));
            server.broadcast("CONNECTEVENT," + geti("numClientsConnected"));
            int connectionID = connection.getLocalSessionData().getValue("ID");

            if(connectionID == -1)
                return;

            for(Entity player : Players){
                if(player.getComponent(BatComponent.class).id == connectionID){
                    player.getComponent(BatComponent.class).connected = false;
                    System.out.println("Player Disconnected");
                    break;
                }
            }

        });


        var t = new Thread(server.startTask()::run);
        t.setDaemon(true);
        t.start();
    }

    @Override
    protected void initPhysics() {
        getPhysicsWorld().setGravity(0, 0);

        getPhysicsWorld().addCollisionHandler(new CollisionHandler(EntityType.BULLET, EntityType.WALL) {
            @Override
            protected void onHitBoxTrigger(Entity a, Entity b, HitBox boxA, HitBox boxB) {
                getGameScene().getViewport().shakeTranslational(5);

                a.getComponent(BallComponent.class).currentBounces ++;

                if(a.getComponent(BallComponent.class).currentBounces >= a.getComponent(BallComponent.class).maxBounces)
                    server.broadcast(BULLET_DESPAWN);
            }
        });

        CollisionHandler bulletTankHandler = new CollisionHandler(EntityType.BULLET, EntityType.PLAYER_BAT) {
            @Override
            protected void onCollisionBegin(Entity a, Entity bat) {
                playHitAnimation(bat);
                a.removeFromWorld();
                server.broadcast(BULLET_DESPAWN);
                getGameScene().getViewport().shakeTranslational(10);

                if(bat == player1)
                    inc("player2score", +1);
                else
                    inc("player1score", +1);

                //server.broadcast(bat == player1 ? BALL_HIT_BAT1 : BALL_HIT_BAT2);
                server.broadcast("SCORES," + geti("player1score") + "," + geti("player2score"));
            }

        };

        CollisionHandler bulletBlockHandler = new CollisionHandler(EntityType.BULLET, EntityType.BLOCK) {
            @Override
            protected void onCollisionBegin(Entity bullet, Entity block) {
                getGameScene().getViewport().shakeTranslational(8);

                bullet.getComponent(BallComponent.class).currentBounces ++;

                if(bullet.getComponent(BallComponent.class).currentBounces >= bullet.getComponent(BallComponent.class).maxBounces)
                    server.broadcast(BULLET_DESPAWN);
            }
        };

        getPhysicsWorld().addCollisionHandler(bulletTankHandler);
        getPhysicsWorld().addCollisionHandler(bulletTankHandler.copyFor(EntityType.BULLET, EntityType.ENEMY_BAT));
        getPhysicsWorld().addCollisionHandler((bulletBlockHandler));
    }

    @Override
    protected void initUI() {
        MainUIController controller = new MainUIController();
        UI ui = getAssetLoader().loadUI("main.fxml", controller);

        controller.getLabelScorePlayer().textProperty().bind(getip("player1score").asString());
        controller.getLabelScoreEnemy().textProperty().bind(getip("player2score").asString());

        getGameScene().addUI(ui);
    }

    @Override
    protected void onUpdate(double tpf) {
        if (!server.getConnections().isEmpty())
        {
            if(isInLobby){
                processLobbyInformation();
                return;
            }

            //return back to lobby if clients disconnect
            if(geti("numClientsConnected") <1)
                resetGame();

            //Send bullet data when there is a bullet active
            String message;

            //CHECK IF POS HAVE CHANGED FIRST
            //POTENTIALLY STOP BROADCASTING - ONLY SEND CLIENT 1 INFORMATION ABOUT CLIENT 2 ETC? ONLY WHEN CLIENT KNOWS ITS OWN POSITION (which it can calculate easily with +- 40).

            //TODO always send velocity
            if(bullet != null && bullet.isActive())
                message = "GAME_DATA," + player1.getY() + "," + player1.getX() + "," + player2.getY() + "," + player2.getX() + "," + bullet.getX() + "," + bullet.getY();
            else
                message = "GAME_DATA," + player1.getY() + "," + player1.getX() + "," + player2.getY() + "," + player2.getX();

            server.broadcast(message);

            //TODO list of barrel components, then do barrels[activeTurn].rotateBarrel()
            //This will allow for multiple tanks at once
            switch(activeTurn){
                case 0:
                    p1barrelComponent.rotateBarrel(mousePosX, mousePosY, 0);
                    break;
                case 1:
                    p2barrelComponent.rotateBarrel(mousePosX, mousePosY, 1);
                    break;
                default:
                    break;
            }

            //TODO replace with heartbeat function
            //For all client connections (active and inactive)
            for(Connection connection: server.getConnections())
            {
                //Checks
                if(!connection.isConnected())
                    continue;

                if(!(boolean)connection.getLocalSessionData().getValue("Connected"))
                    continue;

                //Setup
                long lastHeartBeatTime = connection.getLocalSessionData().getValue("HeartBeatTime");

                //Check last Signal Time from client
                if(System.currentTimeMillis() > lastHeartBeatTime + 3000){
                    //connection.terminate();
                    int connectionID = connection.getLocalSessionData().getValue("ID");
                    System.out.println("Client " + connectionID + " disconnected!");
                    connection.getLocalSessionData().setValue("Connected", false);
                    inc("numClientsConnected", -1);

                    //If not a spectator, make linked player disconnect
                    if(connectionID != -1)
                        Players.get(connectionID).getComponent(BatComponent.class).connected = false;
                }
            }

            //Add spectators to game if one of the main clients disconnect
            for(int i = 0; i < Players.size(); i++)
            {
                if(!Players.get(i).getComponent(BatComponent.class).connected)
                {
                    for(Connection connection : server.getConnections())
                    {
                        int id = connection.getLocalSessionData().getValue("ID");
                        if((boolean)connection.getLocalSessionData().getValue("Connected")  && id == -1)
                        {
                            connection.getLocalSessionData().setValue("ID", i);
                            connection.send("ID," + i);
                            Players.get(i).getComponent(BatComponent.class).connected = true;
                        }
                    }
                }
            }
        }

     if(server.getConnections().size() < 2){
            //add AI?
            //TODO
        }
    }

    private void initScreenBounds() {
        Entity walls = entityBuilder()
                .type(EntityType.WALL)
                .collidable()
                .buildScreenBounds(150);

        getGameWorld().addEntity(walls);
    }

    private void initGameObjects() {
        //could spawn bullet but make this invisible or inactive, then when the player clicks, it is made visible and velocity is put on it and position etc...
        //for int i = 0; i < playerCount ; spawn("tank") if i == .put etc... max is 4?
        player1 = spawn("tank", new SpawnData(getAppWidth() / 4 - 120, getAppHeight() / 2 +20).put("isPlayer", true));
        player2 = spawn("tank", new SpawnData(3 * getAppWidth() / 4 + 60, getAppHeight() / 2 +20).put("isPlayer", false));
        Players.add(player1);
        Players.add(player2);

        player1Bat = player1.getComponent(BatComponent.class);
        p1barrelComponent = player1.getComponent(BarrelComponent.class);
        player2Bat = player2.getComponent(BatComponent.class);
        p2barrelComponent = player2.getComponent(BarrelComponent.class);

        block1 = spawn("block", new SpawnData(600, 380));
        block2 = spawn("block", new SpawnData(420, 80));
        block3 = spawn("block", new SpawnData(780, 80));
        block4 = spawn("block", new SpawnData(420, 620));
        block5 = spawn("block", new SpawnData(780, 620));
    }

    private void playHitAnimation(Entity bat) {
        animationBuilder()
                .autoReverse(true)
                .duration(Duration.seconds(0.5))
                .interpolator(Interpolators.BOUNCE.EASE_OUT())
                .rotate(bat)
                .from(FXGLMath.random(-25, 25))
                .to(0)
                .buildAndPlay();
    }

    private void processLobbyInformation()
    {
        checkHeartBeats();
        swapPlayer1();
        spectatorsToPlayers();
    }

    private void spectatorsToPlayers()
    {
        for(int i = 0; i < Players.size(); i++)
        {
            if(!Players.get(i).getComponent(BatComponent.class).connected)
            {
                for(Connection connection : server.getConnections())
                {
                    int id = connection.getLocalSessionData().getValue("ID");
                    if((boolean)connection.getLocalSessionData().getValue("Connected")  && id == -1)
                    {
                        connection.getLocalSessionData().setValue("ID", i);
                        connection.send("ID," + i);
                        Players.get(i).getComponent(BatComponent.class).connected = true;
                    }
                }
            }
        }
    }

    private void swapPlayer1()
    {
        if (server.getConnections().isEmpty())
            return;

        for(Entity player : Players)
        {
            BatComponent batcomponent = player.getComponent(BatComponent.class);

            //P1 connected, not interested in swapping
            if (batcomponent.connected  && batcomponent.id == 0)
                return;

            //If spectator, skip
            if(batcomponent.id == -1)
                continue;

            //if player 2 is connected, proceed with swapping
            if(batcomponent.connected && batcomponent.id == 1)
            {
                //Swap player 1 for player 2
                batcomponent.connected = false;
                Players.get(0).getComponent(BatComponent.class).connected = true;
                for(Connection connection : server.getConnections())
                {
                    if((int)connection.getLocalSessionData().getValue("ID") == 1)//p2
                    {
                        connection.getLocalSessionData().setValue("ID", 0);
                        connection.send("ID," + 0);
                    }
                }
            }
        }
    }

    private void checkHeartBeats()
    {
        if (server.getConnections().isEmpty())
            return;

        //For all client connections (active and inactive)
        for(Connection connection: server.getConnections())
        {
            //Checks
            if(!connection.isConnected())
                continue;

            if(!(boolean)connection.getLocalSessionData().getValue("Connected"))
                continue;

            //Setup
            long lastHeartBeatTime = connection.getLocalSessionData().getValue("HeartBeatTime");

            //Check last Signal Time from client
            if(System.currentTimeMillis() > lastHeartBeatTime + 4000){
                //connection.terminate();
                int connectionID = connection.getLocalSessionData().getValue("ID");
                System.out.println("Client " + connectionID + " disconnected!");
                connection.getLocalSessionData().setValue("Connected", false);
                inc("numClientsConnected", -1);
                server.broadcast("CONNECTEVENT," + geti("numClientsConnected"));

                //If not a spectator, make linked player disconnect
                if(connectionID != -1)
                    Players.get(connectionID).getComponent(BatComponent.class).connected = false;

            }
        }
    }

    private void resetGame(){
        System.out.println("Resetting game...");
        isInLobby = true;
        if(bullet != null)
            bullet.removeFromWorld();
        player1.getComponent(BatComponent.class).reset();
        player2.getComponent(BatComponent.class).reset();
        p1barrelComponent.resetRotation(0);
        p2barrelComponent.resetRotation(180);
        activeTurn = 0;
        set("player1score", 0);
        set("player2score", 0);
    }

    private void spawnBullet()
    {
        //ensure all bullets are removed before spawning another one
        /*for(Entity bullet : getGameWorld().getEntitiesByType(EntityType.BULLET))
        {
            bullet.removeFromWorld();
        }*/

        //getPhysicsWorld().onEntityRemoved(bullet) THEN activate new turn??? //TODO

        double middlePosX;
        double middlePosY;
        double spawnOffset = 55;

        //must only ever be one bullet
        if(bullet != null)
            if(bullet.isActive())
                return;

        switch(activeTurn)
        {
            case(0):
                middlePosX = player1.getCenter().getX();
                middlePosY = player1.getCenter().getY();
                break;

            case(1):
                middlePosX = player2.getCenter().getX();
                middlePosY = player2.getCenter().getY();
                break;

            default:
                middlePosY = 0;
                middlePosX  = 0;
                break;
        }

        //get positions
        double deltaX = mousePosX - middlePosX;
        double deltaY = mousePosY - middlePosY;

        //normalise
        double length = Math.sqrt(deltaX * deltaX + deltaY * deltaY);

        //check
        if(length >0)
        {
            double directionX = deltaX / length;
            double directionY = deltaY / length;

            //spawn bullet
            bullet = spawn("bullet", new SpawnData(middlePosX + directionX * spawnOffset, middlePosY + directionY * spawnOffset));

            //set velocity
            double bulletSpeed = bullet.getComponent(BallComponent.class).getSpeed();
            bullet.getComponent(PhysicsComponent.class).setLinearVelocity(directionX * bulletSpeed, directionY * bulletSpeed);
            server.broadcast("BULLET_SPAWN," + bullet.getComponent(PhysicsComponent.class).getVelocityX() + "," + bullet.getComponent(PhysicsComponent.class).getVelocityY() + "," + bullet.getX() + "," + bullet.getY());
        }
    }

    @Override
    public void onReceive(Connection<String> connection, String message) {

        var tokens = message.split(",");
        int connectionID = connection.getLocalSessionData().getValue("ID");

        Arrays.stream(tokens).skip(1).forEach(key -> {

            //Receive heartbeat to establish active connection
            if(key.equals("heartbeat"))
                connection.getLocalSessionData().setValue("HeartBeatTime", System.currentTimeMillis());

            //Exit lobby (start main game)
            if(key.equals("EXIT_LOBBY"))
            {
                isInLobby = false;
                server.broadcast("GAME_START");
            }


            //Switch Active Turn
            if(key.equals("TURN_END"))
            {
                switch(connectionID)
                {
                    case(0):
                        activeTurn = 1;
                        for(Connection connect : server.getConnections())
                        {
                            if((boolean)connect.getLocalSessionData().getValue("Connected") && (int)connect.getLocalSessionData().getValue("ID") == 1)
                                connect.send("*"); //TURN START
                        }
                        break;
                    case(1):
                        activeTurn = 0;
                        for(Connection connect : server.getConnections())
                        {
                            if((boolean)connect.getLocalSessionData().getValue("Connected") && (int)connect.getLocalSessionData().getValue("ID") == 0)
                                connect.send("*"); //TURN START
                        }
                        break;
                }
            }

            //MOUSE POS = MP
            //SHOULD ONLY RECIEVE FROM CLIENT WHEN IT IS THEIR TURN...
            if(key.startsWith("MP"))
            {
                String coordinates = "0.0";
                try{
                    coordinates = key.substring(3, key.length()-1);
                }
                catch(Exception e){
                    System.out.println("errored!");
                }

                if(coordinates.split("\\.").length == 2)
                {
                    mousePosX = Integer.parseInt(coordinates.split("\\.")[0]);
                    mousePosY = Integer.parseInt(coordinates.split("\\.")[1]);
                }
            }

            //INPUT

            //left mouse button
            if(key.startsWith("LMB")){
                if(key.endsWith("_DOWN"))
                    getInput().mockButtonPress(MouseButton.PRIMARY);

                if(key.endsWith("_UP"))
                    getInput().mockButtonRelease(MouseButton.PRIMARY);

            }else{
                //keyboard
                if (key.endsWith("_DOWN"))
                //TODO could do up() down() left() and right() and pass in the connection. Then check connection
                    //here and provide p1bat or p2bat (tank) movement
                //TODO change player1Bat to be player1Tank
                {
                    switch(key.substring(0, 1)){
                        case("W"):
                            if(connectionID == 0)
                                player1Bat.up();
                            else if(connectionID == 1)
                                player2Bat.up();
                            break;
                        case("A"):
                            if(connectionID == 0)
                                player1Bat.left();
                            else if(connectionID == 1)
                                player2Bat.left();
                            break;
                        case("S"):
                            if(connectionID == 0)
                                player1Bat.down();
                            else if(connectionID == 1)
                                player2Bat.down();
                            break;
                        case("D"):
                            if(connectionID == 0)
                                player1Bat.right();
                            else if(connectionID == 1)
                                player2Bat.right();
                            break;
                    }
                }
            }
        });
    }

    static class MessageWriterS implements TCPMessageWriter<String> {

        private final OutputStream os;
        private final PrintWriter out;

        MessageWriterS(OutputStream os) {
            this.os = os;
            out = new PrintWriter(os, true);
        }

        @Override
        public void write(String s) throws Exception {
            out.print(s.toCharArray());
            out.flush();
        }
    }

    static class MessageReaderS implements TCPMessageReader<String> {

        private final BlockingQueue<String> messages = new ArrayBlockingQueue<>(50);

        private final InputStreamReader in;

        MessageReaderS(InputStream is) {
            in =  new InputStreamReader(is);

            var t = new Thread(() -> {
                try {

                    char[] buf = new char[36];

                    int len;

                    while ((len = in.read(buf)) > 0) {
                        var message = new String(Arrays.copyOf(buf, len));

                        System.out.println("Recv message: " + message);

                        messages.put(message);
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                }
            });

            t.setDaemon(true);
            t.start();
        }

        @Override
        public String read() throws Exception {
            return messages.take();
        }
    }

    public static void main(String[] args) {
        launch(args);
    }
}
