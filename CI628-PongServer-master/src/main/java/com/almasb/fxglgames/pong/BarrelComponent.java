package com.almasb.fxglgames.pong;

import com.almasb.fxgl.dsl.FXGL;
import com.almasb.fxgl.entity.component.Component;
import javafx.geometry.Point2D;
import javafx.scene.image.ImageView;

import java.awt.event.MouseEvent;


public class BarrelComponent extends Component
{
    private final ImageView barrelView;
    public BarrelComponent(ImageView barrelTexture)
    {
        this.barrelView = barrelTexture;
    }

    @Override
    public void onAdded(){
        entity.getViewComponent().addChild(barrelView);

        barrelView.setTranslateX(5);
        barrelView.setTranslateY(15);
    }

    public void rotateBarrel(int x, int y){
        double angle = Math.toDegrees(Math.atan2(y - entity.getCenter().getY(), x - entity.getCenter().getX() ));
        barrelView.setRotate(angle);
    }
}
