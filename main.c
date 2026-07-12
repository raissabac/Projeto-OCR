#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

//registrar as coordenadas das imagens 
typedef struct {
    unsigned int minX;
    unsigned int maxX;
    unsigned int minY;
    unsigned int maxY;
} Retangulo;

typedef struct {
    Retangulo *palavras;
    unsigned int quant;
    unsigned int capacidade;
} Geral;

void erosao(unsigned char **matriz, unsigned char **matrizErosao, unsigned int linhas, unsigned int colunas){
    for (unsigned int i = 1; i < linhas-1; i++) {
        for(unsigned int j = 1; j < colunas-1; j++){

            bool tudo = true;

            for(int x = -1; x < 2; x++) {
                for(int y = -1; y < 2; y++) {
                    if(matriz[i + x][j + y] == 0) {
                        tudo = false;
                        break;
                    }
                }
                if(tudo == false)
                    break;
            }

            if(tudo == true) {
                matrizErosao[i][j] = 1;
            } else {
                matrizErosao[i][j] = 0;
            }
        }
    }
}

void dilatacao(unsigned char **matriz2, unsigned char **matrizDilatacao, unsigned int linhas, unsigned int colunas) {
    for (unsigned int i = 1; i < linhas-1; i++) {
        for(unsigned int j = 1; j < colunas-1; j++){

            bool tudo = false;

            for(int x = -1; x < 2; x++) {
                for(int y = -1; y < 2; y++) {
                    if(matriz2[i + x][j + y] == 1) {
                        tudo = true;
                        break;
                    }
                }
                if(tudo == true)
                    break;
            }

            if(tudo == true) {
                matrizDilatacao[i][j] = 1;
            } else {
                matrizDilatacao[i][j] = 0;
            }
        }
    }
}

void dilatacaoMD(unsigned char **matrizDilatacao, unsigned char **matrizMapeada, unsigned int linhas, unsigned int colunas) {
    for(unsigned int i = 0; i < linhas; i++) {
        for(unsigned int j = 2; j < colunas - 3; j++) {
            bool um = false;

            for(int x = -3; x < 4; x++){
                if(matrizDilatacao[i][j+x] == 1){
                    um = true;
                    break;
                }
            }
            
            if(um == true) {
                matrizMapeada[i][j] = 1;
            } else {
                matrizMapeada[i][j] = 0;
            }
        }
    }
}

void algoritmoRetangulos(unsigned char **matrizMapeada, unsigned int linhas, unsigned int colunas, Geral *listaPalavras) {
    // Fila estática declarada fora para não pesar a memória recriando toda hora
    unsigned int *filaX = malloc(linhas * colunas * sizeof(unsigned int)); 
    unsigned int *filaY = malloc(linhas * colunas * sizeof(unsigned int));

    for (unsigned int i = 1; i < linhas - 1; i++) {
        for (unsigned int j = 1; j < colunas - 1; j++) {
            
            if (matrizMapeada[i][j] == 1) {
                
                unsigned int minX = i, maxX = i;
                unsigned int minY = j, maxY = j;

                int inicio = 0;
                int fim = 0;

                filaX[fim] = i; 
                filaY[fim] = j;
                fim++;
                matrizMapeada[i][j] = 0; // Apaga o rastro inicial

                while (inicio < fim) {
                    unsigned int atualX = filaX[inicio];
                    unsigned int atualY = filaY[inicio];
                    inicio++;

                    // Atualiza as coordenadas do retângulo!
                    if (atualX < minX) minX = atualX;
                    if (atualX > maxX) maxX = atualX;
                    if (atualY < minY) minY = atualY;
                    if (atualY > maxY) maxY = atualY;
                    
                    // Olha vizinho de CIMA
                    if (matrizMapeada[atualX - 1][atualY] == 1) {
                        filaX[fim] = atualX - 1;
                        filaY[fim] = atualY;
                        fim++;
                        matrizMapeada[atualX - 1][atualY] = 0; 
                    }
                    // Olha vizinho de BAIXO
                    if (matrizMapeada[atualX + 1][atualY] == 1) {
                        filaX[fim] = atualX + 1;
                        filaY[fim] = atualY;
                        fim++;
                        matrizMapeada[atualX + 1][atualY] = 0; 
                    }
                    // Olha vizinho da ESQUERDA
                    if (matrizMapeada[atualX][atualY - 1] == 1) {
                        filaX[fim] = atualX;
                        filaY[fim] = atualY - 1;
                        fim++;
                        matrizMapeada[atualX][atualY - 1] = 0; 
                    }
                    // Olha vizinho da DIREITA
                    if (matrizMapeada[atualX][atualY + 1] == 1) {
                        filaX[fim] = atualX;
                        filaY[fim] = atualY + 1;
                        fim++;
                        matrizMapeada[atualX][atualY + 1] = 0; 
                    }
                } 

                if(listaPalavras->quant == listaPalavras->capacidade) {
                    listaPalavras->capacidade *= 2;
                    listaPalavras->palavras = realloc(listaPalavras->palavras, sizeof(Retangulo) * listaPalavras->capacidade);
                }

                listaPalavras->palavras[listaPalavras->quant].minX = minX;
                listaPalavras->palavras[listaPalavras->quant].maxX = maxX;
                listaPalavras->palavras[listaPalavras->quant].minY = minY;
                listaPalavras->palavras[listaPalavras->quant].maxY = maxY;

                listaPalavras->quant++;
            }
        }
    }

    free(filaX);
    free(filaY);
}


