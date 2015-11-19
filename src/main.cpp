#include "WProgram.h"
#include "ADC.h"
#include "RingBufferDMA.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>

#define WINDOW_SIZE 10
#define THRESHOLD -50.0

using namespace std;

//SETUP PINS
const int readPin = A9; // ADC0
const int readPin2 = A3; // ADC1
const int readPin3 = A2; // ADC0 or ADC1

//SETUP COUNTERS
unsigned int ADC0COUNT = 0;
int SEEN0 = 0;
unsigned int ADC1COUNT = 0;
int SEEN1 = 0;

//INIT ADC OBJECT
ADC *adc = new ADC(); 

//DECLARE STORAGE STRUCTURES
deque<int16_t> *adc0window = new deque<int16_t>(); 
deque<int16_t> *adc1window = new deque<int16_t>(); 
deque<double> *adc0cand = new deque<double>(); 
deque<int16_t> *adc1cand = new deque<int16_t>(); 

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
    adc->setConversionSpeed(ADC_VERY_LOW_SPEED); // change the conversion speed
    adc->setSamplingSpeed(ADC_VERY_LOW_SPEED); // change the sampling speed
    adc->startContinuous(readPin, ADC_0);
    adc->enableInterrupts(ADC_0);

//  adc->setAveraging(32, ADC_1); // set number of averages
//  adc->setResolution(12, ADC_1); // set bits of resolution
//  adc->setConversionSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the conversion speed
//  adc->setSamplingSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the sampling speed
//  adc->startContinuous(readPin2, ADC_1);
//  adc->enableDMA(ADC_1);
//  adc->enableInterrupts(ADC_1);
}

int main () {

    pinSetup();
    adcSetup();

    Serial.begin(9600);
    delay(500);

    while (1) {

        while (!adc0cand->empty()) {
            Serial.print((int) adc0cand->front());
            adc0cand->pop_front();
        }

//      while (!adc1cand->empty()) {
//          Serial.print((int) adc1cand->front());
//          adc1cand->pop_front();
//      }

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

//responsible for reading in new samples, calculating slope, and finding potential rising/falling edges
void adc0_isr(void) {
    ADC0COUNT++;
    
    //make sure sample window is good
    if (adc0window->size() > WINDOW_SIZE) adc0window->pop_front();
    if (SEEN0 > 0) SEEN0--;

    adc0window->push_back(adc->analogReadContinuous());

    deque<int16_t> x;
    const auto n    = adc0window->size();
    for (auto i = 0; i < n; i ++)
        x.push_back(i);
    const auto s_x  = accumulate(x.begin(), x.end(), 0.0);
    const auto s_y  = accumulate(adc0window->begin(), adc0window->end(), 0.0);
    const auto s_xx = inner_product(x.begin(), x.end(), x.begin(), 0.0);
    const auto s_xy = inner_product(x.begin(), x.end(), adc0window->begin(), 0.0);
    const auto a    = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
    if ((a < THRESHOLD) && (SEEN0 == 0)) {
        adc0cand->push_back(a);
        SEEN0 = 15;
    }

    //TODO: add check for falling edges
    ADC0_RA; // clear interrupt
}

void adc1_isr(void) {
//  ADC1COUNT++;
//  
//  //make sure sample window is good
//  if (adc1window->size() > WINDOW_SIZE) adc1window->pop_front();
//  if (SEEN1 > 0) SEEN1--;

//  adc1window->push_back(adc->analogReadContinuous());

//  deque<int16_t> x;
//  const auto n    = adc1window->size();
//  for (auto i = 0; i < n; i ++)
//      x.push_back(i);
//  const auto s_x  = accumulate(x.begin(), x.end(), 0.0);
//  const auto s_y  = accumulate(adc1window->begin(), adc1window->end(), 0.0);
//  const auto s_xx = inner_product(x.begin(), x.end(), x.begin(), 0.0);
//  const auto s_xy = inner_product(x.begin(), x.end(), adc1window->begin(), 0.0);
//  const auto a    = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
//  if ((a < THRESHOLD) && (SEEN1 == 0)) {
//      adc1cand->push_back(a);
//      SEEN1 = 15;
//  }

//  ADC1_RA; // clear interrupt
}
