#include "klasy_projekt.cpp"

#if 1
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#endif


float window_width=800;
float window_height=600;
sf::Vector2f window_center={window_width/2,window_height/2};
sf::Vector2f orbit_position=window_center;
float orbit_radius=200;
float ind_dist=30;
const float pi=3.14159;



//float distance(const sf::Vector2f& a,const sf::Vector2f& b){
//    return sqrtf(powf(a.x-b.x,2)+powf(a.y-b.y,2));
//}
//float distance(float x1, float y1, float x2, float y2){
//    return sqrtf(powf(x1-x2,2)+powf(y1-y2,2));
//}

float toRad(float degrees){
    return degrees*pi/180;
}
float toDeg(float radians){
    return radians/pi*180;
}


//bool collisionCheck(const std::vector<Bullet*>&bullets, const std::vector<Obstacle*> &obstacles){
//    for (auto& x:bullets){
//        for (auto &y: obstacles){
//            if
//        }
//    }
//}

bool checkForCollision(const Bullet* bullet, const Obstacle* obstacle){
    auto temp = bullet->getPosition()-obstacle->getPosition();
    float bruh = temp.x*temp.x+temp.y*temp.y;           //x^2+y^2=dist^2 (...??)
    float dist=bullet->getRadius()+obstacle->getRadius();   //dist^2, porównanko
    if (bruh<dist*dist) {return true;}
    return false;
}

void updateCollisions(BulletList& bullets, ObstacleList& obstacles){
    for (auto &b: bullets.bullets){
        bool collision=0;
        for (auto &o:obstacles.obstacles){
            collision=checkForCollision(b,o);
            if (collision){
                o->terminate();
            }
        }
        if(collision){b->terminate();}
    }
}


//std::vector<Bullet*> BulletList::bullets(0);
//std::vector<Obstacle*> ObstacleList::obstacles(0);


int main() {
//    tworzymy okno
    sf::RenderWindow window(sf::VideoMode(window_width,window_height), "My window",sf::Style::Close);
    //window.setFramerateLimit(100);
    sf::FloatRect window_rect({0,0},{window_width,window_height});

    std::vector<BulletList*> BulletListIndex(0);
    BulletList pociski(window_rect);
    BulletListIndex.emplace_back(&pociski);

    std::vector<ObstacleList*> ObstacleListIndex(0);
    ObstacleList przeszkody1(window_rect);
    ObstacleListIndex.emplace_back(&przeszkody1);

    for (int i=1; i<=10;i++){
        ObstacleListIndex[0]->obstacles.emplace_back(ObstacleList::randomObstacle(40));
    }

//    tworzymy obiekty jakies
    sf::CircleShape orbitLine(orbit_radius);
    orbitLine.setPosition(orbit_position);
    orbitLine.setOrigin({orbit_radius,orbit_radius});
    orbitLine.setFillColor((sf::Color::Transparent));
    orbitLine.setOutlineColor({180,180,180});
    orbitLine.setOutlineThickness(2);
    Ship statek(0,0,&BulletListIndex);
    statek.setSpeeds(150,150,270);
//    statek.setControlMode(GameSettings::Orbit);
    statek.setOrbit(orbit_position,orbit_radius);


    sf::Clock timer;

//    main loop
    while (window.isOpen()) {
    //    startowe rzeczy
            //resetting edge flags
        sf::Time elapsed=timer.restart();


    //    sprawdzamy eventy
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed||(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::Escape))
                window.close();
        }
    //    obslugujemy eventy
        statek.update(elapsed);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)){statek.setControlMode(GameSettings::Orbit);}
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)){statek.setControlMode(GameSettings::Free);}


        pociski.update(elapsed);

    //    czyscimy i rysujemy obiekty
        window.clear(sf::Color::Black);
        window.draw(orbitLine);

        statek.drawShip(window);


        for (auto list:ObstacleListIndex){
            for (auto obst:list->obstacles){
                window.draw(*obst);
            }
        }

        for (auto list:BulletListIndex){
            for (auto bull:list->bullets){
                window.draw(*bull);
            }
        }


    //    wyswietlamy
        window.display();
    }

    return 0;
}
