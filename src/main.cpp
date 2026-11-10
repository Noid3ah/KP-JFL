#include <iostream>
#include <string>
#include "../include/teams_personnel.h"
using namespace std;

int main() {
    Person* person1 = new Player(1, "John Doe", 25, 'M', "Forward", 10, 5, true);
    Person* person2 = new Coach(2, "Jane Smith", 40, 'F', "UEFA Pro License", "Head Coach", "Team A"); 

    Player player1(1, "John Doe", 25, 'M', "Forward", 10, 5, true);

    player1.displayInfo(); // Increment goals for the player
    player1.scoreGoal();
    player1.displayInfo();
    cout << endl;
    person2->displayInfo();
    cout << endl;

    return 0;
}