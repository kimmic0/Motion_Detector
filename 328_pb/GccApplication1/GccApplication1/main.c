 //OCCUPIED: PB0,PB1,PB4, PD0
 /*EDIT: included a while loop in each sweep,
 depending on this distance it will increase or decrease
 delay of led blinking
 TO DO: edit processing so object is green and area 
 around it is red, send data to processing and create sonar 
 */
 #define F_CPU 16000000UL // 16MHz clock
 #define BAUDRATE 9600 // baudrate
 #define BAUD_PRESCALE (((F_CPU / (BAUDRATE * 16UL)))-1)
 //BAUD_PRESCALEFORMULA
 #define Trigger_pin PINB1 //trigger pin
 #include <stdio.h>
 #include <stdlib.h>
 #include <avr/io.h>
 #include <util/delay.h>
 #include <avr/interrupt.h>
 #include <string.h>
 int TimerOverflow = 0;
 char string[20];
 char angle_str[10];
 char dist_str[10];
 unsigned long count;
 int distance = 0;
 int angle = 0;
 
 void usart_init(void)
 {
	 UBRR0H = (uint8_t) (BAUD_PRESCALE >> 8); // LOAD UBRR0 HIGH 8
	 UBRR0L = (uint8_t) (BAUD_PRESCALE); // LOAD UBBR0 LOW
	 UCSR0B = (1 << TXEN0); // USART TRANSMITTER
	 UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 DATA BITS, 1
 }
 
 void usart_send (unsigned char ch)
 {
	 while (! (UCSR0A & (1<<UDRE0))); // WAIT TILL UDR0 IS EMPTY
	 UDR0 = ch; // TRANSMITCHARACTER
 }
 
 void usart_print(char* ChArrPtr)
 {
	 while ((*ChArrPtr) != '\0') // WHILE CHAR ARRAY  ISNT EMPTY
	 {
		 while (! (UCSR0A & (1 << UDRE0))); // WAIT TILL UDR0 IS EMPTY
		 UDR0 = *ChArrPtr; // TRANSMIT CHAR
		 ChArrPtr++; // MOVE TO NEXT CHAR  IN ARRAY
	 }
 }
 
 //interrupt service routine for ultrasonic sensor
 ISR(TIMER1_OVF_vect)
 {
	 TimerOverflow++; // Increment Timer Overflow count
 }
 
 //ultrasonic function
 void ultrasonic()
 {
	 PORTB |= (1 << Trigger_pin); // 10us TRIGGER PULSE
	 _delay_us(10); // DELAY
	 PORTB &= (~(1 << Trigger_pin)); // FALLING EDGE
	 TCNT1 = 0; // CLEAR TIMER1  COUNTER
	 TCCR1B = 0x41; // CAPTURE AT RISING  EDGE SETUP
	 TIFR1 = 1<<ICF1; // CLEAR INPUT CAPTURE FLAG FOR TIMER1
	 TIFR1 = 1<<TOV1; // CLEAR TIMER1  OVERFLOW
	 while ((TIFR1 & (1 << ICF1)) == 0); // WAITS FOR RISING EDGE
	 TCNT1 = 0; // CLEAR TIMER1  COUNTER
	 TCCR1B = 0x01; // SET TO CAPTURE FALLING EDGE
	 TIFR1 = 1<<ICF1; // CLEAR INPUT CAPTURE FLAG FOR TIMER1
	 TIFR1 = 1<<TOV1; // CLEAR TIMER1 OVERFLOW
	 TimerOverflow = 0; // CLEARS TIMER OVERFLOW COUNTER
	 while ((TIFR1 & (1 << ICF1)) == 0); // WAITS TILL FALLING EDGE
	 count = ICR1 + (65535 * TimerOverflow); // COUNT FORMULA
	 distance = (unsigned long)count / (58*16); // DISTANCE FORMULA
	 
	 if(angle <= 9) // IF THE ANGLE IS
	 {
		 dtostrf(angle, 1, 0, angle_str); // CONVERT ANGLE TO
	 }
	 else if(angle <= 99) // ELSE IF THE ANGLE
	 {
		 dtostrf(angle, 2, 0, angle_str); // CONVERT ANGLE TO
	 }
	 else // ELSE
	 {
		 dtostrf(angle, 3, 0, angle_str); // CONVERT ANGLE TO
	 }
	 strcat(angle_str, ","); // APPEND "," TO
	 if(distance <= 9) // IF THE ANGLE IS
	 {
		 dtostrf(distance, 1, 0, dist_str); // CONVERT DISTANCE
	 }
	 else // ELSE
	 {
		 dtostrf(distance, 2, 0, dist_str); // CONVERT DISTANCE
	 }
	 strcat(angle_str, dist_str); // APPEND DIST_STR
	 strcat(angle_str, "."); // APPEND "."
	 usart_print(angle_str); // SEND THE CHAR ARRAY TO USART
 }
 
 //interrupt service routine for timer 2, LED
 ISR(TIMER2_COMPA_vect) // on compare match timer 2, PB4 led will be toggled
{
	PORTB ^= 1<<4;
}
 
 //Servo Motor
 void motor()
 {
	 angle = 0; // START ANGLE AT 0 DEGEREES
	 //servo move forward 0-180
	 for(OCR3A = 1000; OCR3A<=5200; OCR3A+=23)
	 {
		 angle++; // increase angle
		 ultrasonic(); // get distance
		 _delay_ms(5); // delay
		while (distance <= 30)
		 {
			 ultrasonic(); // get distance
			 
			 if(distance > 20 && distance <= 30)
			 {
				OCR2A = OCR2A/2;
				//SWITCH OCR2A HERE
			 }
			 else if(distance > 10 && distance <= 20)
			 {
				OCR2A = OCR2A/4;
			 }
			 else{
				 OCR2A = OCR2A/8;
			 }
			 
		 }
	 }
	 angle = 180; // start angle at 180
	 //servo move backward 180-0
	 for(OCR3A=5200;OCR3A>1000;OCR3A-=23)
	 {
		 angle--; // decrease angle
		 ultrasonic(); // get distance
		 _delay_ms(5); // delay
		 while (distance <= 30) // when object is within range motor stops and led's blink occurs
		 {
			 ultrasonic();  //get distance
			 
			if(distance > 20 && distance <= 30 )
			{
				OCR2A = OCR2A/2;				
			}
			else if(distance > 10 && distance <= 20)
			{
				OCR2A = OCR2A/4;
			}
			else
			{
				OCR2A = OCR2A/8;
			}
		 }
	 }
 }
 
 
 int main ( )
 {
	 /*LED*/
	/* DDRB |= 1<<4; //PB4 as output
	 TIMSK2 |= (1<<OCF2A);
	  OCR2A = 255; // obtained through calculation
	  TCCR2A |= (1<<CS22) | (1<<CS21) | (1<<CS20) | (1<<WGM21); //prescaler = 1024 and wave form generation CTC MODE 
	 */
	 
	 /*MOTOR*/
	 TCCR3A |= (1 << COM3A1) | (1 << COM3B1) | (1 << WGM31); // Compare Output Mode, Fast PWM: Clear OC1A/OC1B on Compare Match, set OC1A/OC1B at BOTTOM (non-inverting mode)
	 TCCR3B |= (1 << WGM33) | (1 << WGM32) | (1 << CS31); //FAST PWM & PRESCALE 8
	 DDRD |= (1 << PIND0); //PWM PINS AS OUT
	 ICR3 = 39999; //FWPM = 50 HZ, T = 20ms. 40k-1
	 
	 /*ULTRASONIC*/
	 DDRB = (1<<DDB1); // PB0 is the Echo Pin & PB1 is the Trigger in
	 usart_init();
	 
	 sei();
	 TIMSK1 = (1 << TOIE1); // ENABLE TIMER1 OVERFLOW INTERRUPTS
	 TCCR1A = 0; // SET
	 
	 while(1)
	 {
		 motor(); // RUN THE MOTOR FUNCTION
	 }
 }