#include "klasy_projekt.cpp"

#if 1
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#endif



//klasa target ktora reaguje na obstacle, hitbox obiektu jest wiekszy niz sama planeta w srodku?
/* Jakie kategorie?
 * -karta tytułowa, press any key albo wyjdź od razu? :(
 * -głowne menu z ustawieniami na boku
 * -gameplay na orbicie
 * -win/lose, można w sumie na jednej karcie dać
 *
 */



const float window_width=800;
const float window_height=600;
const sf::Vector2f window_center={window_width/2,window_height/2};
sf::Vector2f orbit_position=window_center;
const float orbit_radius=100;
const float pi=3.14159;
enum class ShipMovementMode;

sf::Vector2f operator*(float,sf::Vector2f);
float toRad(float degrees){
    return degrees*pi/180;
}
float toDeg(float radians){
    return radians/pi*180;
}


enum class GameState{
        Titlecard, SettingsMenu, Gameplay, Win, Lose, Close
    };

struct SessionData{
    using BulletIndexType=std::vector<BulletList*>;
    using ObstacleIndexType=std::vector<ObstacleList*>;
    static ShipMovementMode movemode;
    static GameState gamestate;
    static int score;
    static BulletIndexType* current_bullet_index;
    static ObstacleIndexType* current_obstacle_index;
    static sf::Clock generation_timer_;
    static float obstacle_time_;
    static float obstacle_frequency_;
    static float generation_modifier_;
    static void reset(){
        movemode=ShipMovementMode::Orbit;
        gamestate=GameState::Titlecard;
        score=0;
        if(current_bullet_index!=nullptr)for (auto b:*current_bullet_index){
            b->bullets.erase(b->bullets.begin(),b->bullets.end());
        }
        if(current_obstacle_index!=nullptr)for (auto o:*current_obstacle_index){
            o->obstacles.erase(o->obstacles.begin(),o->obstacles.end());
        }
        current_bullet_index=nullptr;
        current_obstacle_index=nullptr;
        obstacle_frequency_=1;
        generation_timer_.restart();
        obstacle_time_=0;
        generation_modifier_=0;
    }
    static void setMovementMode(ShipMovementMode mode){movemode=mode;}
    static void setGamestate(GameState state){gamestate=state;}
    static void setBulletIndex(std::vector<BulletList*>* bullet_index){current_bullet_index=bullet_index;}
    static void setObstacleIndex(std::vector<ObstacleList*>* obst_index){current_obstacle_index=obst_index;}
    static void updateCollisions(sf::Time& elapsed){
        for (size_t i=0; i<(current_bullet_index->size());i++){
            BulletList* bullets=(*current_bullet_index)[i];
            ObstacleList* obstacles=(*current_obstacle_index)[i];
            for (auto &b: bullets->bullets){
                bool collision=0;
                for (auto &o:obstacles->obstacles){
                    collision=checkForCollision(b,o);
                    if (collision){
                        o->hit(b->power());
                        int temp=o->terminate();
                        if (temp!=0){
                            std::cout<<generation_modifier_<<" - "<<temp<<std::endl;
                            if(temp==1)generation_modifier_+=5*o->given_bonus_;
                            else generation_modifier_+=10*o->given_bonus_;
                            score+=100+400*(o->given_bonus_==1);
                        }
                        break;
                    }
                }
                if(collision){b->terminate();}
            }
            bullets->update(elapsed);
            obstacles->update(elapsed);
        }
    }
    static void generateObstacles(sf::Time& elapsed){
//        std::cout<<generation_timer_.getElapsedTime().asSeconds()<<std::endl;
        obstacle_time_+=elapsed.asSeconds();
        if (obstacle_time_>1/obstacle_frequency_){
            obstacle_time_-=1/obstacle_frequency_;
            int part=rand()%current_obstacle_index->size();
            (*current_obstacle_index)[part]->randomObstacle_Destination(part,window_center,20,520,1);

        }
    }
    static void updateSession(sf::Time& elapsed){
        obstacle_frequency_=expf(fminf(generation_timer_.getElapsedTime().asSeconds()-generation_modifier_,0)/69);
        updateCollisions(elapsed);
        generateObstacles(elapsed);
    }
    static void setGenerationFrequency(float frequency){obstacle_frequency_=frequency;}
private:
    static bool checkForCollision(const Bullet* bullet, const Obstacle* obstacle){
        auto temp = bullet->getPosition()-obstacle->getPosition();
        float bruh = temp.x*temp.x+temp.y*temp.y;           //x^2+y^2=dist^2 (...??)
        float dist=bullet->getRadius()+obstacle->getRadius();   //dist^2, porównanko
        if (bruh<dist*dist) {return true;}
        return false;
    }
}session_flags;

