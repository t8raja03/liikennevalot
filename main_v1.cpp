#include "mbed.h"

BusOut south(D2, D3, D4);
BusOut west(D5, D6, D7);
 enum lights {r=1, y, rtg, g};
/* BusOut:  0 = kaikki pois,
 *          1 = punainen
 *          2 = keltainen
 *          3 = pun & kelt
 *          4 = vihr
 */


 
InterruptIn west_btn(USER_BUTTON);
InterruptIn south_btn(D8);
DigitalOut led(LED1);

Ticker timeout;

bool timer = 0;
bool southbound = 0;
bool westbound = 0;
const int GREEN = 20;
const int YELLOW = 4;
const int YELLOW_SHORT = 1;
const int RED = 2;
const int RED_SHORT = 1;
const int RTOG = 1;
const int MIN_GREEN = 1;

void isr_change()
{
    timer = 1;
}

void isr_west()
{   
    timeout.detach();
    led = westbound = timer = 1;
    timeout.attach(&isr_change, GREEN);
}

void isr_south()
{
    timeout.detach();
    led = southbound = timer = 1;
    timeout.attach(&isr_change, GREEN);
}

int main()
{
    timeout.attach(&isr_change, GREEN);
    
    south.write(r);
    west.write(g);
    
    west_btn.mode(PullUp);
    west_btn.fall(&isr_west);
    
    south_btn.mode(PullUp);
    south_btn.fall(&isr_south);

    while (true) {
        if (westbound == 1 && southbound == 0) {
            if (west.read() == r && south.read() == g) {
                south.write(y);
                wait(YELLOW_SHORT);
                south.write(r);
                wait(RED_SHORT);
                west.write(rtg);
                wait(RTOG);
                west.write(g);
                wait(MIN_GREEN);
            }
            westbound = 0;
            if (southbound == 0) led = 0;
        }
        else if (southbound == 1 && westbound == 0) {
            if (south.read() == r && west.read() == g) {
                west.write(y);
                wait(YELLOW_SHORT);
                west.write(r);
                wait(RED_SHORT);
                south.write(rtg);
                wait(RTOG);
                south.write(g);
                wait(MIN_GREEN);
            }
            southbound = 0;
            if (westbound == 0) led = 0;
        }
        else if (timer == 1 && westbound == southbound) {
            if (south.read() == r && west.read() == g) {
                west.write(y);
                wait(YELLOW);
                west.write(r);
                wait(RED);
                south.write(rtg);
                wait(RTOG);
                south.write(g);
                wait(MIN_GREEN);
                southbound = 0;
                if (westbound == 0) led = 0;
            }
            else if (west.read() == r && south.read() == g) {
                south.write(y);
                wait(YELLOW);
                south.write(r);
                wait(RED);
                west.write(rtg);
                wait(RTOG);
                west.write(g);
                wait(MIN_GREEN);
                westbound = 0;
                if (southbound == 0) led = 0;
            }
        } // if
        
        timer = 0;
    } // while
    
    
} //main()