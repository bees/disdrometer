#include "WProgram.h"
#include "ADC.h"
#include "RingBufferDMA.h"
#include "cand.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>

#define WINDOW_SIZE 5
#define THRESHOLD -50.0

using namespace std;

//SETUP PINS
const int readPin = A9; // ADC0
const int readPin2 = A3; // ADC1
const int readPin3 = A2; // ADC0 or ADC1

//SETUP COUNTERS
unsigned int ADC0COUNT = 0;
int SEEN0 = 0;
int SEEN_R0 = 0;
unsigned int ADC1COUNT = 0;
int SEEN1 = 0;
int SEEN_R1 = 0;

//INIT ADC OBJECT
ADC *adc = new ADC(); 

//sensor values from calibration
int MAX0 = 0;
int MAX1 = 0;
int MIN0 = 0;
int MIN1 = 0;

//DECLARE STORAGE STRUCTURES
deque<int16_t> *adc0window = new deque<int16_t>(); 
deque<int16_t> *adc1window = new deque<int16_t>(); 
deque<Candidate> *adc0cand = new deque<Candidate>(); 
deque<Candidate> *adc1cand = new deque<Candidate>(); 
deque<Candidate> *adc0cand_r = new deque<Candidate>(); 
deque<Candidate> *adc1cand_r = new deque<Candidate>(); 

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
}

void adcSetup() {
    adc->setAveraging(32); // set number of averages
    adc->setResolution(12); // set bits of resolution
    adc->setConversionSpeed(ADC_LOW_SPEED); // change the conversion speed
    adc->setSamplingSpeed(ADC_VERY_LOW_SPEED); // change the sampling speed

    adc->setAveraging(32, ADC_1); // set number of averages
    adc->setResolution(12, ADC_1); // set bits of resolution
    adc->setConversionSpeed(ADC_LOW_SPEED, ADC_1); // change the conversion speed
    adc->setSamplingSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the sampling speed
}

void calibrationDialog() {
    unsigned char input = '\0';
    while(1) {
        if(Serial.available())
            input = Serial.read();
        if (input == 's')
            break;
    }

    Serial.println("Make sure photodiodes are unblocked, enter \'c\' to continue");
            
    while(1) {
        if(Serial.available())
            input = Serial.read();
        if (input == 'c')
            break;
    }
    
    MAX0 = adc->analogRead(ADC_0);
    //MAX1 = adc->analogRead(ADC_1);
    Serial.print("ADC0 max: ");
    Serial.println(MAX0);
    

    Serial.println("Make sure photodiodes are completely blocked, enter \'c\' to continue");
            
    while(1) {
        if(Serial.available())
            input = Serial.read();
        if (input == 'c')
            break;
    }

    MIN0 = adc->analogRead(ADC_0);

    Serial.print("ADC0 min: ");
    Serial.println(MIN0);

}

int main () {

    pinSetup();
    adcSetup();
    Serial.begin(9600);
    delay(500);

    //calibrationDialog();

    //Serial.println("Calibration complete, now running...");
    adc->startContinuous(readPin, ADC_0);
    adc->startContinuous(readPin2, ADC_1);
    adc->enableInterrupts(ADC_0);
    adc->enableInterrupts(ADC_1);

    while (1) {

        while (!adc0cand->empty()) {

            Serial.println("1");
            Serial.println((int) adc0cand->front().slope);
            Serial.println("1");
            adc0cand->pop_front();
        }

        while (!adc0cand_r->empty()) {
            Serial.println("2");
            Serial.println((int) adc0cand_r->front().slope * 100);
            Serial.println("2");
            adc0cand_r->pop_front();
        }

        while (!adc1cand->empty()) {
            Serial.println("3");
            Serial.println((int) adc1cand->front().slope * 10);
            Serial.println("3");
            adc1cand->pop_front();
        }
        while (!adc1cand_r->empty()) {
            Serial.println("4");
            Serial.println((int) adc1cand_r->front().slope * 1000);
            Serial.println("4");
            adc1cand_r->pop_front();
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
    
    //make sure sample window is good
    if (adc0window->size() > WINDOW_SIZE) adc0window->pop_front();
    if (SEEN0 > 0) SEEN0--;
    if (SEEN_R0 > 0) SEEN_R0--;

    adc0window->push_back(adc->analogReadContinuous(ADC_0));

    deque<int16_t> x;
    const auto n    = adc0window->size();
    for (unsigned int i = 0; i < n; i ++)
        x.push_back(i);
    const auto s_x  = accumulate(x.begin(), x.end(), 0.0);
    const auto s_y  = accumulate(adc0window->begin(), adc0window->end(), 0.0);
    const auto s_xx = inner_product(x.begin(), x.end(), x.begin(), 0.0);
    const auto s_xy = inner_product(x.begin(), x.end(), adc0window->begin(), 0.0);
    const auto a    = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
    if ((a < THRESHOLD) && (SEEN0 == 0)) {
        adc0cand->push_back(Candidate(a, ADC0COUNT - WINDOW_SIZE, adc0window->back()));
        SEEN0 = 20;
    } else if ((-1*abs(a) < THRESHOLD) && (SEEN_R0 == 0)) {
        adc0cand_r->push_back(Candidate(a, ADC0COUNT, adc0window->back()));
        SEEN_R0 = 25;
    }

//  //TODO: add check for falling edges
    ADC0_RA; // clear interrupt
}


void adc1_isr(void) {
    ADC1COUNT++;
    
    //make sure sample window is good
    if (adc1window->size() > WINDOW_SIZE) adc1window->pop_front();
    if (SEEN1 > 0) SEEN1--;
    if (SEEN_R1 > 0) SEEN_R1--;

    adc1window->push_back(adc->analogReadContinuous(ADC_1));

    deque<int16_t> x;
    const auto n    = adc1window->size();
    for (unsigned int i = 0; i < n; i ++)
        x.push_back(i);
    const auto s_x  = accumulate(x.begin(), x.end(), 0.0);
    const auto s_y  = accumulate(adc1window->begin(), adc1window->end(), 0.0);
    const auto s_xx = inner_product(x.begin(), x.end(), x.begin(), 0.0);
    const auto s_xy = inner_product(x.begin(), x.end(), adc1window->begin(), 0.0);
    const auto a    = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
    if ((-1*abs(a) < THRESHOLD) && (SEEN1 == 0)) {
        adc1cand->push_back(Candidate(a, ADC1COUNT - WINDOW_SIZE, adc1window->back()));
        SEEN1 = 20;
    } else if ((-1*abs(a) < THRESHOLD) && (SEEN_R1 == 0)) {
        adc1cand_r->push_back(Candidate(a, ADC1COUNT, adc1window->back()));
        SEEN_R1 = 25;
    }

    ADC1_RA; // clear interrupt
}
