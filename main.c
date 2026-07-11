#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char nome[40];
    unsigned int largura;
    unsigned int comp;
} dados_inicial;

void erosao(unsigned char **matriz, unsigned char **matrizErosao, unsigned int linhas, unsigned int colunas){
    for (unsigned int i = 1; i < linhas-1; i++) {
        for(unsigned int j = 1; j < colunas-1; j++){

            bool tudo = true;

            for(int x = -1; x < 2; x++) {
                for(int y = 0; j < 2; y++) {
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

void dilatacao(unsigned int **matriz2, unsigned int linhas, unsigned int colunas) {
    for (unsigned int i = 1; i < linhas-1; i++) {
        for(unsigned int j = 1; j < colunas-1; j++){

            bool tudo = false;

            for(int x = -1; x < 2; x++) {
                for(int y = 0; j < 2; y++) {
                    if(matriz2[i + x][j + y] == 1) {
                        tudo = true;
                        break;
                    }
                }
                if(tudo == true)
                    break;
            }

            if(tudo == true) {
                matriz2[i][j] = 1;
            } else {
                matriz2[i][j] = 0;
            }
        }
    }
}

void dilatacaoMD(unsigned int **matrizLimpa, unsigned int **matrizMapeada, unsigned int linhas, unsigned int colunas) {
    for(unsigned int i = 0; i < linhas; i++) {
        for(unsigned int j = 2; j < colunas - 3; j++) {
            bool um = false;

            for(int x = -3; x < 4; x++){
                if(matrizLimpa[i][j+x] == 1){
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


int main(int argc, char* argv[]) {
    FILE* arqfonte = fopen(argv[1], "r");
    FILE* arqsaida = fopen(argv[2], "w");
    if (!arqfonte) {
        fprintf(stderr, "Erro ao abrir arquivo de entrada.\n");
        return 1;
    }
    if (!arqsaida) {
        fprintf(stderr, "Erro ao abrir arquivo de entrada.\n");
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
    
    for (unsigned int i = 0; i < linhas; i++) {
        matriz[i] = (unsigned char *)malloc(colunas * sizeof(unsigned char));
        matrizLimpa[i] = (unsigned char *)malloc(colunas * sizeof(unsigned char));
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
    dilatacao(matrizLimpa, linhas, colunas);

    for (unsigned int i = 0; i < linhas; i++) {
        for (unsigned int j = 0; j < colunas; j++) {
            matrizMapeada[i][j] = matrizLimpa[i][j];
        }
    }

    dilatacaoMD(matrizLimpa, matrizMapeada, linhas, colunas);



    return 0;
}