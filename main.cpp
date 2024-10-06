#include <pigpio.h>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <cassert>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
using namespace std;

extern void motorImplementationInitialize(int motorPins[], int motorMax);
extern void motorImplementationFinalize(int motorPins[], int motorMax);
extern void motorImplementationSendThrottles(int motorPins[], int motorMax,volatile double motorThrottle[]);
extern void motorImplementationSet3dModeAndSpinDirection(int motorPins[], int motorMax, int mode3dFlag, int reverseDirectionFlag);
void runSlave();
void closeSlave();
void calibrate_motors();
void test_motors();
void motor1();
void motor2();
void motor3();
void motor4();
void control();
void m1_throttle_direction(int throttle, float direction);
void m2_throttle_direction(int throttle, float direction);
void m3_throttle_direction(int throttle, float direction);
void m4_throttle_direction(int throttle, float direction);

#define N 4
#define M 1

int motorPins[N] = {16, 26, 20, 21};
int m1[M] = {26};
int m2[M] = {20};
int m3[M] = {16};
int m4[M] = {21};
volatile double throttles[N];
float speed = 0.01;
volatile double throttles_m1[M]; 
volatile double throttles_m2[M]; 
volatile double throttles_m3[M]; 
volatile double throttles_m4[M]; 



int getControlBits(int, bool);
vector<float> v;
volatile float throttle_controller; 
volatile float rightJoyAxaY;
volatile float rightJoyAxaX;
volatile float roll;
volatile float pitch;

const int slaveAddress = 0x03; // <-- Your address of choice
bsc_xfer_t xfer; // Struct to control data flow
string value("");
int aux = 0;
int response = 0;

int main(){
    motorImplementationInitialize(motorPins, N);
	motorImplementationSet3dModeAndSpinDirection(motorPins, N, 1, 0);
    calibrate_motors();
    throttles_m1[0] = 0; 
    throttles_m2[0] = 0; 
    throttles_m3[0] = 0; 
    throttles_m4[0] = 0; 
    thread t1(control);
    thread t2(runSlave);
    thread t3(motor1);
    thread t4(motor2);
    thread t5(motor3);
    thread t6(motor4);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    //runSlave();
    //closeSlave();

    return 0;
}

// Aici trebuie prelucrate datele de la arduino , controlate motoarele si tot codul de auto echilibrare
void control(){
    unsigned int microsecond = 1000000;
    while(1) {
        if(throttle_controller < 25){
            m1_throttle_direction(throttle_controller, 1);
            m2_throttle_direction(throttle_controller, -1);
            m3_throttle_direction(throttle_controller, 1);
            m4_throttle_direction(throttle_controller, -1);
        } else {
            m1_throttle_direction(25, 1);
            m2_throttle_direction(25, -1);
            m3_throttle_direction(25, 1);
            m4_throttle_direction(25, -1);
        }
        cout << "Variabila 1: " << throttle_controller << endl; 
        cout << "Variabila 2: " << rightJoyAxaY << endl;
        cout << "Variabila 3: " << rightJoyAxaX << endl;
        cout << "Variabila 4: " << roll << endl;
        cout << "Variabila 5: " << pitch << endl;
        //usleep(1 * microsecond);
    }
}

// Colectam datele trimise de catre arduino prin i2c
void runSlave() {
    gpioInitialise();
    cout << "Initialized GPIOs\n";
    // Close old device (if any)
    xfer.control = getControlBits(slaveAddress, false); // To avoid conflicts when restarting
    bscXfer(&xfer);
    // Set I2C slave Address to 0x0A
    xfer.control = getControlBits(slaveAddress, true);
    int status = bscXfer(&xfer); // Should now be visible in I2C-Scanners
    
    if (status >= 0) {
        cout << "Opened slave\n";
        xfer.rxCnt = 0;
        while(1){
            bscXfer(&xfer);
            if(xfer.rxCnt > 0) {
                for(int i = 0; i < xfer.rxCnt; i++){
                    value = value + xfer.rxBuf[i];
                }
                //cout << "Byte nr" << xfer.rxCnt << endl;
               
                istringstream iss(value);
                copy(std::istream_iterator<float>(iss),
                istream_iterator<float>(),
                back_inserter(v));

                /*Print raw sensor data*/
                // copy(v.begin(), v.end(),
                // ostream_iterator<float>(cout, ", "));
                // cout << "\n";
                rightJoyAxaY = v[0];
                rightJoyAxaX = v[1];
                throttle_controller = v[2];
                roll = v[3];
                pitch = v[4];

                value.clear();
                v.clear();
            }
        }
    } else {
        cout << "Failed to open slave!!!\n";}
}

void closeSlave() {
    gpioInitialise();
    cout << "Initialized GPIOs\n";

    xfer.control = getControlBits(slaveAddress, false);
    bscXfer(&xfer);
    cout << "Closed slave.\n";

    gpioTerminate();
    cout << "Terminated GPIOs.\n";
}

int getControlBits(int address /* max 127 */, bool open) {
    int flags;
    if(open)
        flags = /*RE:*/ (1 << 9) | /*TE:*/ (1 << 8) | /*I2:*/ (1 << 2) | /*EN:*/ (1 << 0);
    else // Close/Abort
        flags = /*BK:*/ (1 << 7) | /*I2:*/ (0 << 2) | /*EN:*/ (0 << 0);

    return (address << 16 /*= to the start of significant bits*/) | flags;
}

// Aceasta functie calibreaza motoarele
void calibrate_motors(){
    int i;
    cout << "Initializing ESC / Arm, waiting 5 seconds.\n";
	// send 0 throttle during 5 seconds
	for (i = 0; i < N; i++)
		throttles[i] = 0;
	for (i = 0; i < 5000; i++)
	{
		motorImplementationSendThrottles(motorPins, N, throttles);
		usleep(1000);
	}
}

// Aceste functii trimit in permanenta throttle la motoare sa sa roteasca chiar daca tu nu le-ai modificat parametri
void motor1(){
    while(1){
		motorImplementationSendThrottles(m1, M, throttles_m1);
		usleep(1000);
    }
}

void motor2(){
    while(1){
		motorImplementationSendThrottles(m2, M, throttles_m2);
		usleep(1000);
    }
}

void motor3(){
    while(1){
		motorImplementationSendThrottles(m3, M, throttles_m3);
		usleep(1000);
    }
}

void motor4(){
    while(1){
		motorImplementationSendThrottles(m4, M, throttles_m4);
		usleep(1000);
    }
}

// Cu aceste functii modificii in timp real rotatia si turatia la motoare si le poti apela din functia control()
void m1_throttle_direction(int throttle, float direction){
    if(direction == 1){
        throttles_m1[0] = throttle/100.0;
    } else {
        throttles_m1[0] = -throttle/100.0;
    }
}

void m2_throttle_direction(int throttle, float direction){
    if(direction == 1){
        throttles_m2[0] = throttle/100.0;
    } else {
        throttles_m2[0] = -throttle/100.0;
    }
}

void m3_throttle_direction(int throttle, float direction){
    if(direction == 1){
        throttles_m3[0] = throttle/100.0;
    } else {
        throttles_m3[0] = -throttle/100.0;
    }
}

void m4_throttle_direction(int throttle, float direction){
    if(direction == 1){
        throttles_m4[0] = throttle/100.0;
    } else {
        throttles_m4[0] = -throttle/100.0;
    }
}