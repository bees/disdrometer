#include "WProgram.h"
#include "ADC.h"
#include "RingBufferDMA.h"
#include "droplet.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include <math.h>

#define WINDOW_SIZE 3
#define THRESHOLD -50.0
#define MAX_WIDTH 900

using namespace std;

//SETUP PINS
const int readPin = A9; // ADC0
const int readPin2 = A3; // ADC1
const int readPin3 = A2; // ADC0 or ADC1
const int LED = 13; // ADC0 or ADC1

//SETUP COUNTERS/FLAGS
int ADC0COUNT = 0;
int SEEN0 = 0;
int ADC1COUNT = 0;
int SEEN1 = 0;


//INIT ADC OBJECT
ADC *adc = new ADC(); 

//sensor values from calibration
int CALIB = 0;
int MAX0 = 0;
int MAX1 = 0;
int MIN0 = 0;
int MIN1 = 0;

//DECLARE STORAGE STRUCTURES
deque<int16_t> *adc0window = new deque<int16_t>(); 
deque<int16_t> *adc1window = new deque<int16_t>(); 
deque<Droplet> *adc0droplets = new deque<Droplet>(); 
deque<Droplet> *adc1droplets = new deque<Droplet>(); 
deque<Droplet> *verif_cand = new deque<Droplet>(); 

//INIT RINGBUFFERS: TODO: get this working if we need extra performance
//const int buffer_size = 8;
//DMAMEM static volatile int16_t __attribute__((aligned(buffer_size+0))) buffer[buffer_size];
//DMAMEM static volatile int16_t __attribute__((aligned(buffer_size+0))) buffer1[buffer_size];
//RingBufferDMA *dmaBuffer0  = new RingBufferDMA(buffer, buffer_size, ADC_0);
//RingBufferDMA *dmaBuffer1 = new RingBufferDMA(buffer1, buffer_size, ADC_1);

void pinSetup() {
    pinMode(readPin, INPUT);
    pinMode(readPin2, INPUT);
    pinMode(readPin3, INPUT);
    pinMode(LED, OUTPUT);       // LED
    digitalWrite(LED, LOW);   // LED on
}

void adcSetup() {
    adc->setAveraging(32); // set number of averages
    adc->setResolution(12); // set bits of resolution
    adc->setConversionSpeed(ADC_MED_SPEED); // change the conversion speed
    adc->setSamplingSpeed(ADC_MED_SPEED); // change the sampling speed

    adc->setAveraging(32, ADC_1); // set number of averages
    adc->setResolution(12, ADC_1); // set bits of resolution
    adc->setConversionSpeed(ADC_MED_SPEED, ADC_1); // change the conversion speed
    adc->setSamplingSpeed(ADC_MED_SPEED, ADC_1); // change the sampling speed
}

void calibrationDialog() {
    CALIB = 1;
    digitalWrite(LED, HIGH);   // LED on
    delay(2000);
    digitalWrite(LED, LOW);   // LED on
            
    
    MAX0 = adc->analogReadContinuous(ADC_0);
    MAX1 = adc->analogReadContinuous(ADC_1);
    delay(1000);
    Serial.print("c");
    Serial.print(MAX0);
    Serial.print(" ");
    Serial.println(MAX1);
    digitalWrite(LED, HIGH);   // LED on
    delay(2000);
    digitalWrite(LED, LOW);   // LED on
            
    MIN0 = adc->analogReadContinuous(ADC_0);
    MIN1 = adc->analogReadContinuous(ADC_1);
    delay(1000);

    Serial.print("c");
    Serial.print(MIN0);
    Serial.print(" ");
    Serial.println(MIN1);
    delay(2000);
    adc1window->clear();
    CALIB = 0;

}

void cross_validation()
{

    Droplet new_drop = adc1droplets->front();

    for (auto cand = adc0droplets->begin(); cand != adc0droplets->end(); ++cand) {
        if (abs(new_drop.timestamp - cand->timestamp) < MAX_WIDTH) {
            verif_cand->push_front(*cand);
            adc1droplets->pop_front();
            adc0droplets->erase(cand);
            Serial.println("c found a match");
        } else
            adc1droplets->pop_front();

        return; 
    }
}




