#include <stdio.h>
#include <stdlib.h>
#include "LED.h"

typedef struct reg *registro; /* Tipo exportado */

typedef struct LED LED; /* Tipo exportado */

LED* cria_led()
{
    LED *led = (LED *) malloc(sizeof(LED));
    led->cabeca->offset = -1;
    led->cabeca->tam_reg = 0;
    led->cabeca->prox = NULL;
    led->tam = 0;
    return led;
}

int inserir_led(LED *led, registro x)
{
    if(led == NULL) {
        printf("LED não existe!");
        return 0;
    } else {
        if(led->tam == 0 || x->tam_reg > led->cabeca->tam_reg) {
            // inserção em LED vazia ou se tam_reg for maior que a cabeça
            x->prox = led->cabeca;
            led->cabeca = x;
        }
        else {
            // inserção em LED não vazia com tam_reg menor que a cabeça
            registro pos, antes;
            antes = led->cabeca;
            while(pos!=NULL && pos->tam_reg > x->tam_reg) {
                pos = antes;
                antes = antes->prox;
            }
            x->prox = pos;
            antes->prox = x;
        }
        return 1;
    }
}

registro remover_led(LED *led)
{
    if(led->cabeca == NULL) return 0; // LED está vazia
    else {
        // remove a cabeça da LED
        registro aux = led->cabeca;
        led->cabeca = aux->prox;
        led->tam--;
        return aux;
    }
}

void imprimir_led(LED *led)
{
    printf("\n\n LED -> ");
    registro pos = led->cabeca;
    while(pos != NULL) {
        printf("[offset: %d, tam: %d] ->", pos->offset, pos->tam_reg);
        pos = pos->prox;
    }
    printf("[offset: -1]");
    printf("\nTotal: %d espacos disponiveis", led->tam);
}

void destruir_led(LED *led)
{
    registro pos = led->cabeca;
    while(pos != NULL) {
        led->cabeca = pos->prox;
        free(pos);
        pos = led->cabeca;
    }
    free(led);
}