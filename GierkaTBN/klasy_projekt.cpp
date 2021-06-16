#pragma once
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "klasy_projekt.h"
#include <cmath>
#include <vector>


float toRad(float);
float toDeg(float);
struct SessionData;

sf::Vector2f operator* (float a, sf::Vector2f b){
    return sf::Vector2f{b.x*a,b.y*a};
}

enum class ShipMovementMode{
    None,Orbit,Free/*,Easy,Medium,Hard,Fast,Slow*/
};

enum class GameState{
        Titlecard, SettingsMenu, Gameplay, Lose, Close, Pause // V | V | V | V | V | V   elegancko stany wszystkie dzialaja
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
    Bullet(float direction ,const sf::Vector2f& pos,int speed,int power=1, float radius=6, float max_distance=400): CircleShape(radius){
        direction_=direction;
        setPosition(pos);
        setOrigin(radius,radius);
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
    int power(){return power_;}
};

class BulletList{

public:
    BulletList(sf::FloatRect &obszar):region(obszar){};
    std::vector<Bullet*> bullets;
    sf::FloatRect region;
    void addBullet(Bullet* new_bullet){     //ok
        bullets.emplace_back(new_bullet);
    }
//    void removeBullet(Bullet* bullet){      //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//        for (auto it=bullets.begin();it!=bullets.end();it++){
//            if (*it==bullet){
//                delete bullet;
//                bullets.erase(it);
//                break;
//            }
//        }

//    }
    int bulletCount(){return bullets.size();}   //ez
    void update(sf::Time& time){            //ok
//        for (auto& x:bullets){
//            x->update(time);
//            if(x->overdue) {removeBullet(x);}
//        }
        for(auto it=bullets.begin(); it<bullets.end();){
            (*it)->update(time);
            if ((*it)->overdue){
                bullets.erase(it);
            }else it++;
        }
    }
};

class Ship: public sf::CircleShape{
    float speed_main_;
    float speed_aux_;
    float speed_angular_;
    int power_=1;
    const float min_cooldown_=0.2;
    float cooldown_=0;
    float indicator_distance_=40;
    float orbiting_radius_=200;
    bool auto_shooting=false;
    float bullet_velocity_=100;
    float count=0;
    std::vector<BulletList*>* bullet_storage_;
    sf::Vector2f orbiting_point_;
    Dot indicator_;
    Dot center_;
    ShipMovementMode mode_=ShipMovementMode::Free;