int main () {

    pinSetup();
    adcSetup();
    Serial.begin(9600);
    delay(500);


    //Serial.println("Calibration complete, now running...");
    adc->startContinuous(readPin, ADC_0);
    adc->startContinuous(readPin2, ADC_1);


    while (1) {

        //catch calibration signal
        if (Serial.available()) {
            if (Serial.read() == 'c') {
                adc->disableInterrupts(ADC_0);
                adc->disableInterrupts(ADC_1);
                calibrationDialog();
                adc1window->clear();
                adc1droplets->clear();
                adc->enableInterrupts(ADC_0);
                adc->enableInterrupts(ADC_1);
            }
        }


        if (!adc0droplets->empty()) {
            if (abs(adc0droplets->front().timestamp - ADC0COUNT) > MAX_WIDTH) {
                Serial.println("clearing adc0");
                Serial.print("c ");
                Serial.print(adc0droplets->front().timestamp);
                Serial.print(" ");
                Serial.println(ADC0COUNT);
                adc0droplets->pop_front();
            }
        }

        if (!adc1droplets->empty()) {
//          Serial.print((int) adc1droplets->front().velocity);
//          Serial.print(" ");
//          Serial.print((int) adc1droplets->front().diameter);
//          Serial.print(" ");
//          Serial.println((int) adc1droplets->front().amplitude);
//          adc1droplets->pop_front();
            Serial.println("c1");
            cross_validation();
            if (!adc1droplets->empty()) {
                if (abs(adc1droplets->front().timestamp - ADC1COUNT) > MAX_WIDTH)
                    Serial.println("clearing");
                    adc1droplets->pop_front();
            }
        }

        if (!verif_cand->empty()) {
            Serial.print((int) verif_cand->front().velocity);
            Serial.print(" ");
            Serial.print((int) verif_cand->front().diameter);
            Serial.print(" ");
            Serial.println((int) verif_cand->front().timestamp);
            verif_cand->pop_front();
        }

        if(adc->adc0->fail_flag) {
            Serial.print("ADC0 error flags: 0x");
            Serial.println(adc->adc0->fail_flag, HEX);
        }

        if(adc->adc1->fail_flag) {
            Serial.print("ADC1 error flags: 0x");
            Serial.println(adc->adc1->fail_flag, HEX);
        }

    }

    return 0;
}

void adc1_sample(void) {
}

//responsible for reading in new samples, calculating slope, and finding potential rising/falling edges
void adc0_isr(void) {
    ADC0COUNT++;
    
    if (CALIB == 1) {
        ADC0_RA; 
        return;
    }

    
    int16_t val = adc->analogReadContinuous(ADC_0);
    
    if (abs(val - MAX0) > 100) {
        adc0window->push_back(val);
        SEEN0++;
        //avoid blowing up heap
        if (SEEN0 > 300) {
            SEEN0 = 0;
            adc0window->clear();
        }
    } else if (SEEN0 > 10) {
        int size = adc0window->size();
        int minval = MAX0 + 100;
        for (auto it = adc0window->begin(); it != adc0window->end(); it++) {
            minval = (minval > *it) ? *it : minval;
        }
        double v = ((double) 7100*10)/(double)size;
        int amplitude = abs(minval - MAX0);
        double c = .88437;
        double k = 2.76949;
        double d = v/c;
        d = pow(d, 1/k);
        if (amplitude > 500)
            adc0droplets->push_back(Droplet(amplitude, v, d, ADC0COUNT));

        adc0window->clear();
        SEEN0 = 0;
    } else {
        //avoid sending things that can't possibly be raindrops
        SEEN0 = 0;
    }

    ADC0_RA; // clear interrupt
}


void adc1_isr(void) {
    ADC1COUNT++;
    
    if (CALIB == 1) {
        ADC1_RA; 
        return;
    }

    
    int16_t val = adc->analogReadContinuous(ADC_1);
    
    if (abs(val - MAX1) > 100) {
        adc1window->push_back(val);
        SEEN1++;
        //avoid blowing up heap
        if (SEEN1 > 300) {
            SEEN1 = 0;
            adc1window->clear();
        }
    } else if (SEEN1 > 10) {
        int size = adc1window->size();
        int minval = MAX1 + 100;
        for (auto it = adc1window->begin(); it != adc1window->end(); it++) {
            minval = (minval > *it) ? *it : minval;
        }
        double v = ((double) 7100*10)/(double)size;
        int amplitude = abs(minval - MAX1);
        double c = .88437;
        double k = 2.76949;
        double d = v/c;
        d = pow(d, 1/k);
        if (amplitude > 500)
            adc1droplets->push_back(Droplet(amplitude, v, d, ADC1COUNT));

        adc1window->clear();
        SEEN1 = 0;
    } else {
        //avoid sending things that can't possibly be raindrops
        SEEN1 = 0;
    }

    ADC1_RA; // clear interrupt
}
