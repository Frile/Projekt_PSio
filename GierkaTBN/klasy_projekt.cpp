#pragma once
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "klasy_projekt.h"
#include <cmath>
#include <vector>

//float distance(sf::Vector2f,sf::Vector2f);
//float distance(float,float,float,float);
float toRad(float);
float toDeg(float);

enum class GameSettings{
    Rows,Orbit,Free/*,Easy,Medium,Hard,Fast,Slow*/
};

class Dot: public sf::CircleShape{
public:
    Dot(const sf::Vector2f &pos):CircleShape(4){
        setFillColor(sf::Color::Red);
        setPosition(pos);
        setOrigin(4,4);
    }
    Dot(float x, float y):CircleShape(4){
        setFillColor(sf::Color::Red);
        setPosition(x,y);
        setOrigin(4,4);
    }
};

class Bullet: public sf::CircleShape{
    int power_;
    int speed_;
    float direction_;
    float max_distance_;
    float current_distance_=0;
public:
    Bullet(float direction ,const sf::Vector2f& pos,int speed,int power=1,float max_distance=400): CircleShape(10){
        direction_=direction;
        setPosition(pos);
        setOrigin(10,10);
        power_=power;
        speed_=speed;
        max_distance_=max_distance;
    }
    bool overdue=false;                             //ez
    void setSpeed(int speed){speed_=speed;}         //ez
    void update(sf::Time& time){                    //ez
        float temp=speed_*time.asSeconds();
        //std::cout<<direction_<<"sranie"<<std::endl;
        move(cosf(toRad(direction_))*temp,-sinf(toRad(direction_))*temp);  //ez
        current_distance_+=temp;
        if (current_distance_>=max_distance_){
            terminate();
        }
    }
    void terminate(){overdue=true;}
};

class BulletList{
public:
    BulletList(sf::FloatRect &obszar):region(obszar){};
    std::vector<Bullet*> bullets;
    sf::FloatRect region;
    void addBullet(Bullet* new_bullet){     //ok
        bullets.emplace_back(new_bullet);
    }
    void removeBullet(Bullet* bullet){      //ok
        for (auto it=bullets.begin();it!=bullets.end();it++){
            if (*it==bullet){
                delete bullet;
                bullets.erase(it);
                break;
            }
        }

    }
    int bulletCount(){return bullets.size();}   //ez
    void update(sf::Time& time){            //ok
        for (auto& x:bullets){
            x->update(time);
            if(x->overdue) {removeBullet(x);}
        }
    }
};

/*class ObstacleList{
    static std::vector<Obstacle*> obstacles;
    void addObstacle(Obstacle* new_obstacle){     //ok
        bullets.emplace_back(new_obstacle);
    }
    void removeObstacle(Obstacle* obstacle){      //ok
        for (auto it=obstacles.begin();it!=obstacles.end();it++){
            if (*it==obstacle){
                delete obstacle;
                obstacles.erase(it);
                break;
            }
        }

    }
    int ObstacleCount(){return obstacles.size();}   //ez
    void update(sf::Time& time){            //ok
        for (auto& x:obstacles){
            x->update(time);
            if(x->overdue) {removeBullet(x);}
        }
    }
};*/

class Ship: public sf::CircleShape{
    float speed_main_;
    float speed_aux_;
    float speed_angular_;
    int power_=1;
    const float min_cooldown_=0.5;
    float cooldown_=0;
    float indicator_distance_=40;
    float orbiting_radius_=200;
    std::vector<BulletList*>* bullet_storage_;
    sf::Vector2f orbiting_point_;
    Dot indicator_;
    Dot center_;
    GameSettings mode_=GameSettings::Free;