    void move(sf::Time& deltaT, bool shootsOutsideOrbit=false){
        if(mode_==ShipMovementMode::Free){
            float horizontal=0,vertical=0;
            float shift_mod=1+0.7*sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
            horizontal=shift_mod*(speed_main_*deltaT.asSeconds())*(sf::Keyboard::isKeyPressed(sf::Keyboard::D)-sf::Keyboard::isKeyPressed(sf::Keyboard::A));
            vertical=shift_mod*(speed_aux_*deltaT.asSeconds())*(sf::Keyboard::isKeyPressed(sf::Keyboard::S)-sf::Keyboard::isKeyPressed(sf::Keyboard::W));

            sf::CircleShape::move(horizontal,vertical);
            center_.move(horizontal,vertical);
            indicator_.move(horizontal,vertical);
        }
        if(mode_==ShipMovementMode::Orbit){
            float angle=0;
            float shift_mod=1+0.7*sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
            angle=getRotation()+shift_mod*(sf::Keyboard::isKeyPressed(sf::Keyboard::D)-sf::Keyboard::isKeyPressed(sf::Keyboard::A))*deltaT.asSeconds()*speed_angular_;

            sf::CircleShape::setRotation(angle);
            setPosition(orbiting_point_+sf::Vector2f{orbiting_radius_*cosf(toRad(angle)),orbiting_radius_*sinf(toRad(angle))});
            center_.setPosition(sf::CircleShape::getPosition());
            indicator_.setPosition(sf::CircleShape::getPosition()+(1-2*!shootsOutsideOrbit)*sf::Vector2f{indicator_distance_*cosf(toRad(angle)),indicator_distance_*sinf(toRad(angle))});
        }
    }
    void rotate(sf::Time& deltaT){
        float angle_=getRotation()+deltaT.asSeconds()*speed_angular_*(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)-sf::Keyboard::isKeyPressed(sf::Keyboard::Right));
        setRotation(angle_);
        indicator_.setPosition(getPosition()+sf::Vector2f{indicator_distance_*cosf(toRad(-angle_)),indicator_distance_*sinf(toRad(-angle_))});
    }
    BulletList* bulletlistChoose(std::vector<BulletList*>* index){
        for(auto it=index->begin();it!=index->end();it++){
            if((*it)->region.contains(this->getPosition())){return *it;}
        }
        return nullptr;
    }
public:
    Ship(sf::Vector2f pos, std::vector<BulletList*>* shooting_log):CircleShape(20),indicator_(pos),center_(pos){
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
    void resetStats(){
        speed_angular_=0;
        speed_aux_=0;
        speed_main_=0;
        power_=1;
        setRotation(0);
        cooldown_=0;
        bullet_velocity_=80;
    }
    void shoot(float speed, bool shootsOutsideOrbit=false){                            //ez
        if(cooldown_>min_cooldown_){
            auto dest=bulletlistChoose(bullet_storage_);
            if(dest!=nullptr){
                        std::cout<<count++<<"pew pew"<<std::endl;
                cooldown_=0;
                Bullet* temp=new Bullet(getRotation()*(1-2*(mode_==ShipMovementMode::Orbit))+180*(!shootsOutsideOrbit)*(mode_==ShipMovementMode::Orbit),indicator_.getPosition(),speed,power_);
                dest->bullets.emplace_back(temp);
            }
        }
    }
    void update(sf::Time& deltaT, bool shootsOutsideOrbit=false){                                      //ez
        if (cooldown_<min_cooldown_) cooldown_+=deltaT.asSeconds();
        move(deltaT,shootsOutsideOrbit);
        if (mode_==ShipMovementMode::Free)rotate(deltaT);
        if (auto_shooting||sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){
            shoot(bullet_velocity_,shootsOutsideOrbit);
        }

    }
    void setSpeeds(float main, float aux, float angular){               //ez
        speed_main_=main;
        speed_aux_=aux;
        speed_angular_=angular;
    }
    void setPower(int new_power){power_=new_power;}     //ez
    void setHitbox(int radius){setRadius(radius);}      //ez
    void setControlMode(ShipMovementMode mode){mode_=mode;}     //ez
    void setBulletSpeed(float newspeed){bullet_velocity_=newspeed;}
    void drawShip(sf::RenderWindow& window){
        window.draw(*this);
        window.draw(indicator_);
        window.draw(center_);
    };
    void setOrbit(sf::Vector2f& center, float radius){
        orbiting_radius_=radius;
        orbiting_point_=center;
    }
    bool shootingMode(){return auto_shooting;}
    void setRotation(float angle){
        sf::CircleShape::setRotation(angle);
        indicator_.setPosition(getPosition()+sf::Vector2f{indicator_distance_*cosf(toRad(-angle)),indicator_distance_*sinf(toRad(-angle))});

    }
    void setBulletStorage(std::vector<BulletList*>* storage){bullet_storage_=storage;}
    void setShootingMode(bool is_auto){auto_shooting=is_auto;}
};

class Obstacle: public sf::CircleShape{
    float direction_;
    float speed_;
    float max_distance_;
    float current_distance_=0;
    int health_=1;
public:
    int given_bonus_;
    Obstacle(float direction, float radius, int health=1, float speed=0,float max_distance=400, int bonus=0):sf::CircleShape(radius){
        speed_=speed;
        max_distance_=max_distance;
        direction_=direction;
        health_=health;
        setOrigin(radius,radius);
        if(bonus==0){
            setFillColor(sf::Color::Cyan);
        }else if (bonus==1){
            setFillColor(sf::Color::Magenta);
        }else if (bonus==-1){
            setFillColor(sf::Color{200,60,20});
        }
        given_bonus_=bonus;
    };
    void hit(float strength){
        health_-=strength;
    }
    int update(sf::Time& time){
        if (speed_!=0){
            float temp=speed_*time.asSeconds();
            move(cosf(toRad(direction_))*temp,-sinf(toRad(direction_))*temp);  //ez
            current_distance_+=temp;
        }
        return terminate();
    }
    bool overdue_=false;
    int terminate(bool forced=false){
        if(health_<1){
            overdue_=true;
            return 1;
        }
        if(forced||current_distance_>=max_distance_){
            overdue_=true;
            return 2;
        }
        return  0;
    }
    void setSpeed(float speed){speed_=speed;}
};

class ObstacleList{
    int max_size=40;
public:
    sf::Vector2f position_;
    float direction_;
    sf::FloatRect region;
    std::vector<Obstacle*> obstacles;
    ObstacleList(sf::FloatRect& obszar){
        region=obszar;
    }
    void addObstacle(Obstacle* new_obstacle){
        if(obstacleCount()<max_size)obstacles.emplace_back(new_obstacle);
    }
    void randomObstacles(int n=1, float speed=0, bool isSpeedRandom=0, float angle=0, bool isAngleRandom=0, sf::Vector2f pos={0,0}, bool isPositionRandom=1, int health=1, float radius=25){
        for(int i=0;i<n;i++){
            Obstacle* temp = new Obstacle(isAngleRandom*rand()%360+angle*!isAngleRandom,radius,health,isSpeedRandom*(fmodf(rand(),speed)));
            if (isPositionRandom){
                float x=fmodf(rand(),region.width-radius*2)+region.left+radius;
                float y=fmodf(rand(),region.height-radius*2)+region.top+radius;
                temp->setPosition(x,y);
            } else {temp->setPosition(pos);}
            temp->setOrigin(radius,radius);
            temp->setSpeed(speed);
            obstacles.emplace_back(temp);
        }
    }
    void randomObstacle_Destination(int region_number_0_3, sf::Vector2f destination, float speed, float distance,  int health=1, float radius=25){
        int angle=rand()%70+10+90*(region_number_0_3-2);
        int powerup=rand()%25;
        Obstacle* temp;
        if (powerup>4){
            temp = new Obstacle(angle,radius,health,speed,distance,0);
//            std::cout<<"normal"<<std::endl;
        }else if(powerup>2){
            temp = new Obstacle(angle,radius,2,1.5*speed,distance,1);
//            std::cout<<"power-up--------"<<std::endl;
        }else{
            temp = new Obstacle(angle,radius,3,speed,distance,-1);
//            std::cout<<"baddieboy >:D"<<std::endl;
        }
        temp->setOrigin(radius,radius);
        temp->setPosition(destination+sf::Vector2f(cosf(toRad(angle+180))*distance,-sinf(toRad(angle+180))*distance));
        obstacles.emplace_back(temp);
    }
//    void removeObstacle(Obstacle* obstacle){    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//        for (auto it=obstacles.begin();it!=obstacles.end();it++){
//            if (*it==obstacle){
//                delete obstacle;
//                obstacles.erase(it);
//                std::cout<<"wywalone"<<std::endl;
//                break;
//            }
//        }
//    }
    int obstacleCount(){return obstacles.size();}
    void update(sf::Time& time){            //ok
//        for (auto& x:obstacles){
//            x->update(time);
//            if(x->overdue_) {removeObstacle(x);}
//        }
        for(auto it=obstacles.begin(); it<obstacles.end();){
            (*it)->update(time);
            if ((*it)->overdue_){
                obstacles.erase(it);
            }else it++;
        }
    }
};

struct SessionData{
    static sf::Font font;
    const static sf::Color main_color;                  //jasniejszy czerwony taki dla tekstu
    const static sf::Color secondary_color;             //ceimny czrwony taki
    static sf::Clock generation_timer_;                 //zegar do generowania frequency
    static sf::Clock timer;                             //odmierza czas klatki deltaT/elapsed
    static sf::Vector2f window_center_;                 //elegancko przechowane wymiary okna
    static sf::Vector2f target_position_;
    static bool game_is_on_;
    static bool exponential;
    static bool pause_is_on_;
    static ShipMovementMode movemode;                   //jak sie statek rusza
    static GameState gamestate;                         //obecny stan gry
    static int score;                                   //no wynik, normalne i czerwone 100, fioletowe 500
    static int target_health_;                          //ile hp mamy
    static float target_radius_;                        //dystans na jakim obstacle zabiraja hp
    static float obstacle_frequency_;                   //wynik funkcji, mowi ile/s generowac ma obstacle
    static float generation_modifier_;                  //modyfikuje x funkcji generujacej obstacle_frequency_, dodatni zmniejsza ujemny zwieksza f(x)
    static float start_difficulty_modifier_;            //ile plasko jest dodawane do obstacle_frequency_

