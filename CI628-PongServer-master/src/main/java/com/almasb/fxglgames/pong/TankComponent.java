package com.almasb.fxglgames.pong;

import com.almasb.fxgl.entity.component.Component;
import com.almasb.fxgl.dsl.FXGL;
import com.almasb.fxgl.physics.PhysicsComponent;

public class TankComponent extends Component{
    private static final double TANK_MOVEMENT = 40;

    protected PhysicsComponent physics;

    public void up() {
        physics.overwritePosition(entity.getPosition().add(0, -TANK_MOVEMENT));
    }

    public void down() {
        physics.overwritePosition(entity.getPosition().add(0, TANK_MOVEMENT));
    }

    public void right(){

    }

    public void left(){

    }

    public void stop() {
        physics.setLinearVelocity(0, 0);
    }


}
