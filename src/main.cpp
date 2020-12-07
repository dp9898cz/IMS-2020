#include "main.hpp"

#define DEBUG true

//wing drone specs
const int lightMinutes = 633;       // Average number of sunlight minutes a day 
const int numberOfDrones = 10;
const int droneBattery = 20000;     // meters to drain battery
const int droneSpeed = 1878;        // speed - meters per minute (also battery consumption per minute)
const int droneMaxDistance = 10000; // meters
const int chargeRate = 332;         // meters per minute (60 minutes to charge the drone 0 to 100 %)
const int lastOrderDepach = 622;    // last possible time for drone to leave
const int dispachTime = 1;          // minutes (use zipline or whatever)

int packagesDelivered = 0;
int packagesCreated = 0;

Drone drones[numberOfDrones];

Stat S_deliveryTime("Delivery time (minutes)");
Stat S_distance("Destination distance (metres)");
Stat S_idleWhenCharged("Idling fully charged (minutes)");
Stat S_charging("Charging (minutes)");
Stat S_flight("Drone in the air (minutes)");

void debugPrint(string message) {
    if (DEBUG) {
        cerr << "Debug message " << Time << ": " << message;
    }
}

Drone::Drone() {
    this->currentBattery = droneBattery;
    this->idleTime = Time;
}

Drone * Drone::findChargedDrone(double distance) {
    // search through the list and find optimally charged drone
    for(int i = 0; i < numberOfDrones; i++) {
        // find avaiable drone
        if (!drones[i].Busy()) {
            // get possible distance of that drone
            double possibleDistance = drones[i].currentBattery + ((Time - drones[i].idleTime) * chargeRate);
            if (possibleDistance >= distance) {
                return &drones[i];
            }
        }
    }
    // there is not any avaiable drone 
    return NULL;
}

Package::Package() {
    this->timeCreated = Time;
    this->drone = NULL;
    this->distance = Random() * droneMaxDistance;
    S_distance(this->distance);

    packagesCreated++;  
    string h = "Package created with distance " + to_string(this->distance) + "\n";
    debugPrint(h);
}

void Package::Behavior() {
    this->assignDrone();

    Wait(Uniform(3, 4));        // loading the package to the drone

    debugPrint("Package on the way (distance: " + to_string(this->distance) +  ", round-trip distance: " + to_string(this->distance * 2) + ", battery: " + to_string(this->drone->currentBattery) + "\n");
    Wait(this->distance / droneSpeed);
    this->drone->currentBattery -= this->distance; // drone battery has drained

    Wait(1);        // unloading the package to the drone (wing)

    //package arrived at destination
    debugPrint("Package arrived at destination. Sending the drone home.\n");
    packagesDelivered++;
    S_deliveryTime(Time - this->timeCreated);
    this->releaseDrone();
}

void Package::assignDrone() {
    while (!this->drone) {
        // try to find charged drone
        this->drone = Drone::findChargedDrone(this->distance * 2);
        if (!this->drone) {
            //packageQueue.Insert(this);
            Wait(1);
        }
        else {
            debugPrint("Package has been added to the drone\n");
            Seize(*this->drone);
            // charge the drone (time it has been idling)
            double temp_battery = this->drone->currentBattery;
            this->drone->currentBattery = min(double(droneBattery), this->drone->currentBattery + ((Time - this->drone->idleTime) * chargeRate));
            

            S_charging((this->drone->currentBattery - temp_battery) / chargeRate);
            S_idleWhenCharged(max((Time - this->drone->idleTime) - ((this->drone->currentBattery - temp_battery) / chargeRate), 0.0));

            this->drone->idleTime = -1;
        }
    }
}

void Package::releaseDrone() {
    // wait for the drone to get back
    Wait(this->distance / droneSpeed);
    this->drone->currentBattery -= this->distance; // drone battery has drained

    debugPrint("Drone is back at the center\n");

    S_flight((this->distance *2) / droneSpeed);

    //release drone
    Release(*this->drone);
    this->drone->idleTime = Time;
}


void PackageGenerator::Behavior() {
    if (Time < (lastOrderDepach - 4)){      //dont create if its 15 minutes to the end
        (new Package)->Activate();          //create new package
        Activate(Time + Exponential(1));   //set activation to generate another package 
    }
}


int main() {
    debugPrint("Starting simulation\n");

    Init(0, lightMinutes);
    (new PackageGenerator)->Activate();

    Run();

    debugPrint("Ending silmulation\n");
    
    S_flight.Output();
    S_charging.Output();
    S_idleWhenCharged.Output();
    S_deliveryTime.Output();
    S_distance.Output();

    cout << "-------------------------------------------------------------\n";
    cout << "Drones:\t\t\t" << to_string(numberOfDrones) << "\n";
    cout << "Packages delivered:\t" << to_string(packagesDelivered) << "\n";
    cout << "Packages created:\t" << to_string(packagesCreated) << "\n";

    return 0;
}