    static bool endOfGameCheck(bool forced=false){      //konczy gre, ustawia stan gry i czysci
        if(forced||(target_health_<=0&&game_is_on_)){
            resetGame();
            gamestate=GameState::Lose;
            return 1;
        }
        return 0;
    }
    static bool gameStartTrigger(){                     //zaczyna gre
        if(gamestate==GameState::Gameplay&&!game_is_on_){
            game_is_on_=true;
            generation_timer_.restart();
            pause_time_=0;
            return 1;
        }
        return 0;
    }
    static void pauseOnTrigger(){                       //odpowiedz na uruchomienie pauzy
        if(gamestate==GameState::Pause&&!pause_is_on_){
            pause_start_=generation_timer_.getElapsedTime().asSeconds();
            pause_is_on_=true;
        }
    }
    static void endOfPause(bool forced=false){          //odpowiedz na koniec pauzy
        if(forced||(pause_is_on_&&gamestate!=GameState::Pause)){
            pause_time_+=generation_timer_.getElapsedTime().asSeconds()-pause_start_;
        }

    }
    static void setBulletIndex(std::vector<BulletList*>* bullet_index){current_bullet_index=bullet_index;}
    static void setObstacleIndex(std::vector<ObstacleList*>* obst_index){current_obstacle_index=obst_index;}
    static void updateCollisions(sf::Time& elapsed){    //to jest bydle, najpierw sprawdza kolizje obiektow, updateuje wynik ich polozenie i hp, a potem od nowa sprawdza ktore uderzyly w target i zadaly nam hp, wprowadza ich bonusy w zycie
        for (size_t i=0; i<(current_bullet_index->size());i++){
            //kolizje pocisk-obstacle
            BulletList* bullets=(*current_bullet_index)[i];
            ObstacleList* obstacles=(*current_obstacle_index)[i];
            for (auto &b: bullets->bullets){
                if(!b->overdue){
                    bool collision=0;
                    for (auto &o:obstacles->obstacles){
                        collision=checkForCollision(b,o);
                        if (collision){
                            o->hit(b->power());
                            b->terminate();
                        }
                        if(o->overdue_) continue;
                        int temp=o->terminate();
                        if (temp!=0){
                            generation_modifier_+=9*(o->given_bonus_==1);
                            score+=100+300*(o->given_bonus_==1);
                            target_health_+=o->given_bonus_==1;
//                            std::cout<<"shot "<<temp<<"  "<<o->given_bonus_<<"  "<<generation_modifier_<<std::endl;
                        break;
                        }
                    }
                }
            }
            //osobno kolizje z targetem >_<
            for (auto &o:obstacles->obstacles){
                float dist=o->getRadius()+target_radius_;
                dist*=dist;
                float x=target_position_.x-o->getPosition().x;
                float y=target_position_.y-o->getPosition().y;
                if(dist>=x*x+y*y){
                    o->terminate(1);
                    generation_modifier_-=5*(o->given_bonus_==-1);
//                    std::cout<<"natural   "<<o->given_bonus_<<"  "<<generation_modifier_<<"  "<<target_health_<<"  "<<obstacle_frequency_<<std::endl;
                    target_health_-=(o->given_bonus_==0);
                }
            }
            bullets->update(elapsed);
            obstacles->update(elapsed);
        }
    }
    static void generateObstacles(sf::Time& elapsed){   //losuje gdzie obstacle bedzie na podstawie tego czy juz nadszedl na to czas (obstacle_time_)
//        std::cout<<generation_timer_.getElapsedTime().asSeconds()<<std::endl;
        obstacle_time_+=elapsed.asSeconds();
        if (obstacle_time_>1/obstacle_frequency_){
            obstacle_time_-=1/obstacle_frequency_;
            int part=rand()%current_obstacle_index->size();
            (*current_obstacle_index)[part]->randomObstacle_Destination(part,window_center_,20,520,1);
        }
    }
    static void updateSession(sf::Time& elapsed){       //update wszystikego, wszystkie listy jak i tez timer od frequency
        float temp=generation_timer_.getElapsedTime().asSeconds();
        if(gamestate!=GameState::Pause){
            if(exponential)obstacle_frequency_=expf(fmaxf(temp-generation_modifier_-pause_time_,0)/69)+start_difficulty_modifier_; //exponential
            if(!exponential)obstacle_frequency_=1/20*fmaxf(temp-0.8*generation_modifier_-pause_time_,0)+start_difficulty_modifier_; //linear
//            std::cout<<"czas zegara: "<<temp<<" | modifier: "<<generation_modifier_<<" | frequency: "<<obstacle_frequency_<<" | czas gry: "<<temp-pause_time_<<" | score: "<<score<<std::endl;
            updateCollisions(elapsed);
            generateObstacles(elapsed);
        }
    }
private:
    using BulletIndexType=std::vector<BulletList*>;
    using ObstacleIndexType=std::vector<ObstacleList*>;
    static BulletIndexType* current_bullet_index;
    static ObstacleIndexType* current_obstacle_index;
    static float pause_start_;
    static float pause_time_;                           //calkowity czas spedzony w grze ale w pauzie, jest odejmowany zeby nie zepsuc funkcji
    static float obstacle_time_;                        //odstep pomiedzy stworzeniem obstacle
    static void resetGame(){                            //zeruje flagi itp
        movemode=ShipMovementMode::Orbit;
        score=0;
        if(current_bullet_index!=nullptr)for (auto b:*current_bullet_index){
            b->bullets.erase(b->bullets.begin(),b->bullets.end());
        }
        if(current_obstacle_index!=nullptr)for (auto o:*current_obstacle_index){
            o->obstacles.erase(o->obstacles.begin(),o->obstacles.end());
        }
        obstacle_frequency_=1;
        generation_timer_.restart();
        obstacle_time_=0;
        generation_modifier_=0;
        target_health_=3;
    }
    static bool checkForCollision(const Bullet* bullet, const Obstacle* obstacle){      //sparwdza czy dwa obiekty koliduja ze soba, ez
        auto temp = bullet->getPosition()-obstacle->getPosition();
        float bruh = temp.x*temp.x+temp.y*temp.y;           //x^2+y^2=dist^2 (...??)
        float dist=bullet->getRadius()+obstacle->getRadius();   //dist^2, porównanko
        if (bruh<dist*dist) {return true;}
        return false;
    }
}session_flags;

struct CustomNeatText: public sf::Text{
    CustomNeatText(std::string text, sf::Font& font, int size=30):sf::Text(text,font,size){
        setFillColor(session_flags.main_color);
        setOutlineColor(session_flags.secondary_color);
        setOutlineThickness(2);
    }
};

class Button:public sf::RectangleShape{
protected:
    CustomNeatText text_params_;
    Dot origin_;
    bool state_;
    bool latch=false;
    SessionData session_handle_;
public:
    Button(std::string text, sf::Font* font, int fontsize=30,sf::Vector2f position={0,0} ):text_params_(text,*font,fontsize),origin_(position){
        text_params_.setPosition(position+sf::Vector2f{10,0});
        auto temp=text_params_.getGlobalBounds();
//        setOrigin(sf::Vector2f{10,0});
        setPositionButton(position);
        setSize(sf::Vector2f(temp.width+20,temp.height+20));
        setFillColor({173, 153, 106});
        setOutlineColor({133,115,73});
        setOutlineThickness(3);
    }
    void draw(sf::RenderTarget& target){
        target.draw(*this);
        target.draw(text_params_);
//        target.draw(origin_);
    }
    void setPositionButton(sf::Vector2f pos){
        RectangleShape::setPosition(pos);
        text_params_.setPosition(pos+sf::Vector2f{10,0});
    }
    void setPositionButton(float x, float y){
        RectangleShape::setPosition(x,y);
        text_params_.setPosition(sf::Vector2f{x,y}+sf::Vector2f{10,0});
    }
    virtual void update_state(sf::RenderWindow& target){
        state_=sf::Mouse::isButtonPressed(sf::Mouse::Left)&&getGlobalBounds().contains(target.mapPixelToCoords(sf::Mouse::getPosition(target)));
    };
};

class Button_Gamestate: public Button{
    GameState state_it_sets_;
public:
    Button_Gamestate(std::string text, sf::Font* font,GameState its_state, int fontsize=30,sf::Vector2f position={0,0}):Button(text,font,fontsize,position){state_it_sets_=its_state;};
    bool execute(sf::RenderWindow& target){
        update_state(target);
        if(state_){
            session_handle_.gamestate=state_it_sets_;
        }
        return state_;
    }
};

class Button_MovementMode:public Button{
    ShipMovementMode movement_it_sets_;
public:
    Button_MovementMode(std::string text, sf::Font* font,ShipMovementMode its_mode, int fontsize=30,sf::Vector2f position={0,0}):Button(text,font,fontsize,position){movement_it_sets_=its_mode;}
    void execute(sf::RenderWindow& target){
        update_state(target);
        if(state_){
            session_handle_.movemode=movement_it_sets_;
        }
    }
};

class Button_Autoshoot:public Button{   //moglem osobny class do button_latch zrobic ale nie jest potrzebny raczej, tak jak tu na 0-1 moznaby zrobic dowolna ilosc stanow, clockwork dropping gate
    std::string base="AutoShoot: ";
    std::string yee="yes";
    std::string nah="noo";
    Ship* stateczek;
//    int count=0;
public:
    Button_Autoshoot(Ship* ship, sf::Font* font, int fontsize=30,sf::Vector2f position={0,0} ):Button("AutoShoot: noo",font,fontsize,position){stateczek=ship;}
    void execute(sf::RenderWindow& target){
        update_state(target);
        if(state_&&!latch){
//            std::cout<<count++<<" "<<state_;
            latch=true;
//            std::cout<<" lmao";
//            std::cout<<stateczek->shootingMode();
            if(stateczek->shootingMode()){
                text_params_.setString(base+nah);
                stateczek->setShootingMode(0);
            }
            else{
                text_params_.setString(base+yee);
                stateczek->setShootingMode(1);
            }
        }
        if(latch&&!state_){
            latch=false;
//            std::cout<<"oof";
        }
//        std::cout<<std::endl;
    }
};

class Button_Generation:public Button{
    SessionData* data;
    std::string base="Exponential: ";
    std::string yee="yes";
    std::string nah="noo";
public:
    Button_Generation(SessionData* handle, sf::Font* font, int fontsize=30,sf::Vector2f position={0,0}):Button("Exponential: yes",font,fontsize,position){data=handle;}
    void execute(sf::RenderWindow& target){
        update_state(target);
        if(state_&&!latch){
            latch=true;
            if(data->exponential){
                text_params_.setString(base+nah);
            }
            else{
                text_params_.setString(base+yee);
            }
            data->exponential=!data->exponential;
        }
        if(latch&&!state_){
            latch=false;
        }
    }
};

class PauseSymbol{
    sf::RectangleShape pt1;
    sf::RectangleShape pt1_2;
    sf::RectangleShape pt2;
    sf::RectangleShape pt2_2;
    sf::RectangleShape cross;
//    sf::RectangleShape base;
    CustomNeatText label;
public:
    PauseSymbol(float x, float y, sf::Font &font):label("P",font){
        pt1.setSize(sf::Vector2f{6,24});
        pt1.setPosition(x,y);
        pt1.setOrigin(8,12);
        pt1.setFillColor(session_flags.main_color);
        pt1.setOutlineColor(session_flags.secondary_color);
        pt1.setOutlineThickness(2);
        pt1_2.setSize(sf::Vector2f{6,24});
        pt1_2.setPosition(x,y);
        pt1_2.setOrigin(8,12);
        pt1_2.setFillColor(session_flags.main_color);


        pt2.setSize(sf::Vector2f{6,24});
        pt2.setPosition(x,y);
        pt2.setOrigin(-4,12);
        pt2.setFillColor(session_flags.main_color);
        pt2.setOutlineColor(session_flags.secondary_color);
        pt2.setOutlineThickness(2);
        pt2_2.setSize(sf::Vector2f{6,24});
        pt2_2.setPosition(x,y);
        pt2_2.setOrigin(-4,12);
        pt2_2.setFillColor(session_flags.main_color);

//        base.setSize({24,24});
//        base.setPosition(x,y);
//        base.setOrigin(12,12);

        label.setPosition(x,y);
        label.setOrigin(40,20);

        cross.setSize(sf::Vector2f{4,32});
        cross.setPosition(x,y);
        cross.setOrigin(2,17);
        cross.setRotation(80);
        cross.setFillColor(session_flags.main_color);
        cross.setOutlineColor(session_flags.secondary_color);
        cross.setOutlineThickness(2);
    }
    void draw(sf::RenderTarget& target, bool crossed_=true, bool key_=true){
        target.draw(pt1);
        target.draw(pt2);
        if(crossed_){
            target.draw(cross);
            target.draw(pt1_2);
            target.draw(pt2_2);
        }
        if(key_)target.draw(label);
    }

};

struct X_Symbol{
    X_Symbol(float x, float y, sf::Font &font):label("I",font){
        pt1.setSize(sf::Vector2f{6,30});
        pt1.setPosition(x,y);
        pt1.setOrigin(4,15);
        pt1.setFillColor(session_flags.main_color);
        pt1.setOutlineColor(session_flags.secondary_color);
        pt1.setOutlineThickness(2);
        pt1.setRotation(45);
        pt1_2.setSize(sf::Vector2f{6,30});
        pt1_2.setPosition(x,y);
        pt1_2.setOrigin(4,15);
        pt1_2.setFillColor(session_flags.main_color);
        pt1_2.setRotation(45);

        pt2.setSize(sf::Vector2f{6,30});
        pt2.setPosition(x,y);
        pt2.setOrigin(4,15);
        pt2.setFillColor(session_flags.main_color);
        pt2.setOutlineColor(session_flags.secondary_color);
        pt2.setOutlineThickness(2);
        pt2.setRotation(-45);

        label.setOrigin(40,20);
        label.setPosition(x,y);
    }
    void draw(sf::RenderTarget& window){
        window.draw(pt1);
        window.draw(pt2);
        window.draw(pt1_2);
        window.draw(label);
    }
private:
    CustomNeatText label;
    sf::RectangleShape pt1;
    sf::RectangleShape pt1_2;
    sf::RectangleShape pt2;

};



