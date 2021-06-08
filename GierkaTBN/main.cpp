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

sf::Vector2f operator*(float,sf::Vector2f);



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

void updateCollisions(BulletList* bullets, ObstacleList* obstacles){
    for (auto &b: bullets->bullets){
        bool collision=0;
        for (auto &o:obstacles->obstacles){
            collision=checkForCollision(b,o);
            if (collision){
                o->terminate();
                break;
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
    window.setFramerateLimit(100);

    sf::FloatRect window_rect({0,0},{window_width,window_height});
    sf::FloatRect r1({0,0},{window_width/2,window_height}),r2({window_width/2,0},{window_width/2,window_height});

    std::vector<BulletList*> BulletListIndex(0);
    std::vector<ObstacleList*> ObstacleListIndex(0);

    BulletList left_area(r1), right_area(r2);
    BulletListIndex.emplace_back(&left_area);
    BulletListIndex.emplace_back(&right_area);

    ObstacleList przeszkody1(r1),prz2(r2);
    ObstacleListIndex.emplace_back(&przeszkody1);
    ObstacleListIndex.emplace_back(&prz2);


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
        sf::Time elapsed=timer.restart();


    //    sprawdzamy eventy
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed||(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::Escape))
                window.close();
            if(event.type==sf::Event::KeyPressed&&event.key.code==sf::Keyboard::J){
                int i= rand()%2;
                ObstacleListIndex[i]->randomObstacles(1,10,0,0,1,window_center,0,3);
//                std::cout<<i<<"  "<<ObstacleListIndex[0]->obstacleCount()<<std::endl;
            }
        }
    //    obslugujemy eventy
        statek.update(elapsed);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)){statek.setControlMode(GameSettings::Orbit);}
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)){statek.setControlMode(GameSettings::Free);}


        updateCollisions(BulletListIndex[0],ObstacleListIndex[0]);
        updateCollisions(BulletListIndex[1],ObstacleListIndex[1]);
        BulletListIndex[0]->update(elapsed);
        BulletListIndex[1]->update(elapsed);
        ObstacleListIndex[0]->update(elapsed);
        ObstacleListIndex[1]->update(elapsed);

    //    czyscimy i rysujemy obiekty
        window.clear(sf::Color::Black);
        window.draw(orbitLine);

        for (auto list:ObstacleListIndex){
            for (auto obst:list->obstacles){
//                std::cout<<"przeszkodyy"<<std::endl;
                window.draw(*obst);
            }
        }
        for (auto list:BulletListIndex){
            for (auto bull:list->bullets){
                window.draw(*bull);
            }
        }
        statek.drawShip(window);

    //    wyswietlamy
        window.display();
    }

    return 0;
}
