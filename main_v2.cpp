/*
*        Rankinen Jarno TVT19KMO
*        Sulautetun tietokoneen perusteet
*        Viikotehtävät 4, tehtävä 7
*        Liikennevalot
*/



#include "mbed.h"


/********************** Liikennevalojen väyläy ***********************************/
BusOut south_lights(D2, D3, D4);
BusOut west_lights(D5, D6, D7);
 enum lights {r=1, y, rtg, g};
/* BusOut:  0 = kaikki pois,
 *          1 = punainen
 *          2 = keltainen
 *          3 = pun & kelt
 *          4 = vihr
 */


 /*********************** Keskeytysten kytkentä *********************************/
InterruptIn west_btn(USER_BUTTON);
InterruptIn south_btn(D8);
DigitalOut led(LED1);   // Nucleon ledillä ilmaistaan, että jompaan kumpaan (tai kumpaankin) suuntaan on autoja odottamassa 

Ticker ticker;

/************************ Tilakoneen liput *****************************/
char triggers = 0;
enum flags {idle, timeout, westbound, southbound=4, all=7};

char green_light = 0;
enum state {west=1, south};

/************************* Aikavakiot ***********************************/
const int GREEN = 20;                       
const int YELLOW = 4;
const int YELLOW_SHORT = 1;
const int RED = 2;
const int RED_SHORT = 1;
const int RTOG = 1;
const int MIN_GREEN = 1;


/************************** Keskeytysfunktiot ****************************/
void isr_change()
{                                           // isr_change() on ajastin, kääntää timeout-lipun triggersistä päälle (1. bitti)
    triggers |= timeout;
}

void isr_west()
{                                           // isr_west() länteen menevien tunnistin,
    ticker.detach();                        // kääntää westbound-lipun triggersistä päälle (2. bitti)
    led = 1;
    triggers |= westbound;
    ticker.attach(&isr_change, GREEN);
}

void isr_south()                            // isr_south() etelään menevien tunnistin
{                                           // kääntää southbound-lipun triggersistä päälle (3.bitti)
    ticker.detach();
    led = 1;                                // HUOM. jostain syystä ticker lakkasi toimimasta, kun näitä funktioita kutsuttiin
    triggers |= southbound;                 // Tästä johtuen ajastin aloitetaan alusta ( attach()->detach() ) kun
    ticker.attach(&isr_change, GREEN);      // tunnistimia käytetään
}


/******* Valojen vaihtofunktiot: ********/
int west_yellow () //Vaihtaa valot länteen vihreiksi
{   
    int yellow_delay;
    int red_delay;

    if (triggers & timeout) {           // Jos ajastettu vaihdos, valot vaihtuvat hieman hitaammin kolareiden välttämiseksi.
        yellow_delay = YELLOW;
        red_delay = RED;
    }
    else {                              // Jos tunnistimien laukaisema vaihdos, keltaisia valoja lyhennetään
        yellow_delay = YELLOW_SHORT;
        red_delay = RED_SHORT;
    }

    south_lights.write(y);
    wait(yellow_delay);
    south_lights.write(r);
    wait(red_delay);
    west_lights.write(rtg);
    wait(RTOG);
    west_lights.write(g);
    wait(MIN_GREEN);

    return west;                // funktio palauttaa arvon 0b0001
}

int south_yellow () // Vaihtaa valot etelään vihreiksi
{
    int yellow_delay;
    int red_delay;

    if (triggers & timeout) {   // Jos ajastettu vaihdos, valot vaihtuvat hieman hitaammin kolareiden välttämiseksi.
        yellow_delay = YELLOW;
        red_delay = RED;
    }
    else {                      // Jos tunnistimien laukaisema vaihdos, keltaisia valoja lyhennetään
        yellow_delay = YELLOW_SHORT;
        red_delay = RED_SHORT;
    }

    west_lights.write(y);
    wait(yellow_delay);
    west_lights.write(r);
    wait(red_delay);
    south_lights.write(rtg);
    wait(RTOG);
    south_lights.write(g);
    wait(MIN_GREEN);

    return south;               // palauttaa arvon 0b0010
}

int main()
{   

/************** Alkuasetukset *********************************/
    ticker.attach(&isr_change, GREEN);      // Ajastimen aloitus
    
    south_lights.write(r);                  // Vihreät länteen
    west_lights.write(g);
    green_light = west;             
    
    west_btn.mode(PullUp);                  // Keskeytysten kytkeminen nappeihin
    west_btn.fall(&isr_west);
    
    south_btn.mode(PullUp);
    south_btn.fall(&isr_south);

    while (true) {

/*************** INCOMING: Auto menossa vain länteen päin ****************/
        if (triggers & westbound) {                 // Jos triggersin 2. bitti 1, eli auto menossa länteen
            if (green_light & south)                // Jos vihreät etelään, green_light 0b0010
                green_light = west_yellow();         // west_yellow() palauttaa arvon 0b0001

            triggers ^= westbound;                  // triggersin toinen bitti käännetään

            if (~triggers & southbound) led = 0;    // sammutetaan led jos etelään ei ole menossa autoja

        }

/*************** INCOMING: Auto menossa vain etelään päin ***********************/
        else if (triggers & southbound) {           // Jos triggersin 3. bitti 1, eli auto menossa etelään

            if (green_light & west)                 // Jos vihreät länteen, green_light 0b0001
                green_light = south_yellow();        // south_yellow() palauttaa arvon 0b0010

            triggers ^= southbound;                 // triggersin kolmas bitti käännetään

            if (~triggers & westbound) led = 0;     // sammutetaan led jos länteen ei ole menossa autoja
        }

/*************** TIMEOUT: Ajastettu vaihtuminen, ei autoja tai molemmista suunnista autoja ***********************/
        else if ( triggers == timeout || triggers == all ) {    // triggersin 1. bitti TAI 1.,2. ja 3. bitti, eli ajastin täynnä, ei autoja tai autoja molempiin suuntiin
            
            if (green_light & west) {                           // Jos vihreät länteen
                green_light = south_yellow();                   // south_yellow() palauttaa arvon 0b0010
                triggers ^= southbound;                         // triggersin 3. bitti, etelään käännetään
                triggers ^= timeout;                            // triggersin 1. bitti, timeout käännetään
                if (~triggers & westbound) led = 0;             // sammutetaan led jos länteen ei ole menossa autoja
            }

            else if (green_light & south) {                     // Jos vihreät etelään
                green_light = west_yellow();                     // west_yellow() palauttaa arvon 0b0001
                triggers ^= westbound;                          // triggersin 2. bitti, länteen käännetään
                triggers ^= timeout;                            // triggersin 1. bitti, timeout käännetään
                if (~triggers & southbound) led = 0;            // sammutetaan led jos etelään ei ole menossa autoja
            }
        } // if
        
    } // while
    
    
} //main()