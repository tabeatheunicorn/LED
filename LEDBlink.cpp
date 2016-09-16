#include "MKL27Z644.h"
#include <stdlib.h>




void Delay()
{
	int i;
	for (i = 0; i < 100000; i++)
		asm("nop");
}
void Delays(int a)
{
	int i;
	for (i = 0; i < 7000*a; i++)
		asm("nop");
}
void RGBinit()
{
	//Clocks für die Ports & Pins setzten
	SIM_BASE_PTR->SCGC5 |= SIM_SCGC5_PORTB_MASK;

	SIM_BASE_PTR->SCGC5 |= SIM_SCGC5_PORTA_MASK;
	SIM_BASE_PTR->SCGC5 |= SIM_SCGC5_PORTC_MASK;


	SIM_BASE_PTR->SCGC6 |= SIM_SCGC6_TPM0_MASK | SIM_SCGC6_TPM1_MASK | SIM_SCGC6_TPM2_MASK;
	SIM_BASE_PTR->SOPT2 |= SIM_SOPT2_TPMSRC(3); 

	//Muxen
	PORTA_BASE_PTR->PCR[13] = PORT_PCR_MUX(3);
	PORTB_BASE_PTR->PCR[18] = PORT_PCR_MUX(3);
	PORTB_BASE_PTR->PCR[19] = PORT_PCR_MUX(3);
	PORTA_BASE_PTR->PCR[2]  = PORT_PCR_MUX(3);
	PORTA_BASE_PTR->PCR[4]  = PORT_PCR_MUX(1);
	PORTC_BASE_PTR->PCR[1] = PORT_PCR_MUX(1);


	//TPM konfigurieren
	TPM0_SC |= TPM_SC_CMOD(1) | TPM_SC_CPWMS(1);
	TPM1_SC |= TPM_SC_CMOD(1) | TPM_SC_CPWMS(1);
	TPM2_SC |= TPM_SC_CMOD(1) | TPM_SC_CPWMS(1);


	//TPM Kanäle konfigurieren
	TPM1_C1SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);
	TPM2_C1SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);
	TPM2_C0SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);

	//Default Werte
	TPM1_MOD = 350;
	TPM2_MOD = 300;

	TPM2_C0V = 0; //rot
	TPM2_C1V = 0; //grün
	TPM1_C1V = 0; //blau
	
	TPM1_POL |= TPM_POL_POL1_MASK;
	TPM2_POL |= TPM_POL_POL0_MASK | TPM_POL_POL1_MASK;


}

void setzeFarbe(int r, int g, int b)
{
	TPM2_C0V = r; //rot
	TPM2_C1V = g; //grün
	TPM1_C1V = b; //blau
}

int fade_counter = 0;

//232 61 57 Zielfarbton
int wrot = 232;
int wgreen = 61; 
int wblau = 57;

void setzeZielfarbe(int r, int g, int b)
{
//startwerte definieren
	int start_rot = TPM2_C0V; //rot
	int start_gruen = TPM2_C1V; //grün
	int start_blau = TPM1_C1V; //blau

	fade_counter = 0;

	wrot = r;
	wgreen = g;
	wblau = b;

}

int farbe1[3] = { 0,0,0 };
int farbe2[3] = { 0,0,0 };
int farbe3[3] = {0,0,0 }; 

//Farbwerte ueberwachen
int farbe11 = farbe1[0];

void dreiFarbenFade()
{
	
	int j = 1;
	int start_rot = TPM2_C0V; //rot
	int start_gruen = TPM2_C1V; //grün
	int start_blau = TPM1_C1V; //blau



	//Abstaende der Farbwerte zueinander
	int srot = (farbe1[0] - start_rot);
	int sblau = (farbe1[2] - start_blau);
	int sgruen = (farbe1[1] - start_gruen);

	int temprot = 0;
	int tempgruen = 0;
	int tempblau = 0;

	//for (int zaehler = 0; zaehler < 3;)
	int zaehler = 0;
	while(zaehler<3)
	{
	
		if (fade_counter < 255)
		{
			temprot = start_rot + ((srot * fade_counter) / 255);
			tempgruen = start_gruen + (sgruen * fade_counter / 255);
			tempblau = start_blau + (sblau * fade_counter / 255);
			setzeFarbe(temprot, tempgruen, tempblau);
			fade_counter++;
			Delay();
		}
		else if (j == 1)
		{
			// Neuer Farbabstand
			srot = (farbe2[0] - farbe1[0]);
			sgruen = (farbe2[1] - farbe1[1]);
			sblau = (farbe2[2] - farbe1[2]);

			//Neue Startwerte der Farbtoene
			start_rot = farbe1[0];
			start_gruen = farbe1[1];
			start_blau = farbe1[2];

			fade_counter = 0;
			j = 2;
			zaehler++;
		}
		else if (j == 2)
		{
			srot = (farbe3[0] - farbe2[0]);
			sgruen = (farbe3[1] - farbe2[1]);
			sblau = (farbe3[2] - farbe2[2]);

			//Neue Startwerte der Farbtoene
			start_rot = farbe2[0];
			start_gruen = farbe2[1];
			start_blau = farbe2[2];

			fade_counter = 0;
			j = 0;
			zaehler++;
		}
		else
		{
			srot = (farbe1[0] - farbe3[0]);
			sgruen = (farbe1[1] - farbe3[1]);
			sblau = (farbe1[2] - farbe3[2]);
			
			//Neue Startwerte der Farbtoene
			start_rot = farbe3[0];
			start_gruen = farbe3[1];
			start_blau = farbe3[2];

			fade_counter = 0;
			j = 1;
			zaehler++;
		}	
	}
}
	
