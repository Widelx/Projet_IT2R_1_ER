#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "GPIO.h"


void initPwm(void);
void Initialise_GPIO (void);

int main(void)
{
	LPC_GPIO0->FIODIR2 |= 0xD8;
	LPC_GPIO3->FIODIR0 |= 0x02;
	
	initPwm();
	
	
	return 0;
}


void initPwm(void){ 
	


Initialise_GPIO(); // init GPIO
// initialisation de timer 1
	
	
LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040; // enable PWM1

// TC s'incr�mente toutes les 480 ns
LPC_PWM1->PR = 0; // prescaler

LPC_PWM1->MR0 =1249; // Ceci ajuste la p�riode de la PWM � 40 us

// P3.26 est la sortie PWM Channel 3 de Timer 1
LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7 |0x00300000;
LPC_PWM1->MCR = LPC_PWM1->MCR |0x00000002; // Timer relanc� quand MR0 repasse � 0
// ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
LPC_PWM1->LER = LPC_PWM1->LER | 0x00000009;
LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00000e00; // autorise la sortie PWM
LPC_PWM1->TCR = 1; /*validation de timer 1 et reset counter */
}


void Initialise_GPIO (void)
{
	// Sortie LEDs
	LPC_GPIO2->FIODIR0 = LPC_GPIO2->FIODIR0 | 0x7C;	// FIO2DIR0 pour 5 LEDs LED0 = P2.6 ... LED4 = P2.2  en sortie
	LPC_GPIO1->FIODIR3 = LPC_GPIO1->FIODIR3 | 0xB0;	// FIO2DIR3 pour 3 LEDs LED5 = P1.31, LED6 = P1.29 et LED7 = P1.28 en sortie
	// Entree BP carte principale
	LPC_GPIO2->FIODIR1 = LPC_GPIO2->FIODIR1 & 0xFB;	// FIO2DIR1 pour BP = P2.10 en entree
	// Entrees BP carte d'extension
	LPC_GPIO0->FIODIR2 = LPC_GPIO0->FIODIR2 & 0x00;	// FIO0DIR2 pour BP0 = P0.16 ... BP0 = en entree
	// Entrees Joystick Carte Keil
	LPC_GPIO1->FIODIR3 = LPC_GPIO1->FIODIR3 & 0xF8;	// FIO1DIR3 pour JS en entree
	LPC_GPIO1->FIODIR2 = LPC_GPIO1->FIODIR2 & 0x6F;	// FIO1DIR2 pour JS en entree
	LPC_PINCON->PINMODE1 = LPC_PINCON->PINMODE1|0x0000aaaa; //d�sactive les r�sistances internes du �C pour P0.16, 17, 187, 19, 20, 21, 22,23

}