    void move(sf::Time& deltaT){
        if(mode_==GameSettings::Free){
            float horizontal=0,vertical=0;
            float shift_mod=1+0.5*sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
            horizontal=shift_mod*(speed_main_*deltaT.asSeconds())*(sf::Keyboard::isKeyPressed(sf::Keyboard::D)-sf::Keyboard::isKeyPressed(sf::Keyboard::A));
            vertical=shift_mod*(speed_aux_*deltaT.asSeconds())*(sf::Keyboard::isKeyPressed(sf::Keyboard::S)-sf::Keyboard::isKeyPressed(sf::Keyboard::W));

            sf::CircleShape::move(horizontal,vertical);
            center_.move(horizontal,vertical);
            indicator_.move(horizontal,vertical);
        }
        if(mode_==GameSettings::Orbit){
            float angle=0;
            float shift_mod=1+0.5*sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
            angle=getRotation()+shift_mod*(sf::Keyboard::isKeyPressed(sf::Keyboard::D)-sf::Keyboard::isKeyPressed(sf::Keyboard::A))*deltaT.asSeconds()*speed_angular_*0.5;
            std::cout<<angle<<"   "<<getRotation()<<std::endl;

            sf::CircleShape::setRotation(angle);
            setPosition(orbiting_point_+sf::Vector2f{orbiting_radius_*cosf(toRad(angle)),orbiting_radius_*sinf(toRad(angle))});
            center_.setPosition(sf::CircleShape::getPosition());
            indicator_.setPosition(sf::CircleShape::getPosition()+sf::Vector2f{indicator_distance_*cosf(toRad(angle)),indicator_distance_*sinf(toRad(angle))});
        }
    }
    void rotate(sf::Time& deltaT){
        float angle_=getRotation()+deltaT.asSeconds()*speed_angular_*(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)-sf::Keyboard::isKeyPressed(sf::Keyboard::Right));
        setRotation(angle_);
        indicator_.setPosition(getPosition()+sf::Vector2f{indicator_distance_*cosf(toRad(-angle_)),indicator_distance_*sinf(toRad(-angle_))});
    }
    BulletList* bulletlistChoose(std::vector<BulletList*>* index){
        for(auto it=index->begin();it!=index->end();it++){
            if((*it)->region.contains(this->getPosition())){return *it;}          //nie wiem czy lepiej niż poniżej
        }
//        for (BulletList* &x:*index){
//            if(x->region.contains(this->getPosition())){
//                return x;
//            }
//        }
        return nullptr;
    }
public:
    Ship(sf::Vector2f pos, std::vector<BulletList*>* shooting_log):CircleShape(30),indicator_(pos),center_(pos){
        setFillColor({124,215,135});
        setOrigin(30,30);
        setPosition(pos);
        indicator_.setFillColor(sf::Color::Yellow);
        indicator_.move(40,0);
        bullet_storage_=shooting_log;
    }
    Ship(float x, float  y,std::vector<BulletList*>* shooting_log):CircleShape(30),indicator_(x,y),center_(x,y){
        setFillColor({124,215,135});
        setOrigin(30,30);
        setPosition({x,y});
        indicator_.setFillColor(sf::Color::Yellow);
        indicator_.move(40,0);
        bullet_storage_=shooting_log;
    }
    float radius_2_=900;
    void resetStats(){
        speed_angular_=0;
        speed_aux_=0;
        speed_main_=0;
        power_=1;
        setRotation(0);
        cooldown_=0;
    }
    void shoot(float speed){                            //ez
        if(cooldown_>min_cooldown_){
            auto dest=bulletlistChoose(bullet_storage_);
            if(dest!=nullptr){
                cooldown_=0;
                Bullet* temp=new Bullet(getRotation()*(1-2*(mode_==GameSettings::Orbit)),indicator_.getPosition(),speed,power_);
                dest->bullets.emplace_back(temp);
            }
        }
    }
    void update(sf::Time& deltaT){                                      //ez
        if (cooldown_<min_cooldown_) cooldown_+=deltaT.asSeconds();
        move(deltaT);
        if (mode_==GameSettings::Free)rotate(deltaT);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){
            shoot(200);
        }

    }
    void setSpeeds(float main, float aux, float angular){               //ez
        speed_main_=main;
        speed_aux_=aux;
        speed_angular_=angular;
    }
    void setPower(int new_power){power_=new_power;}     //ez
    void setHitbox(int radius){setRadius(radius);}      //ez
    void setControlMode(GameSettings mode){mode_=mode;}     //ez
    void drawShip(sf::RenderWindow& window){
        window.draw(*this);
        window.draw(indicator_);
        window.draw(center_);
    };
    void setOrbit(sf::Vector2f& center, float radius){
        orbiting_radius_=radius;
        orbiting_point_=center;
    }
    void setRotation(float angle){
        sf::CircleShape::setRotation(angle);
        indicator_.setPosition(getPosition()+sf::Vector2f{indicator_distance_*cosf(toRad(-angle)),indicator_distance_*sinf(toRad(-angle))});

    }
    void setBulletStorage(std::vector<BulletList*>* storage){bullet_storage_=storage;}
};

