#include "stm32l476xx.h"
#include <time.h>

// PA.5  <--> Green LED
// PC.13 <--> Blue user button
#define LED_PIN    5
#define BUTTON_PIN 13

// User HSI (high-speed internal) as the processor clock
void enable_HSI(){
	// Enable High Speed Internal Clock (HSI = 16 MHz)
  RCC->CR |= ((uint32_t)RCC_CR_HSION);
	
  // wait until HSI is ready
  while ( (RCC->CR & (uint32_t) RCC_CR_HSIRDY) == 0 ) {;}
	
  // Select HSI as system clock source 
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSI;  //01: HSI16 oscillator used as system clock

  // Wait till HSI is used as system clock source 
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) == 0 ) {;}
}

void configure_LED_pin(){
  // Enable the clock to GPIO Port A	
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;   
		
	// GPIO Mode: Input(00), Output(01), AlterFunc(10), Analog(11, reset)
	GPIOA->MODER &= ~(3UL<<(2*LED_PIN));  
	GPIOA->MODER |=   1UL<<(2*LED_PIN);      // Output(01)
	
	// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)
	GPIOA->OSPEEDR &= ~(3<<(2*LED_PIN));
	GPIOA->OSPEEDR |=   2<<(2*LED_PIN);  // Fast speed
	
	// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 
	GPIOA->OTYPER &= ~(1<<LED_PIN);      // Push-pull
	
	// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOA->PUPDR  &= ~(3<<(2*LED_PIN));  // No pull-up, no pull-down
}

void configure_Button_pin(){
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN; // Enable clock of Port C
	GPIOC->MODER &= ~GPIO_MODER_MODER13; // Set PC.13 as digital input
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR13; // No pull-up, no pull-down
}

void initialize_controller(){
	enable_HSI();
	configure_LED_pin();
	configure_Button_pin();
}

void turn_on_LED(){
	GPIOA->ODR |= 1 << LED_PIN;
}

void turn_off_LED(){
	GPIOA->ODR &= ~(1 << LED_PIN);
}

void toggle_LED(){
	GPIOA->ODR ^= (1 << LED_PIN);
}

void delay_ms(int milli_seconds){
		int i;
    for(i=0; i<milli_seconds*1000; i++); // Wait milli-seconds
	/* not working (probably because of clock();)
		// Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds) 
		{}
	*/
}

void light_short() {
	turn_on_LED(); // Light is on
	delay_ms(250); // Wait 250ms for short light (1/4 sec)
	turn_off_LED(); // Light is off
	delay_ms(250); // Wait 250ms for on/off (1/4 sec)
}

void light_long() {
	turn_on_LED(); // Light is on
	delay_ms(500); // Wait 500ms for long light (1/2 sec)
	turn_off_LED(); // Light is off
	delay_ms(250); // Wait 250ms for on/off (1/4 sec)
}

void light_sos_message() {
	// 3 shorts 3 longs 3 shorts (...---...)
	light_short();
	light_short();
	light_short();
	
	light_long();
	light_long();
	light_long();
	
	light_short();
	light_short();
	light_short();
}

int main(void){
	// Configurations
	initialize_controller();
	// Variables
	uint32_t input;
	
	// Infinite loop
	while(1){
		// Read pin 13
		input = (GPIOC->IDR & GPIO_IDR_IDR_13);
		if (input == 0) {
		// Button is pressed
			light_sos_message();
		}
	}
}