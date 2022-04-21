/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   STM32F4xx HAL API Template project 
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_LED.h"                  // ::Board Support:LED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Driver_USART.h>

void reception(void const * argument);
void traitement(void const * argument);

osThreadId ID_reception;
osThreadId ID_traitement;

osThreadDef(reception, osPriorityNormal, 1, 0);
osThreadDef(traitement, osPriorityNormal, 1, 0);

  

osMailQId ID_BAL;
osMailQDef(BAL_data,100, char);

extern ARM_DRIVER_USART Driver_USART3;

void Init_UART(void) {
    Driver_USART3.Initialize(NULL);
    Driver_USART3.PowerControl(ARM_POWER_FULL);
    Driver_USART3.Control(	ARM_USART_MODE_ASYNCHRONOUS |						// liaison asynchrone
                            ARM_USART_DATA_BITS_8		|           		// donn�e 8 bits
                            ARM_USART_STOP_BITS_1		|           		// 1 bit de stop
                            ARM_USART_PARITY_NONE		|								// pas de bit de parit�
                            ARM_USART_FLOW_CONTROL_NONE,            // aucun contr�le du flux
                            9600);                                  // default baud rate 9,6 kBD
    Driver_USART3.Control(ARM_USART_CONTROL_TX,1);                  // validation �mission
    Driver_USART3.Control(ARM_USART_CONTROL_RX,1);                  // validation r�ception
}

 
void reception(void const * argument)         // r�ception	
{
	char carac;
	char trameIntro[7]="GPGGA";            // trame GPS qu'on souhaite r�cup�rer
	char data[100];
	
	char *ptr;
	char i;
	
	 while(1)
	 {
		
		 // tache : isolement de la trame
		 // analyse de la trame
     // r�cup�ration un caract�re � la fois d�s qu'il arrive (on �crase l'ancien)
		 
     Driver_USART3.Receive(&carac,1);     // remplir une case
     while(Driver_USART3.GetRxCount()<1); // on attend que 1 case soit pleine
		 
		 if(carac == '$')
		 {
			 Driver_USART3.Receive(data ,5); // la fonction remplira jusqu'� 6 cases
       while (Driver_USART3.GetRxCount()<5 ); // on attend que 6 cases soit pleine
			 data[5]=NULL;
			 
			 if(strncmp(data ,trameIntro,5)==0)
			 {
				 data[5]= ' ';						// r�cup�rer le reste de la trame
				 Driver_USART3.Receive(data ,72); // la fonction remplira jusqu'� 72 cases
				 while (Driver_USART3.GetRxCount()<72); // on attend que 72 cases soient pleines
				 
				 ptr = osMailAlloc(ID_BAL, osWaitForever); // on associe le pointeur � une entr�e libre de la BAL
				 
				 for(i=0;i<100;i++)
				 {
					 *(ptr+i) = data[i];
				 }
				 osMailPut(ID_BAL, ptr); // on envoie l'information
			 }
		 }

	 }
}
void traitement(void const * argument)  // traitement
{
	//char dataCAN[39];
	char trame_recue[100];
	char *recep;
	osEvent EVretour;
	char j;
	
	char  latdir, londir;
	static int heure, minute, latdeg, latmin, londeg, lonmin;
	static float seconde, latitude, longitude, latsec, lonsec;
	
	
	while(1)
	{
		EVretour = osMailGet(ID_BAL, osWaitForever); // attente mail=>MISE EN SOMMEIL
		recep = EVretour.value.p;  // on r�cup�re le pointeur...
		for(j=0;j<100;j++)
		{
			trame_recue[j] = *(recep+j);                        // et la valeur point�e 
		}
		osMailFree(ID_BAL, recep);                   // lib�ration m�moire allou�e
		sscanf(trame_recue,",%2d%2d%f,%f,%c,%f,%c,", 
		&heure,&minute,&seconde,&latitude,&latdir,&longitude,&londir); //on range les donn�es envoy�es par le GPS dans des variables
			
		heure+=2; //on veut l'heure UTC +2:00
						
		latitude *= 10000;
		latsec = (float)((int)latitude%10000)*0.006; //on r�cup�re les sec de la latitude
		latitude-=(int)latitude%10000;
		latitude/=10000;
		latmin = (int)latitude%100; //on r�cup�re les min de la latitude
		latitude -=latmin;
		latitude/=100;
	  latdeg=(int)latitude;//on r�cup�re les deg de la latitude
						
		longitude *= 10000;
		lonsec = (float)((int)longitude%10000)*0.006; // on r�cup�re les sec de la longitude
		longitude-=(int)longitude%10000;
		longitude/=10000;
		lonmin = (int)longitude%100;//on r�cup�re les min de la longitude
		longitude -=lonmin;
		longitude/=100;
		londeg=(int)longitude; //on r�cup�re les deg de la longitude 
						
		
		
	}
}

int main (void){
		osKernelInitialize(); // initialisation CMSIS-RTOS
    Init_UART();          // initialisation p�riph�rique UART
		ID_reception = osThreadCreate(osThread(reception), NULL);
		ID_traitement = osThreadCreate(osThread(traitement), NULL);
		ID_BAL = osMailCreate(osMailQ(BAL_data),NULL);
		osKernelStart();
		osDelay(osWaitForever);
		return 0;
}
