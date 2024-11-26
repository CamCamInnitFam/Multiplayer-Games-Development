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

import static com.almasb.fxgl.dsl.FXGL.getAppHeight;
import static com.almasb.fxgl.dsl.FXGL.getAppWidth;

/**
 * @author Almas Baimagambetov (AlmasB) (almaslvl@gmail.com)
 */
public class BatComponent extends Component {

    private static final double TANK_MOVEMENT = 60; //was 40
    private boolean isColliding = false;
    public int id = -1;
    public boolean connected = false;

    protected PhysicsComponent physics;

    //for angle movement
    //physics.overwriteAngle(entity.getRotation() + 90);

    public void up() {
        if(entity.getY() <= 0)
            return;

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
        if(entity.getBottomY() >= 840)
            return;

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
        if(entity.getRightX() >= 1200)
            return;

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
        if(entity.getX() <= 0)
            return;

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


    public void reset(){

        //Reset Positions
        Point2D newpoint = new Point2D(0,0);
        switch(id){
            case(0):
                newpoint = new Point2D((FXGL.getAppWidth() / 4 - 120),(FXGL.getAppHeight() / 2 +20));
                physics.overwritePosition(newpoint);
                //physics.overwriteAngle(0);
                break;
            case(1):
                newpoint = new Point2D((3 * FXGL.getAppWidth() / 4 + 80), (FXGL.getAppHeight() / 2 +20));
                //physics.overwriteAngle(0);
                break;
        }
        physics.overwritePosition(newpoint);

        //Reset ID & connected
        id = -1;
        connected = false;
    }

}