int main(int argc, char* argv[]) {
    
    FILE* arqfonte = fopen(argv[1], "r");
    FILE* arqsaida = fopen(argv[2], "w");
    if (!arqfonte) {
        fprintf(stderr, "Erro ao abrir arquivo de entrada.\n");
        return 1;
    }
    if (!arqsaida) {
        fprintf(stderr, "Erro ao abrir arquivo de saida.\n");
        return 1;
    }

    char linha[150];
    unsigned int linhas = 0;
    unsigned int colunas = 0;

    fgets(linha, sizeof(linha), arqfonte);
    fgets(linha, sizeof(linha), arqfonte);
    fscanf(arqfonte, "%u %u", &colunas, &linhas);

    unsigned char **matriz = (unsigned char **)malloc(linhas * sizeof(unsigned char *));
    unsigned char **matrizLimpa = (unsigned char **)malloc(linhas * sizeof(unsigned char *));
    unsigned char **matrizMapeada = (unsigned char **)malloc(linhas * sizeof(unsigned char *));
    unsigned char **matrizDilatacao = (unsigned char **)malloc(linhas * sizeof(unsigned char *));
    
    for (unsigned int i = 0; i < linhas; i++) {
        matriz[i] = (unsigned char *)malloc(colunas * sizeof(unsigned char));
        matrizLimpa[i] = (unsigned char *)malloc(colunas * sizeof(unsigned char));
        matrizMapeada[i] = (unsigned char *)malloc(colunas * sizeof(unsigned char));
        matrizDilatacao[i] = (unsigned char *)malloc(colunas * sizeof(unsigned char));
    }

    unsigned int a; 
    for(unsigned int i = 0; i < linhas; i++) {
        for(unsigned int j = 0; j < colunas; j++) {
            fscanf(arqfonte, "%u", &a); 
            matriz[i][j] = (unsigned char)a; // Faz o cast e salva na matriz
            if(i == 0 || i == linhas - 1 || j == 0 || j == colunas - 1){
                matrizLimpa[i][j] = (unsigned char)a;
            }
        }
    }
    
    erosao(matriz, matrizLimpa, linhas, colunas);
    dilatacao(matrizLimpa, matrizDilatacao, linhas, colunas);

    dilatacaoMD(matrizDilatacao, matrizMapeada, linhas, colunas);

    Geral *listaPalavras = malloc(sizeof(Geral));
    listaPalavras->capacidade = 64;
    listaPalavras->palavras = malloc(sizeof(Retangulo) * listaPalavras->capacidade);
    listaPalavras->quant = 0;

    algoritmoRetangulos(matrizMapeada, linhas, colunas, listaPalavras);

    return 0;
}