ShipMovementMode SessionData::movemode=ShipMovementMode::Orbit;
GameState SessionData::gamestate=GameState::Titlecard;
int SessionData::score=0;
SessionData::BulletIndexType* SessionData::current_bullet_index=nullptr;
SessionData::ObstacleIndexType* SessionData::current_obstacle_index=nullptr;
sf::Clock SessionData::generation_timer_=sf::Clock();
float SessionData::obstacle_frequency_=1;
float SessionData::obstacle_time_=0;
float SessionData::generation_modifier_=0;


class Button:public sf::RectangleShape{
protected:
    sf::Text text_params_;
    Dot origin_;
    bool state_;
    SessionData session_handle_;
public:
    Button(sf::Vector2f position, std::string text, sf::Font* font, int fontsize=30):text_params_(text,*font,fontsize),origin_(position){
        text_params_.setPosition(position);
        auto temp=text_params_.getGlobalBounds();
//        setOrigin(sf::Vector2f{10,0});
        setPosition(position);
        setSize(sf::Vector2f(temp.width+20,temp.height+20));
        text_params_.setFillColor(sf::Color::Cyan);
    }
    void draw(sf::RenderTarget& target){
        target.draw(*this);
        target.draw(text_params_);
        target.draw(origin_);
    }
    virtual void update_state(sf::RenderWindow& target){
        state_=sf::Mouse::isButtonPressed(sf::Mouse::Left)&&getGlobalBounds().contains(target.mapPixelToCoords(sf::Mouse::getPosition(target)));
//        std::cout<<state_;
    };
};

class Button_Gamestate: public Button{
    GameState state_it_sets_;
public:
    Button_Gamestate(sf::Vector2f position, std::string text, sf::Font* font,GameState its_state, int fontsize=30):Button(position,text,font,fontsize){state_it_sets_=its_state;};
    void execute(sf::RenderWindow& target){
        update_state(target);
        if(state_){
            session_handle_.setGamestate(state_it_sets_);
        }
    }
};

class Button_MovementMode:public Button{
    ShipMovementMode movement_it_sets_;
public:
    Button_MovementMode(sf::Vector2f position, std::string text, sf::Font* font,ShipMovementMode its_mode, int fontsize=30):Button(position,text,font,fontsize){movement_it_sets_=its_mode;}
    void execute(sf::RenderWindow& target){
        update_state(target);
        if(state_){
            session_handle_.setMovementMode(movement_it_sets_);
        }
    }
};





