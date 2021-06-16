#include "klasy_projekt.cpp"

#if 1
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#endif

/* Wersja 1.0!! Co my tu mamy za dobra:
 * -karta tytułowa, esc wyłącza, inne klawisze przechodzą do -
 * -menu! tutaj można uruchomić grę lub wrócić do karty, z menu nie wychodzi się przez esc
 * -gameplay!! noo śmiga elegancko, są przeszkody które trzeba zestrzelić przed ich dotarciem do niebieskiego kółka
 * -rodzaje przeszkód:
 * --niebieskie - normalne, 1hp, jak dolecą to zabierają ci 1hp
 * --fioletowe - nie zadają obrażeń, 2hp, po zestrzeleniu zmniejszają tempo nowych pocisków i regenerują ci 1hp
 * --czerwone - bydlaki 3hp, nie zadają obrażeń ale jak dolecą to znacząco przyspieszają tempo
 * -pauza podczas gry po wciśnięciu esc, P wraca do gry, I kończy grę natychmiast
 * 53000 wynik miałem (serio chcialo mi sie), może to być jakiś próg do testowania się...?
 */


/*
 * tak wgl to mogłem list użyć zamiast vector xD jako że dużo usuwam i dodaję ze środka
 * ahh buul
 */

//używane tylko przed main loop
const float window_width=800;
const float window_height=600;
const sf::Vector2f window_center={window_width/2,window_height/2};
sf::Vector2f orbit_position=window_center;
const float orbit_radius=100;

enum class ShipMovementMode;
enum class GameState;
struct SessionData;
class Button;
class Button_Gamestate;
class Button_MovementMode;


const float pi=3.14159;
sf::Vector2f operator*(float,sf::Vector2f);
float toRad(float degrees){
    return degrees*pi/180;
}
float toDeg(float radians){
    return radians/pi*180;
}

//inicjalizacja static members, no sporo wiem, bywa :P
ShipMovementMode SessionData::movemode=ShipMovementMode::Orbit;
GameState SessionData::gamestate=GameState::Titlecard;
bool SessionData::game_is_on_=false;
bool SessionData::exponential=true;
bool SessionData::pause_is_on_=false;
SessionData::BulletIndexType* SessionData::current_bullet_index=nullptr;
SessionData::ObstacleIndexType* SessionData::current_obstacle_index=nullptr;
const sf::Color SessionData::main_color={180,90,30};
const sf::Color SessionData::secondary_color={140,50,10};
sf::Clock SessionData::generation_timer_=sf::Clock();
sf::Clock SessionData::timer=sf::Clock();
int SessionData::score=0;
float SessionData::obstacle_frequency_=1;
float SessionData::obstacle_time_=0;
float SessionData::generation_modifier_=0;
float SessionData::pause_start_=0;
float SessionData::start_difficulty_modifier_=0;
float SessionData::target_radius_=orbit_radius+20;
float SessionData::pause_time_=0;
sf::Vector2f SessionData::target_position_=window_center;
sf::Vector2f SessionData::window_center_=window_center;
int SessionData::target_health_=3;
sf::Font SessionData::font=sf::Font();