//Anzahl der Durchläufe von Interrupt A
int anzdurch = 0;

extern "C" void PORTA_IRQHandler() 
{
	fade_counter = 0;
	if (anzdurch == 0)
	{
		anzdurch++;
		int ton1[3] = { 249, 131, 81 };
		int ton2[3] = { 175, 38, 20 };
		int ton3[3] = { 44, 42, 45 };
		for (int i = 0; i < 3; i++)
		{
			farbe1[i] = ton1[i];
			farbe2[i] = ton2[i];
			farbe3[i] = ton3[i];
		}
		dreiFarbenFade();
	}
	else if (anzdurch == 1)
	{
		anzdurch++;
		int ton1[3] = { 232, 61, 57 };
		int ton2[3] = { 100, 0, 100 };
		int ton3[3] = { 12, 34, 56 };
		for (int i = 0; i < 3; i++)
		{
			farbe1[i] = ton1[i];
			farbe2[i] = ton2[i];
			farbe3[i] = ton3[i];
		}
		dreiFarbenFade();
	}
	else
	{
		anzdurch = 0;
		int ton1[3] = {174, 28, 40 };
		int ton2[3] = {255, 255, 255 };
		int ton3[3] = {30, 70, 139 };
		for (int i = 0; i < 3; i++)
		{
			farbe1[i] = ton1[i];
			farbe2[i] = ton2[i];
			farbe3[i] = ton3[i];
		}
		dreiFarbenFade();
	}
	PORTA_BASE_PTR->ISFR = (1<<4);
	NVIC_ClearPendingIRQ(PORTA_IRQn);
}

extern "C" void PORTBCDE_IRQHandler()
{
	fade_counter = 0;
	//setzeFarbe(255, 0, 0);
	int ton1[3] = { rand() % 256, rand() % 256, rand() % 256 };
	int ton2[3] = { rand() % 256, rand() % 256, rand() % 256 };
	int ton3[3] = { rand() % 256, rand() % 256, rand() % 256 };
	for (int i = 0; i < 3; i++)
	{
		farbe1[i] = ton1[i];
		farbe2[i] = ton2[i];
		farbe3[i] = ton3[i];
	}
	dreiFarbenFade();
	//Delay();
	//Delay();
	PORTC_BASE_PTR->ISFR = 0x2; 
	NVIC_ClearPendingIRQ(PORTBCDE_IRQn);
}


/*int main_alt()
{

	RGBinit();
	

	for(;;)
	{
		setzeFarbe(i/2, i/2, 0);
		Delays((256-i)*2);
		i++;
		i = i % 256;
		if (i == 0)
		{
			for (int j = 0; j < 256; j++)
			{
				setzeFarbe((256 - j)/2, (256 - j)/2, 0);
				Delays(j*2);
			}
			i++;
		}
		
		
	}

	return 0;
}*/


int main()
{

	RGBinit();
	PORTA_BASE_PTR->PCR[4] |= PORT_PCR_IRQC(0b1011);
	PORTC_BASE_PTR->PCR[1] |= PORT_PCR_IRQC(0b1011);
	NVIC_EnableIRQ(PORTBCDE_IRQn);
	NVIC_EnableIRQ(PORTA_IRQn);

	//keine Farbe zu Beginn
	TPM2_C0V = 0;
	TPM2_C1V = 0;
	TPM1_C1V = 0;

	for (;;)
	{	
		/*setzeFarbe(i, 0, 0);
		Delays((256 - i) * 2);
		i++;
		i = i % 256;
		while (i == 0)
		{
			k = 1;
			setzeFarbe(255, j, k);
			Delays((256 - j) * 2);
			j++;
			j = j % 256;
			while (j == 0 && k!=0)
			{
				setzeFarbe(255, 255, k);
				Delays((256 - k) * 2);
				k++;
				k = k % 256;
				i = 1; 
			}*/
		//Schrittweite für die Farbtoene festlegen
		/*int srot = (wrot - TPM1_C0V) / 100;
		int sblau = (wblau - TPM2_C1V) / 100;
		int sgruen = (wgreen - TPM1_C1V) / 100;

		int i = 0;
		while (i < 101)
		{	
			setzeFarbe((rot + i*srot), (gruen + i*sgruen), (blau + i*sblau));
			Delay();
			i++;
		}*/
		//Festlegen der Farbtoene, zwischen denen gewechselt werden soll.
		int ton1[3] = {250, 0, 0};
		int ton2[3] = {0, 0, 0};
		int ton3[3] = {250, 250, 250};
		for (int i = 0; i < 3; i++)
		{
			farbe1[i] = ton1[i];
			farbe2[i] = ton2[i];
			farbe3[i] = ton3[i];
		}
		dreiFarbenFade();
			
	}
	return 0;
}