#ifndef MAIN_HPP
#define MAIN_HPP

#include "simlib.h"

#include <iostream>
#include <string>

using namespace std;

class Drone : public Facility {
    public:
        double currentBattery;
        double idleTime;

        Drone();
        static Drone * findChargedDrone(double distance);
};

class Package : public Process {
    public:
        double distance;
        Drone * drone;
        double timeCreated;

        Package();
        void Behavior();
        void assignDrone();
        void releaseDrone();
};

class PackageGenerator : public Event {
	void Behavior();
};

/* class PackageQueue : public Queue {
    using Queue::Queue;

    public:
        void activateFirst();
}; */

void debugPrint(string message);

#endif // #define MAIN_HPP