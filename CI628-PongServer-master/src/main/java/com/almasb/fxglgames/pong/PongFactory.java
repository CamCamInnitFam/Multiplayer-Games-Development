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

import com.almasb.fxgl.entity.Entity;
import com.almasb.fxgl.entity.EntityFactory;
import com.almasb.fxgl.entity.SpawnData;
import com.almasb.fxgl.entity.Spawns;
import com.almasb.fxgl.entity.components.CollidableComponent;
import com.almasb.fxgl.physics.BoundingShape;
import com.almasb.fxgl.physics.HitBox;
import com.almasb.fxgl.physics.PhysicsComponent;
import com.almasb.fxgl.physics.box2d.dynamics.BodyType;
import com.almasb.fxgl.physics.box2d.dynamics.FixtureDef;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.Circle;

import static com.almasb.fxgl.dsl.FXGL.*;

/**
 * @author Almas Baimagambetov (AlmasB) (almaslvl@gmail.com)
 */
public class PongFactory implements EntityFactory {

    @Spawns("bullet")
    public Entity newBall(SpawnData data) {
        PhysicsComponent physics = new PhysicsComponent();
        physics.setBodyType(BodyType.DYNAMIC);
        physics.setFixtureDef(new FixtureDef().density(0.3f).restitution(1.0f));
        physics.setOnPhysicsInitialized(() -> physics.setLinearVelocity(5 * 60, -5 * 60));

        var endGame = getip("player1score").isEqualTo(10).or(getip("player2score").isEqualTo(10));
        return entityBuilder(data)
                .type(EntityType.BULLET)
                .bbox(new HitBox(BoundingShape.circle(5)))
                .view(new Circle(5,5,5, Color.RED))
                .with(physics)
                .with(new CollidableComponent(true))
                .with(new BulletComponent())
                .build();
    }

    @Spawns("tank")
    public Entity newBat(SpawnData data) {
        boolean isPlayer = data.get("isPlayer");

        PhysicsComponent physics = new PhysicsComponent();
        physics.setBodyType(BodyType.KINEMATIC);

        return entityBuilder(data)
                .type(isPlayer ? EntityType.PLAYER_BAT : EntityType.ENEMY_BAT)
                .viewWithBBox("testTank2.png")
                .with(new CollidableComponent(true))
                .with(physics)
                .with(new TankComponent())
                .with(new BarrelComponent(texture("barrel.png")))
                .rotate(isPlayer ? 0 : 180)
                .build();
    }

    @Spawns("block")
    public Entity newBlock(SpawnData data){
        PhysicsComponent physics = new PhysicsComponent();
        physics.setBodyType(BodyType.STATIC);

        return entityBuilder(data)
                .type(EntityType.BLOCK)
                .viewWithBBox(new Rectangle(60, 120, Color.SADDLEBROWN))
                .with(new CollidableComponent(true))
                .with(physics)
                .build();
    }
}