int main() {
//    tworzymy okno
    sf::RenderWindow window(sf::VideoMode(window_width,window_height), "My window",sf::Style::Close);
    window.setFramerateLimit(100);

    sf::FloatRect window_rect({0,0},{window_width,window_height});
    sf::FloatRect r1({window_width/2,0},{window_width/2,window_height/2}),r2({0,0},{window_width/2,window_height/2}),r3({0,window_height/2},{window_width/2,window_height/2}),r4({window_width/2,window_height/2},{window_width/2,window_height/2});

    std::vector<BulletList*> BulletListIndex_Orbit(0);
    std::vector<ObstacleList*> ObstacleListIndex_Orbit(0); //ekran podzielony na cwiartki jak w matematyce i ii iii iv

    std::vector<BulletList*> BulletList_Free(0);
    std::vector<ObstacleList*> ObstacleList_Free(0); //caly ekran to 1 obszar

    sf::Font font;
    if(!font.loadFromFile("Inconsolata-Medium.ttf")){
        std::cout<<"Nie udalo sie zaladowac tekstur, uruchom ponownie <3"<<std::endl;
        window.close();
    }

    Button_Gamestate start_button(sf::Vector2f{100,200},"Start",&font,GameState::Gameplay);
    Button_Gamestate menu_button(sf::Vector2f{100, 300},"To Title",&font, GameState::Titlecard);

    sf::Text score_count(std::to_string(session_flags.score),font);
    score_count.setPosition(0,0);
    score_count.setFillColor({180,90,30});

    BulletList bullet_area1(r1);
    BulletListIndex_Orbit.emplace_back(&bullet_area1);
    BulletList bullet_area2(r2);
    BulletListIndex_Orbit.emplace_back(&bullet_area2);
    BulletList bullet_area3(r3);
    BulletListIndex_Orbit.emplace_back(&bullet_area3);
    BulletList bullet_area4(r4);
    BulletListIndex_Orbit.emplace_back(&bullet_area4);

    BulletList bullet_area_zenbu(window_rect);
    ObstacleList obstacle_area_zenbu(window_rect);

    ObstacleList obstacle_list_temp(r1);
    ObstacleListIndex_Orbit.emplace_back(&obstacle_list_temp);
    obstacle_list_temp=ObstacleList(r2);
    ObstacleListIndex_Orbit.emplace_back(&obstacle_list_temp);
    obstacle_list_temp=ObstacleList(r3);
    ObstacleListIndex_Orbit.emplace_back(&obstacle_list_temp);
    obstacle_list_temp=ObstacleList(r4);
    ObstacleListIndex_Orbit.emplace_back(&obstacle_list_temp); //moglem w petli generowac ale jedyne co potrzebuje to 4 obszary, na wiecej nie zadziala to po co uogolniac :p

    session_flags.setBulletIndex(&BulletListIndex_Orbit);
    session_flags.setObstacleIndex(&ObstacleListIndex_Orbit);




//    tworzymy obiekty jakies
    sf::CircleShape orbitLine(orbit_radius);
    orbitLine.setPosition(orbit_position);
    orbitLine.setOrigin({orbit_radius,orbit_radius});
    orbitLine.setFillColor(sf::Color::Transparent);
    orbitLine.setOutlineColor({180,180,180});
    orbitLine.setOutlineThickness(2);
    Ship statek(0,0,&BulletListIndex_Orbit);
    statek.setControlMode(ShipMovementMode::Orbit);
    statek.setOrbit(orbit_position,orbit_radius);
    statek.setSpeeds(100,150,120);
    sf::RectangleShape linijka(sf::Vector2f(10,10));
    linijka.setFillColor(sf::Color{200,90,200});


    sf::Clock timer;
    SessionData::generation_timer_.restart();

//    main loop
    while (window.isOpen()) {
    //    startowe rzeczy
        sf::Time elapsed=timer.restart();
        sf::Event event;
        window.clear(sf::Color::Black);

        if(session_flags.gamestate==GameState::Titlecard){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed||(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::Escape))
                    window.close();
                if(event.type==sf::Event::KeyPressed&&!(event.key.code==sf::Keyboard::Escape)){
                    session_flags.setGamestate(GameState::SettingsMenu);
                }
            }
            sf::Text splash("=press any key=",font);
            splash.setPosition(350,280);
            splash.setFillColor(sf::Color::Green);
            window.draw(splash);

        }
        else if(session_flags.gamestate==GameState::SettingsMenu){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
             }
            menu_button.draw(window);
            start_button.draw(window);
            menu_button.execute(window);
            start_button.execute(window);

        }
        else if(session_flags.gamestate==GameState::Gameplay){
        //    sprawdzamy eventy
            if(session_flags.movemode==ShipMovementMode::Orbit){
                session_flags.setBulletIndex(&BulletListIndex_Orbit);
            }

            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed||(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::Escape)){
                    window.close();
                }
                if(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::J){
                    int part=rand()%4;
                    ObstacleListIndex_Orbit[part]->randomObstacle_Destination(part,window_center,20,520,1);
                }
            }
        //    obslugujemy eventy
            statek.update(elapsed,1);
            session_flags.updateSession(elapsed);
            score_count.setString(std::to_string(session_flags.score));

        //    czyscimy i rysujemy obiekty
            window.draw(orbitLine);
            for (auto list:ObstacleListIndex_Orbit){
                for (auto obst:list->obstacles){
    //                std::cout<<"przeszkodyy"<<std::endl;
                    window.draw(*obst);
                }
            }
            for (auto list:BulletListIndex_Orbit){
                for (auto bull:list->bullets){
                    window.draw(*bull);
                }
            }
            statek.drawShip(window);
            window.draw(score_count);
        }
        else if(session_flags.gamestate==GameState::Close){
            window.close();
            break;
        }

    //    wyswietlamy
        window.draw(linijka);
        window.display();
    }
    std::cout<<"\n\nNaraaa\n\n-zie\n!"<<std::endl;
    return 0;
}
