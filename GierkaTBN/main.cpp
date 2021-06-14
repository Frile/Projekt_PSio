#include "klasy_projekt.cpp"

#if 1
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#endif


/* Jakie kategorie?
 * -głowne menu z ustawieniami na boku
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
enum class GameState;
struct SessionData;
class Button;
class Button_Gamestate;
class Button_MovementMode;


sf::Vector2f operator*(float,sf::Vector2f);
float toRad(float degrees){
    return degrees*pi/180;
}
float toDeg(float radians){
    return radians/pi*180;
}


ShipMovementMode SessionData::movemode=ShipMovementMode::Orbit;
GameState SessionData::gamestate=GameState::Titlecard;
int SessionData::score=0;
SessionData::BulletIndexType* SessionData::current_bullet_index=nullptr;
SessionData::ObstacleIndexType* SessionData::current_obstacle_index=nullptr;
sf::Clock SessionData::generation_timer_=sf::Clock();
float SessionData::obstacle_frequency_=1;
float SessionData::obstacle_time_=0;
float SessionData::generation_modifier_=0;
sf::Vector2f SessionData::target_position_=window_center;
float SessionData::target_radius_=orbit_radius+20;
int SessionData::target_health_=1;
bool SessionData::game_is_on_=false;
float SessionData::start_difficulty_modifier_=0;
sf::Vector2f SessionData::window_center_=window_center;



int main() {
//    tworzymy okno
    sf::RenderWindow window(sf::VideoMode(window_width,window_height), "PSiO Gierka TBN",sf::Style::Close);
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

    Button_Gamestate start_button("Start",&font,GameState::Gameplay);
    start_button.setButtonPosition(sf::Vector2f{70,200});
    Button_Gamestate title_button("To Title",&font,GameState::Titlecard);
    title_button.setButtonPosition(sf::Vector2f{70, 300});
    Button_Gamestate menu_button("To Menu",&font, GameState::SettingsMenu);
    menu_button.setButtonPosition(10,10);

    CustomNeatText score_count(std::to_string(session_flags.score),font);
    CustomNeatText health_points(std::to_string(session_flags.target_health_),font);
    CustomNeatText score_text("Score: ",font);
    CustomNeatText health_text("Lives: ",font);


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

    sf::CircleShape target(SessionData::target_radius_);
    target.setPosition(SessionData::target_position_);
    target.setFillColor(sf::Color::Transparent);
    target.setOrigin(SessionData::target_radius_,SessionData::target_radius_);
    target.setOutlineColor({20,30,160});
    target.setOutlineThickness(2);


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

        if(sf::Mouse::isButtonPressed(sf::Mouse::Right)){}//       wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww

         //czyścimy okno
        if(session_flags.gamestate!=GameState::Pause)window.clear(sf::Color{11,15,23});

        if(session_flags.gamestate==GameState::Titlecard){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed||(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::Escape))
                    window.close();
                if(event.type==sf::Event::KeyPressed&&!(event.key.code==sf::Keyboard::Escape)){
                    session_flags.setGamestate(GameState::SettingsMenu);
                }
            }
            CustomNeatText splash("=press any key=",font);
            CustomNeatText namecard("TBN Shooter Game", font, 80);
            namecard.setPosition(80,60);
            splash.setPosition(280,400);
            namecard.setScale(1,2);

            window.draw(splash);
            window.draw(namecard);

        }
        else if(session_flags.gamestate==GameState::SettingsMenu){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
             }
            title_button.draw(window);
            start_button.draw(window);
            title_button.execute(window);
            start_button.execute(window);
            if(session_flags.gameStart()){
                score_text.setPosition(10,0);
                score_count.setPosition(110,0);
                health_points.setPosition(110,45);
                health_text.setPosition(10,45);
            }
        }
        else if(session_flags.gamestate==GameState::Lose){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
             }
            CustomNeatText namecard("Final score:",font);
            namecard.setPosition(310,260);
            score_count.setPosition(370,300);
            window.draw(namecard);
            window.draw(score_count);
            menu_button.setButtonPosition(session_flags.window_center_.x-menu_button.getSize().x/2,350);
            menu_button.execute(window);
            menu_button.draw(window);
        }
        else if(session_flags.gamestate==GameState::Pause){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed){
                    window.close();
                }
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::P)){session_flags.gamestate=GameState::Gameplay;}
        }
        else if(session_flags.gamestate==GameState::Gameplay){
        //    sprawdzamy eventy
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed){
                    window.close();
                }
                if((event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::Escape)){
                    session_flags.gamestate=GameState::Pause;
                }
//                if(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::J){
//                    int part=rand()%4;
//                    ObstacleListIndex_Orbit[part]->randomObstacle_Destination(part,session_flags.window_center_,20,520,1);
//                }
            }
        //    obslugujemy eventy
            statek.update(elapsed,1);
            session_flags.updateSession(elapsed);
            score_count.setString(std::to_string(session_flags.score));
            health_points.setString(std::to_string(session_flags.target_health_));
            session_flags.game_is_on_=!session_flags.endOfGameCheck();

        //    czyscimy i rysujemy obiekty
            window.draw(orbitLine);
            window.draw(target);
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
            window.draw(score_text);
            window.draw(score_count);
            window.draw(health_text);
            window.draw(health_points);

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
