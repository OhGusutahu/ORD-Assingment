#ifndef LED_H_INCLUDED
#define LED_H_INCLUDED

/**
 * Este é a interface (header) que descreve um TAD de uma Lista de Espaços Disponíveis
 * Essa LED utiliza a estratégia worst-fit
 * Feito por Gustavo Giozeppe Bulgarelli
**/

typedef struct reg *registro;

struct reg{
    int offset; // offset do registro inserido
    int tam_reg; // tamanho do registro
    registro prox; // próximo registro na LED
};

typedef struct LED LED;

struct LED {
    registro cabeca; // cabeça da LED
    int tam; // tamanho da LED
};

LED* cria_led();
/* Cria a LED inicializando todos os dados necessários para seu gerenciamento */

int inserir_led(LED *led, registro x);
/* Insere o registro x na LED
   Retorna 1 em caso de sucesso e 0 caso contrário
*/

registro remover_led(LED *led);
/* Remove a cabeça da LED para inserção no arquivo
   Retorna 1 em caso de sucesso e 0 caso contrário
*/

void imprimir_led(LED *led);
/* Imprime os offsets dos espaços disponíveis que estão encadeados na LED */

void destruir_led(LED *led);
/* Libera a memória alocada pela LED */

#endif // LED_H_INCLUDED