class Obstacle: public sf::CircleShape{
    float direction_;
    float speed_;
    float max_distance_;
    float current_distance_=0;
    int health_=1;
public:
    Obstacle(float direction, float radius, int health=1, float speed=0,float max_distance=400):sf::CircleShape(radius){
        speed_=speed;
        max_distance_=max_distance;
        direction_=direction;
        health_=health;
        setOrigin(radius/2,radius/2);
        setFillColor(sf::Color::Cyan);
    };
    void hit(float strength){
        health_-=strength;
    }
    void update(sf::Time& time){
        if (speed_!=0){
            float temp=speed_*time.asSeconds();
            move(cosf(toRad(direction_))*temp,-sinf(toRad(direction_))*temp);  //ez
            current_distance_+=temp;
        }
        if (current_distance_>=max_distance_||health_<=0){
            terminate();
        }
    }
    bool overdue_=false;
    void terminate(){overdue_=true;}
};

class ObstacleList{
public:
    sf::Vector2f position_;
    float direction_;
    sf::FloatRect region;
    std::vector<Obstacle*> obstacles;
    ObstacleList(sf::FloatRect& obszar){
        region=obszar;
    }
    void addObstacle(Obstacle* new_obstacle){
        obstacles.emplace_back(new_obstacle);
    }
    static Obstacle* randomObstacle(float radius, int health=1, bool isSpeedRandom=0){
        Obstacle* temp = new Obstacle(rand()%360,radius,health,isSpeedRandom*(rand()%100));
        return temp;
    }
    void removeObstacle(Obstacle* obstacle){
        for (auto it=obstacles.begin();it!=obstacles.end();it++){
            if (*it==obstacle){
                delete obstacle;
                obstacles.erase(it);
                break;
            }
        }
    }
    int obstacleCount(){return obstacles.size();}
    void update(sf::Time& time){            //ok
        for (auto& x:obstacles){
            x->update(time);
            if(x->overdue_) {removeObstacle(x);}
        }
    }
};

//template<typename T>
//class ObjectList{
//    static std::vector<T*> objects;
//    static void addObject(T* new_object){
//        objects.emplace_back(new_object);
//    }
//    static void removeObstacle(Obstacle* object){
//        for (auto it=objects.begin();it!=objects.end();it++){
//            if (*it==object){
//                delete object;
//                objects.erase(it);
//                break;
//            }
//        }
//    }
//    int obstacleCount(){return objects.size();}
//    void update(sf::Time& time){
//        for (auto& x:objects){
//            x->update(time);
//            if(x->overdue_) {removeObstacle(x);}
//        }
//    }
//};




class Target: public sf::CircleShape{
    int health_;
public:
    Target(float& radius,const sf::Vector2f& pos):CircleShape(radius){
        setPosition(pos);
    }
    void healthUp(int change=1){health_+=change;}
    void healthDown(int change=1){health_-=change;}
};