/**
 * TRABALHO 1 de Organização e Recuperação de Dados
 * 
 * Docente: Paulo Roberto de Oliveira
 * Discente: Gustavo Giozeppe Bulgarelli (RA 129658)
 * 
 * Programa de Busca, Inserção e Remoção de registros
 * em um arquivos de dados em binário (dados.dat)
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LED/LED.h"

#define NOME_ARQ_DADOS "dados.dat"
#define LIMIAR_INSERCAO 10

typedef struct reg *registro; /* Tipo exportado */
typedef struct LED LED; /* Tipo exportado */

typedef struct {
    char op_tipo; // b (busca), i (inserção) ou r (remoção)
    char op_info[512]; // as informações da operação
} operacao;

operacao op_leitura(FILE *op_arq);
// operações a serem executadas
int busca_reg(char *IDENTIFICADOR, FILE *arq_dados);
void insercao_reg(char *novo_reg, FILE *arq_dados);
void remocao_reg(char *IDENTIFICADOR, FILE *arq_dados);

void executa_operacoes(FILE *arq_dados, FILE *op_arq);
void imprime_led(FILE *arq_dados);

// auxiliares
void le_led(LED *led, FILE *arq_dados);
void escreve_led(LED *led, FILE *arq_dados);
char fpeek(FILE *arq_dados);

int main(int argc, char *argv[])
{
    FILE *arq_dados = fopen(NOME_ARQ_DADOS, "rb+");

    if(arq_dados == NULL) {
        // caso dados.dat não possa ser encontrado
        fprintf(stderr, "Não foi possivel encontrar o arquivo de dados. Tente novamente.");
        exit(EXIT_FAILURE);
    }

    if (argc == 3 && strcmp(argv[1], "-e") == 0) {
        printf("Modo de execucao de operacoes ativado ... nome do arquivo = %s\n", argv[2]);
        FILE *op_arq = fopen(argv[2], "r");
        // sem if(op_arq == NULL) considerando não necessário a verificação da integridade do arquivo
        executa_operacoes(arq_dados, op_arq);
        fclose(op_arq);

    } else if (argc == 2 && strcmp(argv[1], "-p") == 0) {
        printf("Modo de impressao da LED ativado ...\n");
        imprime_led(arq_dados);

    } else {
        fprintf(stderr, "Argumentos incorretos!\n");
        fprintf(stderr, "Modo de uso:\n");
        fprintf(stderr, "$ %s -e nome_arquivo\n", argv[0]);
        fprintf(stderr, "$ %s -p\n", argv[0]);
        exit(EXIT_FAILURE);

    }

    fclose(arq_dados);
    return 0;
}

operacao op_leitura(FILE *op_arq)
{
    // lê no arquivo de operações a operação a ser realizada
    operacao op;

    op.op_tipo = fgetc(op_arq);

    fseek(op_arq,1,SEEK_CUR);
    // setando o ponteiro L/E na posição da informação da operação
    fgets(op.op_info, 512, op_arq);
    if(op.op_info[strlen(op.op_info) - 1] == '\n') op.op_info[strlen(op.op_info) - 1] = '\0';
    // remover '\n' da informação da operação para não afetar a leitura
    else op.op_info[strlen(op.op_info)] = '\0';
    return op;
}

char fpeek(FILE *arq_dados)
{
    // função auxiliar para busca
    char c = fgetc(arq_dados);
    ungetc(c, arq_dados);

    return c;
}

int busca_reg(char *IDENTIFICADOR, FILE *arq_dados)
{
    // busca de registro através da chave primária
    // retorna a posição do registro no arquivo ou 0 caso não encontre
    fseek(arq_dados, sizeof(int), SEEK_SET); // passar cabeçalho
    short tamanho; // tamanho do registro
    int i = 0;
    char c, chave_prim[64], buffer[512];

    while(fpeek(arq_dados) != EOF) {
        fread(&tamanho, sizeof(short), 1, arq_dados);
        while(c = fgetc(arq_dados), c != '|' && c != '*') { // se c == '*' então o registro foi removido
            chave_prim[i] = c;
            i++;
        }
        if(c == '*') chave_prim[0] = '\0'; // se c == '*' então "limpa" chave_prim
        chave_prim[++i] = '\0';
        if(strcmp(chave_prim, IDENTIFICADOR) == 0) {
            return ftell(arq_dados) - i - 2; // retorna a posição do registro
        } else {
            fseek(arq_dados, tamanho - i, SEEK_CUR); // L/E vai para o próximo registro
        }
    }
    return 0;
}