int main() {
//    tworzymy okno
    sf::RenderWindow window(sf::VideoMode(session_flags.window_center_.x*2,window_height), "PSiO Gierka TBN",sf::Style::Close);
    window.setFramerateLimit(100);

    //font initialization, nic nie poradze jak sie czcionka nie zaladuje, niestety
    session_flags.font.loadFromFile("Inconsolata-Medium.ttf");
    if(!session_flags.font.loadFromFile("Inconsolata-Medium.ttf")){
        std::cout<<"Nie udalo sie zaladowac tekstur, uruchom ponownie <3"<<std::endl;
        window.close();
    }
    //regiony
    sf::FloatRect window_rect({0,0},{session_flags.window_center_.x*2,window_height}); // vv moglem w petli ale nie zrobilem ^_^
    sf::FloatRect r1({session_flags.window_center_.x*2/2,0},{session_flags.window_center_.x*2/2,window_height/2}),r2({0,0},{session_flags.window_center_.x*2/2,window_height/2}),r3({0,window_height/2},{session_flags.window_center_.x*2/2,window_height/2}),r4({session_flags.window_center_.x*2/2,window_height/2},{session_flags.window_center_.x*2/2,window_height/2});

    //pregen list obiektowych
    std::vector<BulletList*> BulletListIndex_Orbit(0);
    std::vector<ObstacleList*> ObstacleListIndex_Orbit(0); //ekran podzielony na cwiartki jak w matematyce i ii iii iv
    std::vector<BulletList*> BulletList_Free(0);
    std::vector<ObstacleList*> ObstacleList_Free(0); //caly ekran to 1 obszar

    //guziczki generujemy
    Button_Gamestate start_button("Start",&session_flags.font,GameState::Gameplay);
    start_button.setPositionButton(sf::Vector2f{70,200});
    Button_Gamestate title_button("To Title",&session_flags.font,GameState::Titlecard);
    title_button.setPositionButton(sf::Vector2f{70, 300});
    Button_Gamestate menu_button("To Menu",&session_flags.font, GameState::SettingsMenu);
    Button_Gamestate quit_button("Quit",&session_flags.font,GameState::Close);

    //labelki generujemy textboxy wszelkie
    CustomNeatText score_count(std::to_string(session_flags.score),session_flags.font);
    CustomNeatText health_points(std::to_string(session_flags.target_health_),session_flags.font);
    CustomNeatText score_text("Score: ",session_flags.font);
    CustomNeatText health_text("Lives: ",session_flags.font);
    std::string tutorial_text("\
A,D - movement on orbit <-0->\n\
Shift - makes you move faster\n\
Space - shoot bullets, pew pew\n\
Esc(or changing the focus) - pause\n\n\
Your goal is to shoot as much\n\
as you can :) Shoot the obstacles before\n\
they reach the dark blue circle.\n\
The obstacles come in 3 colors:\n\
* Purple - 2HP, decrease generation rate and\n\
give the target +1HP (very good indeed), they\n\
give more points too ^_^\n\
* Red - 3HP, increases obstacle rate\n\
if not shot down (not good)\n\
* Blue - 1HP, only these deal damage to\n\
the target (you don't want that either)\
");
    CustomNeatText tutorial_splash(tutorial_text,session_flags.font,20);
    tutorial_splash.setPosition(320,50);

    //customowe 'znaczki' +tekst
    PauseSymbol pause_symbol(760,30,session_flags.font);
    X_Symbol x_symbol(760,70,session_flags.font);

    //przydzial list do regionow
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
    ObstacleListIndex_Orbit.emplace_back(&obstacle_list_temp); //moglem w petli generowac ale jedyne co potrzebuje to 4 obszary, dla !=4 nie zadziala to po co uogolniac :p

    //ustawienie list w sesji startowe
    session_flags.setBulletIndex(&BulletListIndex_Orbit);
    session_flags.setObstacleIndex(&ObstacleListIndex_Orbit);

    //elementy mapy
    sf::CircleShape target(SessionData::target_radius_);
    target.setPosition(SessionData::target_position_);
    target.setFillColor(sf::Color::Transparent);
    target.setOrigin(SessionData::target_radius_,SessionData::target_radius_);
    target.setOutlineColor({20,30,160});
    target.setOutlineThickness(2);
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

    Button_Autoshoot autoshoot_button(&statek,&session_flags.font,30,{330,440});
    Button_Generation generation_button(&session_flags,&session_flags.font,30,{330,500});

    //setup zegarów żeby nie koślawiły mi numerków
    SessionData::generation_timer_.restart();
    session_flags.timer.restart();

    //main loop
    while (window.isOpen()) {
        //startowe rzeczy
        sf::Time elapsed=session_flags.timer.restart();
        sf::Event event;
         //czyścimy okno
        if(session_flags.gamestate!=GameState::Pause)window.clear(sf::Color{21,25,33});


        //sprawdzamy stany gry
        if(session_flags.gamestate==GameState::Titlecard){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed||(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::Escape))
                    window.close();
                if(event.type==sf::Event::KeyPressed&&!(event.key.code==sf::Keyboard::Escape)){
                    session_flags.gamestate=GameState::SettingsMenu;
                }
            }
            CustomNeatText splash("=press any key=",session_flags.font);
            CustomNeatText namecard("TBN Shooter Game", session_flags.font, 80);
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
            quit_button.setPositionButton(70,400);
            quit_button.draw(window);
            quit_button.execute(window);
            window.draw(tutorial_splash);
            autoshoot_button.execute(window);
            autoshoot_button.draw(window);
            generation_button.execute(window);
            generation_button.draw(window);
            if(session_flags.gameStartTrigger()){
                score_text.setPosition(10,0);
                health_points.setPosition(110,45);
                health_text.setPosition(10,45);
            }
        }
        else if(session_flags.gamestate==GameState::Lose){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
             }
            CustomNeatText namecard("Final score:",session_flags.font);
            namecard.setPosition(310,260);
            score_count.setPosition(370,300);
            window.draw(namecard);
            window.draw(score_count);
            menu_button.setPositionButton(session_flags.window_center_.x-menu_button.getSize().x/2,350);
            menu_button.execute(window);
            menu_button.draw(window);
            quit_button.setPositionButton(session_flags.window_center_.x-quit_button.getSize().x/2,410);
            quit_button.execute(window);
            quit_button.draw(window);
        }
        else if(session_flags.gamestate==GameState::Pause){
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed){
                    window.close();
                }
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::P)){session_flags.gamestate=GameState::Gameplay;}
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::I)){
                session_flags.game_is_on_=!session_flags.endOfGameCheck(true);
            }
            session_flags.endOfPause();
            pause_symbol.draw(window);
            x_symbol.draw(window);
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
            if(!window.hasFocus())session_flags.gamestate=GameState::Pause;

        //    obslugujemy eventy
            statek.update(elapsed,1);
            session_flags.updateSession(elapsed);
            score_count.setString(std::to_string(session_flags.score));
            score_count.setPosition(110,0);
            health_points.setString(std::to_string(session_flags.target_health_));

            session_flags.game_is_on_=!session_flags.endOfGameCheck();
            session_flags.pauseOnTrigger();

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
//        window.draw(linijka);
        window.display();
    }
    std::cout<<"\nMilego dnia zycze\\\nNara"<<std::endl;
    return 0;
}
