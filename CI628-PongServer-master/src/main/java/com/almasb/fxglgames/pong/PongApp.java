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
import com.almasb.fxgl.entity.component.Component;
import com.almasb.fxgl.input.UserAction;
import com.almasb.fxgl.net.*;
import com.almasb.fxgl.physics.CollisionHandler;
import com.almasb.fxgl.physics.HitBox;
import com.almasb.fxgl.physics.PhysicsComponent;
import com.almasb.fxgl.ui.UI;
import javafx.geometry.Point2D;
import javafx.scene.input.KeyCode;
import javafx.scene.input.MouseButton;
import javafx.scene.paint.Color;
import javafx.util.Duration;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Arrays;
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
        settings.setHeight(800);
    }

    private Entity player1;
    private Entity player2;
    private Entity bullet;
    private BatComponent player1Bat;
    private BarrelComponent p1barrelComponent;
    private BatComponent player2Bat;
    //private
    private Entity block1;
    private Entity block2;
    private Entity AIPlayer; //TODO

    private Server<String> server;

    int maxBounces = 5;
    int currentBounces = 0;
    int mousePosX = 0;
    int mousePosY = 0;

    //implement different bullet types???
    //more score for direct bullet hit?
    //less score for bounced bullet hit???
    //Two different entity types (ricochet bullet, non ricochet)
    //different key to fire these, or perhaps you have to reach a powerup and the next bullet is one that can ricochet?


    //perhaps the bullet (ball?) is always spawned in but is not visible
    //server sends information about bullet location and then a 1 or 0 depending on whether or not it is visible and therefore
    //should be drawn by the client's renderer
    //if we only send bullet pos when its active or visible, this minimised traffic though
    //perhaps talk about this in the report, what i chose to do and why?

    @Override
    protected void initInput() {
        getInput().addAction(new UserAction("Up1") {
            @Override
            protected void onActionBegin() {
                player1Bat.up();
            }

            @Override
            protected void onActionEnd() {
                player1Bat.stop();
            }
        }, KeyCode.W);

        getInput().addAction(new UserAction("Down1") {
            @Override
            protected void onActionBegin() {
                player1Bat.down();
            }

            @Override
            protected void onActionEnd() {
                player1Bat.stop();
            }
        }, KeyCode.S);

        getInput().addAction(new UserAction("Right1") {
            @Override
            protected void onActionBegin() {
                player1Bat.right();
            }

            @Override
            protected void onActionEnd() {
                player1Bat.stop();
            }
        }, KeyCode.D);

        getInput().addAction(new UserAction("Left1") {
            @Override
            protected void onActionBegin() {
                player1Bat.left();
            }

            @Override
            protected void onActionEnd() {
                player1Bat.stop();
            }
        }, KeyCode.A);

        getInput().addAction(new UserAction("Up2") {
            @Override
            protected void onActionBegin() {
                player2Bat.up();
            }

            @Override
            protected void onActionEnd() {
                player2Bat.stop();
            }
        }, KeyCode.I);

        getInput().addAction(new UserAction("Down2") {
            @Override
            protected void onActionBegin() {
                player2Bat.down();
            }

            @Override
            protected void onActionEnd() {
                player2Bat.stop();
            }
        }, KeyCode.K);

        getInput().addAction(new UserAction("Left2") {
            @Override
            protected void onActionBegin() {
                player2Bat.left();
            }

            @Override
            protected void onActionEnd() {
                player2Bat.stop();
            }
        }, KeyCode.J);

        getInput().addAction(new UserAction("Right2") {
            @Override
            protected void onActionBegin() {
                player2Bat.right();
            }

            @Override
            protected void onActionEnd() {
                player2Bat.stop();
            }
        }, KeyCode.L);

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
    }

    @Override
    protected void initGame() {
        Writers.INSTANCE.addTCPWriter(String.class, outputStream -> new MessageWriterS(outputStream));
        Readers.INSTANCE.addTCPReader(String.class, in -> new MessageReaderS(in));

        server = getNetService().newTCPServer(55555, new ServerConfig<>(String.class));

        server.setOnConnected(connection -> {
            connection.addMessageHandlerFX(this);
        });

        getGameWorld().addEntityFactory(new PongFactory());
        getGameScene().setBackgroundColor(Color.DARKGREEN);

        initScreenBounds();
        initGameObjects();

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
                if (boxB.getName().equals("LEFT")) {
                    server.broadcast(HIT_WALL_LEFT);
                } else if (boxB.getName().equals("RIGHT")) {
                    server.broadcast(HIT_WALL_RIGHT);
                } else if (boxB.getName().equals("TOP")) {
                    server.broadcast(HIT_WALL_UP);
                } else if (boxB.getName().equals("BOT")) {
                    server.broadcast(HIT_WALL_DOWN);
                }

                getGameScene().getViewport().shakeTranslational(5);

                currentBounces++;

                if(currentBounces >= maxBounces){
                    a.removeFromWorld();
                    currentBounces = 0;
                    server.broadcast("BULLET_DESPAWN");
                }
            }
        });

        CollisionHandler ballBatHandler = new CollisionHandler(EntityType.BULLET, EntityType.PLAYER_BAT) {
            @Override
            protected void onCollisionBegin(Entity a, Entity bat) {
                playHitAnimation(bat);
                a.removeFromWorld();
                server.broadcast("BULLET_DESPAWN");
                getGameScene().getViewport().shakeTranslational(10);

                if(bat == player1)
                    inc("player2score", +1);
                else
                    inc("player1score", +1);

                server.broadcast(bat == player1 ? BALL_HIT_BAT1 : BALL_HIT_BAT2);
                server.broadcast("SCORES," + geti("player1score") + "," + geti("player2score"));
            }

        };

        CollisionHandler bulletBlockHandler = new CollisionHandler(EntityType.BULLET, EntityType.BLOCK) {
            @Override
            protected void onCollisionBegin(Entity bullet, Entity block) {
                getGameScene().getViewport().shakeTranslational(8);
                currentBounces++;

                if(currentBounces >= maxBounces){
                    bullet.removeFromWorld();
                    currentBounces = 0;
                    server.broadcast("BULLET_DESPAWN");
                }
            }
        };

        getPhysicsWorld().addCollisionHandler(ballBatHandler);
        getPhysicsWorld().addCollisionHandler(ballBatHandler.copyFor(EntityType.BULLET, EntityType.ENEMY_BAT));
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
            //Send bullet data when there is a bullet active
            String message;
            if(bullet != null && bullet.isActive())
                message = "GAME_DATA," + player1.getY() + "," + player1.getX() + "," + player2.getY() + "," + player2.getX() + "," + bullet.getX() + "," + bullet.getY();
            else
                message = "GAME_DATA," + player1.getY() + "," + player1.getX() + "," + player2.getY() + "," + player2.getX();

            server.broadcast(message);
        }

        p1barrelComponent.rotateBarrel(mousePosX, mousePosY);
    }

    private void initScreenBounds() {
        Entity walls = entityBuilder()
                .type(EntityType.WALL)
                .collidable()
                .buildScreenBounds(150);

        getGameWorld().addEntity(walls);
    }

    private void initGameObjects() {
       // bullet = spawn("bullet", getAppWidth() / 2 - 5, getAppHeight() / 2 - 5);
        //could spawn bullet but make this invisible or inactive, then when the player clicks, it is made visible and velocity is put on it and position etc...
        player1 = spawn("tank", new SpawnData(getAppWidth() / 4, getAppHeight() / 2).put("isPlayer", true));
        player2 = spawn("tank", new SpawnData(3 * getAppWidth() / 4, getAppHeight() / 2).put("isPlayer", false));

        player1Bat = player1.getComponent(BatComponent.class);
        p1barrelComponent = player1.getComponent(BarrelComponent.class);
        player2Bat = player2.getComponent(BatComponent.class);

        block1 = spawn("block", new SpawnData(600, 340));
        block2 = spawn("block", new SpawnData(420, 220));
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
    /*public void playMovementAnimation(Entity tank, String direction){
        int x = 0;
        int y = 0;
        if(direction == "UP")
            y = -60;
        else if(direction == "DOWN")
            y = 60;
        else if(direction == "RIGHT")
            x = 60;
        else
            x = -60;


        animationBuilder()
                .duration(new Duration(1))
                .interpolator(Interpolators.LINEAR.EASE_OUT())
                .translate(tank)
                .from(tank.getPosition())
                .to(tank.getPosition().add(x, y))
                .buildAndPlay();
    }*/

    private void spawnBullet()
    {
        //ensure all bullets are removed before spawning another one
        /*for(Entity bullet : getGameWorld().getEntitiesByType(EntityType.BULLET))
        {
            bullet.removeFromWorld();
        }*/

        //getPhysicsWorld().onEntityRemoved(bullet) THEN activate new turn??? //TODO

        //must only ever be one bullet
        if(bullet != null)
            if(bullet.isActive())
                return;

        double middlePosX = player1.getCenter().getX();
        double middlePosY = player1.getCenter().getY();
        double spawnOffset = 55;

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
            server.broadcast("BULLET_SPAWN");

            //set velocity
            double bulletSpeed = bullet.getComponent(BallComponent.class).getSpeed();
            bullet.getComponent(PhysicsComponent.class).setLinearVelocity(directionX * bulletSpeed, directionY * bulletSpeed);
        }
    }

    @Override
    public void onReceive(Connection<String> connection, String message) {
        var tokens = message.split(",");
        //WILL HAVE TO DEAL WITH IDs  ?

        Arrays.stream(tokens).skip(1).forEach(key -> {
            //MOUSE POS = MP
            if(key.startsWith("MP"))
            {
                String coordinates = key.substring(3, key.length()-1);
                if(coordinates.split("\\.").length ==2)
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
                if (key.endsWith("_DOWN")) {
                    getInput().mockKeyPress(KeyCode.valueOf(key.substring(0, 1)));
                } else if (key.endsWith("_UP")) {
                    getInput().mockKeyRelease(KeyCode.valueOf(key.substring(0, 1)));
                }
            }
        });
    }

    static class MessageWriterS implements TCPMessageWriter<String> {

        private OutputStream os;
        private PrintWriter out;

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

        private BlockingQueue<String> messages = new ArrayBlockingQueue<>(50);

        private InputStreamReader in;

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