void insercao_reg(char *novo_reg, FILE *arq_dados)
{
    // insere o resgistro no arquivo de dados
    short novo_tam = strlen(novo_reg);
    printf("\nInsercao do registro de chave \"%s\" (%i bytes)", novo_reg, novo_tam);
    LED *led;
    led = cria_led();
    le_led(led, arq_dados);

    if(led->cabeca->offset == -1 || led->cabeca->tam_reg < novo_tam) {
        // caso a LED esteja vazia ou o novo registro for maior do que os espaços livres
        fseek(arq_dados, 0, SEEK_END);
        fwrite(novo_tam, sizeof(short), 1, arq_dados);
        fwrite(novo_reg, novo_tam, 1, arq_dados);
        printf("\nLocal: Fim do arquivo");
    } else {
        fseek(arq_dados, led->cabeca->offset, SEEK_SET);
        if((led->cabeca->tam_reg - novo_tam) <= LIMIAR_INSERCAO) {
            // se o espaço que sobrar for menor ou igual que o limiar de excessão
            // não precisa da reinserção na LED
            fwrite(novo_tam, sizeof(short), 1, arq_dados);
            fwrite(novo_reg, novo_tam, 1, arq_dados);
            fwrite('\0', sizeof(char), (led->cabeca->tam_reg - novo_tam), arq_dados); // '\0' na fragmentação
            printf("Tamanho de espaco reutilizado: %i bytes", led->cabeca->tam_reg);
            printf("Local: offset = %i bytes (0x%x)\n", led->cabeca->offset, led->cabeca->offset);

            remover_led(led);
            escreve_led(led, arq_dados); // escreve a nova LED no arquivo
        } else {
            // reinserção na LED
            fwrite(novo_tam, sizeof(short), 1, arq_dados);
            fwrite(novo_reg, novo_tam, 1, arq_dados);
            printf("Tamanho de espaco reutilizado: %i bytes (Sobre de %i bytes)", led->cabeca->tam_reg, (led->cabeca->tam_reg - novo_tam));
            printf("Local: offset = %i bytes (0x%x)\n", led->cabeca->offset, led->cabeca->offset);

            short fragmentacao = led->cabeca->tam_reg - novo_tam;
            registro frag;
            frag->offset = led->cabeca->offset + sizeof(short) + novo_tam; // cabeça da LED + offset do novo reg + tamanho do novo reg
            frag->tam_reg = fragmentacao;
            // salva o offset e o tamanho da fragmentação em uma struct registro
            
            inserir_led(led, frag); // insere a fragmentação de volta para a LED
            escreve_led(led, arq_dados); // escreve a nova LED no arquivo
        }
    }

    destruir_led(led);
}

void escreve_led(LED *led, FILE *arq_dados)
{
    // função auxiliar para escrever a LED no arquivo
    registro led_reg;
    led_reg = led->cabeca;
    fseek(arq_dados, 0, SEEK_SET);
    fwrite(&led_reg->offset, sizeof(int), 1, arq_dados); // escreve a cabeça da LED no cabeçalho

    while(led_reg->offset != -1) {
        fseek(arq_dados, (led_reg->offset + sizeof(short)), SEEK_SET);
        fputc('*', arq_dados);
        fwrite(&led_reg->prox->offset, sizeof(short), 1, arq_dados);
        // insere o offset do próximo registro na LED
        
        led_reg = led_reg->prox;
    }
}

void remocao_reg(char *IDENTIFICADOR, FILE *arq_dados)
{
    // remove o resgistro através da chave primária
    printf("\nRemocao do registro de chave \"%s\"", IDENTIFICADOR);
    LED *led;
    led = cria_led();
    le_led(led, arq_dados);

    int pos = busca_reg(IDENTIFICADOR, arq_dados);
    if(pos >= sizeof(int)) { // menor que o cabeçalho (4 bytes) quer dizer que o registro não foi encontrado
        short tam;
        fseek(arq_dados, pos, SEEK_SET);
        fread(&tam, sizeof(short), 1, arq_dados); // armazena o tamanho do registro

        fseek(arq_dados, pos+sizeof(short), SEEK_SET);
        fputc('*', arq_dados); // adiciona '*' para remover logicamente

        registro reg; // salva a struct do registro
        reg->offset = pos;
        reg->tam_reg = tam;
        inserir_led(led,reg); // insere na LED
        escreve_led(led, arq_dados); // insere a nova LED no arquivo

        printf("\nRegistro removido! (%i bytes)\n", tam);
        printf("Local: offset = %i bytes (0x%x)\n", pos, pos);
    } else printf("\nErro: registro nao encontrado!\n");

    destruir_led(led);
}

void executa_operacoes(FILE *arq_dados, FILE *op_arq)
{
    // executa as operações no arquivo de operações
    operacao op;
    while(op  = op_leitura(op_arq), op.op_tipo != EOF) {
        switch(op.op_tipo) {
            case 'b':
                // operação não completa em função para usar busca_reg em remocao_reg
                printf("\nBusca pelo registro de chave \"%s\"", op.op_info);
                int pos;
                short tam;
                char buffer[512];
                pos = busca_reg(op.op_info, arq_dados);
                
                if(pos >= sizeof(int)) { // menor que o cabeçalho (4 bytes) quer dizer que o registro não foi encontrado
                    fseek(arq_dados, pos, SEEK_SET);
                    fread(&tam, sizeof(short), 1, arq_dados);
                    fread(buffer, sizeof(char), tam, arq_dados);
                    printf("\n%s (%i bytes)\n", buffer, strlen(buffer));
                } else printf("\nErro: registro nao encontrado!\n");
                break;
            // case 'i':
            //     insercao_reg(op.op_info, arq_dados);
            //     break;
            case 'r':
                remocao_reg(op.op_info, arq_dados);
                break;
            // sem default considerando não necessário a verificação da integridade do arquivo
        }
    }
}

void le_led(LED *led, FILE *arq_dados)
{
    // função auxiliar para ler a LED armazenada no arquivo
    int pos_reg;
    registro reg;
    fseek(arq_dados, 0, SEEK_SET);

    fread(&pos_reg, sizeof(int), 1, arq_dados);
    while(pos_reg > sizeof(int)) {
        fseek(arq_dados, pos_reg, SEEK_SET); // L/E na posição do registro removido
        reg->offset = pos_reg; // salva o offset da cabeça
        fread(&reg->tam_reg, sizeof(short), 1, arq_dados); // salva o tamanho do registro;
        inserir_led(led, reg); // insere o registro removido na LED

        fseek(arq_dados, 1, SEEK_CUR); // passa pelo '*'

        fread(&pos_reg, sizeof(int), 1, arq_dados);
    }
}

void imprime_led(FILE *arq_dados)
{
    // imprime as informações na LED do arquivo de dados
    LED *led;
    led = cria_led();
    le_led(led, arq_dados);
    imprimir_led(led);

    destruir_led(led);
}