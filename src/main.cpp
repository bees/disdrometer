#include "WProgram.h"
#include "ADC.h"
#include "RingBufferDMA.h"
#include <vector>
#include <deque>
#include <queue>

#define WINDOW_SIZE 10

using namespace std;

//SETUP PINS
const int readPin = A9; // ADC0
const int readPin2 = A3; // ADC1
const int readPin3 = A2; // ADC0 or ADC1

//INIT ADC OBJECT
ADC *adc = new ADC(); 

queue<int16_t> *adc0window = new queue<int16_t>(); 
queue<int16_t> *adc1window = new queue<int16_t>(); 

//INIT RINGBUFFERS
const int buffer_size = 8;
DMAMEM static volatile int16_t __attribute__((aligned(buffer_size+0))) buffer[buffer_size];
DMAMEM static volatile int16_t __attribute__((aligned(buffer_size+0))) buffer1[buffer_size];
RingBufferDMA *dmaBuffer0  = new RingBufferDMA(buffer, buffer_size, ADC_0);
RingBufferDMA *dmaBuffer1 = new RingBufferDMA(buffer1, buffer_size, ADC_1);

void pinSetup() {
    pinMode(LED_BUILTIN, OUTPUT);
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
    //adc->enableDMA(ADC_0);
//  adc->enableInterrupts(ADC_0);

//  adc->setAveraging(32, ADC_1); // set number of averages
//  adc->setResolution(12, ADC_1); // set bits of resolution
//  adc->setConversionSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the conversion speed
//  adc->setSamplingSpeed(ADC_VERY_LOW_SPEED, ADC_1); // change the sampling speed
//  adc->startContinuous(readPin2, ADC_1);
//  adc->enableDMA(ADC_1);
//  adc->enableInterrupts(ADC_1);
}

int main () {

//  char msg[32] = {0};
    char c = 0;

//  for (int i = 0; i < WINDOW_SIZE; i++) {
//      adc0Cand->push(0);
//      adc1Cand->push(0);
//  }

    pinSetup();
    adcSetup();


    Serial.begin(9600);
    delay(500);

    while (1) {
        if (Serial.available()) {
            c = Serial.read();
            if(c=='s') { // start dma
                adc->enableInterrupts(ADC_0);
                delay(2000);
                adc->disableInterrupts(ADC_0);
                while (!adc0window->empty()) {
                    Serial.println(adc0window->front());
                    adc0window->pop();
                }

                delay(5);
                Serial.println("d");
            }
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
}

//responsible for reading in new samples, calculating slope, and finding potential rising/falling edges
void adc0_isr(void) {
    adc0window->push(adc->analogReadContinuous());
    ADC0_RA; // clear interrupt
}

void adc1_isr(void) {
    ADC1_RA; // clear interrupt
}

