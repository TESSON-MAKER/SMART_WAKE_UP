#include "ds3231.h"

#define AF4 0x04
#define TIMEOUT_MAX 300

uint8_t DS3231_HS = 0;

void DS3231_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    
    GPIOB->MODER |= GPIO_MODER_MODER8_1;
    GPIOB->MODER &= ~GPIO_MODER_MODER8_0;
    
    GPIOB->MODER |= GPIO_MODER_MODER9_1;
    GPIOB->MODER &= ~GPIO_MODER_MODER9_0;
    
    GPIOB->OTYPER |= GPIO_OTYPER_OT8;
    GPIOB->OTYPER |= GPIO_OTYPER_OT9;
    
    GPIOB->AFR[1] |= (AF4<<0)|(AF4<<4);

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = 0x0000C1C1;
}

int DS3231_BCD_DEC(unsigned char x) {
    return x - 6 * (x >> 4); // Conversion du BCD en decimal
}

int DS3231_DEC_BCD(unsigned char x) {
    return x + 6 * (x / 10); // Conversion du decimal en BCD
}

void DS3231_Read(uint8_t slav_add, uint8_t memadd, uint8_t *data, uint8_t length ) {
    DS3231_HS = 0;

    // Activation du bus I2C1
    I2C1->CR1 |= I2C_CR1_PE;

    // Configuration des registres pour la lecture
    I2C1->CR2 = 0; // Reinitialisation du registre CR2
    I2C1->CR2 = (slav_add << 1); // Configuration de l'adresse de l'esclave (decalage de 1 bit)

    I2C1->CR2 &= ~I2C_CR2_ADD10; // Effacement du bit ADD10 pour une adresse sur 7 bits
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos); // Definition du nombre d'octets a transmettre
    I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Definition de l'operation en mode ecriture
    I2C1->CR2 &= ~I2C_CR2_AUTOEND; // Desactivation de l'arret automatique
    I2C1->CR2 |= I2C_CR2_START; // Generation d'un signal de depart (adresse du debut inclus)

    // Attente de la fin de la transmission
    uint16_t timeout = 0;
    while(!(I2C1->ISR & I2C_ISR_TC) && timeout < TIMEOUT_MAX) {
        if(I2C1->ISR & I2C_ISR_TXE) // Verification si le buffer de transmission est vide
            I2C1->TXDR = memadd; // Transmission de l'adresse memoire
        timeout++; //Increment de 1 a chaque passage
    }
    
    if (timeout >= (TIMEOUT_MAX - 10)) 
        DS3231_HS = 1;
    else {
        // Desactivation du bus I2C1
        I2C1->CR1 &= ~I2C_CR1_PE;

        /*------------------LECTURE DES DONNEES--------------------------------*/

        // Activation du bus I2C1
        I2C1->CR1 |= I2C_CR1_PE; 

        I2C1->CR2 = 0; // Reinitialisation du registre CR2
        I2C1->CR2 = (slav_add << 1); // Configuration de l'adresse de l'esclave (decalage de 1 bit)
        I2C1->CR2 |= I2C_CR2_RD_WRN; //Passage en mode lecture
        I2C1->CR2 |= (length << I2C_CR2_NBYTES_Pos);
        I2C1->CR2 |= I2C_CR2_AUTOEND;
        I2C1->CR2 |= I2C_CR2_START;

        // Attente de la fin de la reception
        while(!(I2C1->ISR & I2C_ISR_STOPF)) // Boucle d'attente de la condition STOPF (Stop detection flag)
            if(I2C1->ISR & I2C_ISR_RXNE) // Verification si le buffer de reception est non vide
                *data++ = I2C1->RXDR; // Lecture des donnees recues

        // Desactivation du bus I2C1
        I2C1->CR1 &= ~I2C_CR1_PE;
    }
}

/*-----------------------------------------------------------------------------------------------------------------*/

void DS3231_WriteMemory(uint8_t slav_add, uint8_t memadd, uint8_t *data, uint8_t length) {
    // Activation du bus I2C1
    I2C1->CR1 |= I2C_CR1_PE;
    I2C1->CR2 = 0; // Reinitialisation du registre CR2
    I2C1->CR2 = (slav_add << 1);
    I2C1->CR2 &= ~I2C_CR2_ADD10;
    I2C1->CR2 |= ((length + 1) << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR2 |= I2C_CR2_START;
    
    // Attente de la fin de la transmission
    int i = 0; // Correction : initialiser la variable i
    while(!(I2C1->ISR & I2C_ISR_STOPF)) {
        if(I2C1->ISR & I2C_ISR_TXE) {
            I2C1->TXDR = (i == 0) ? memadd : data[i - 1]; // Envoi des donnees
            i++;
        }
    }
    
    // Desactivation du bus I2C1
    I2C1->CR1 &= ~I2C_CR1_PE;
}
