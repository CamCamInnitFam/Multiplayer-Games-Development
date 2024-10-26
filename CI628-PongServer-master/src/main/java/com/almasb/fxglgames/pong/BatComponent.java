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

import com.almasb.fxgl.animation.AnimationBuilder;
import com.almasb.fxgl.dsl.FXGL;
import com.almasb.fxgl.entity.Entity;
import com.almasb.fxgl.entity.GameWorld;
import com.almasb.fxgl.entity.component.Component;
import com.almasb.fxgl.physics.PhysicsComponent;
import javafx.geometry.Point2D;

import java.awt.*;
import java.time.Duration;

/**
 * @author Almas Baimagambetov (AlmasB) (almaslvl@gmail.com)
 */
public class BatComponent extends Component {

    //private static final double BAT_SPEED = 420;
    private static final double TANK_MOVEMENT = 60; //was 40
    private boolean isColliding = false;

    protected PhysicsComponent physics;

    //for angle movement
    //physics.overwriteAngle(entity.getRotation() + 90);

    public void up() {
        for(Entity block : FXGL.getGameWorld().getEntitiesByType(EntityType.BLOCK)){

            if(entity.isColliding(block)){

                if(block.getBottomY() <= entity.getY()){

                    if(block.getX() == entity.getX())
                        isColliding = true;
                }
            }

            if(!isColliding)
                physics.overwritePosition(entity.getPosition().add(0, -TANK_MOVEMENT));

            isColliding = false;
        }
    }

    public void down() {
        for(Entity block : FXGL.getGameWorld().getEntitiesByType(EntityType.BLOCK)){
            if(entity.isColliding(block)){

                if(block.getY() <= entity.getBottomY())

                    if(entity.getX() == block.getX())
                        isColliding = true;
            }
        }
        if(!isColliding)
            physics.overwritePosition(entity.getPosition().add(0, TANK_MOVEMENT));

        isColliding = false;
    }

    public void right(){
        for(Entity block : FXGL.getGameWorld().getEntitiesByType(EntityType.BLOCK)) {
            if (entity.isColliding(block)){

                if(entity.getBottomY() <= block.getY())
                    isColliding = false;

                else if(entity.getY() >= block.getBottomY())
                    isColliding = false;

                else
                    if(block.getX() > entity.getX())
                        isColliding = true;
            }
        }

        if(!isColliding)
            physics.overwritePosition(entity.getPosition().add(TANK_MOVEMENT, 0));

        isColliding = false;
    }

    public void left(){
        for(Entity block : FXGL.getGameWorld().getEntitiesByType(EntityType.BLOCK)){
            if(entity.isColliding(block)){

                if(entity.getBottomY() <= block.getY())
                    isColliding = false;

                else if(entity.getY() >= block.getBottomY())
                    isColliding = false;

                else
                    if(block.getX() < entity.getX())
                        isColliding  = true;
            }
        }

        if(!isColliding)
            physics.overwritePosition(entity.getPosition().add(-TANK_MOVEMENT, 0));

        isColliding = false;

    }

    public void stop() {
        physics.setLinearVelocity(0, 0);
    }

    public void shoot()
    {
        //Spawn bullet?
        //any action related to shooting a bullet?
        //places player back a little and then forward to mimic recoil?

    }
}
