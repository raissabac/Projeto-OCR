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

void erosao(unsigned char **matriz, unsigned char **matrizErosao, unsigned int altura, unsigned int largura){
    for (unsigned int i = 1; i < altura-1; i++) {
        for(unsigned int j = 1; j < largura-1; j++){

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

//fazer a dilatacao, adição dos campos int mX e int mY para poder utilizar a função com qualquer mascara
void dilatacao(unsigned char **matriz2, unsigned char **matrizLimpa, unsigned int altura, unsigned int largura, int mX, int mY) {
    for (unsigned int i = 1; i < altura-1; i++) {
        for(unsigned int j = 1; j < largura-1; j++){

            bool tudo = false;

            for(int x = -(mX/2); x < (mX/2) + 1; x++) {
                for(int y = -(mY/2); y < (mY/2) + 1; y++) {
                    if(matriz2[i + x][j + y] == 1) {
                        tudo = true;
                        break;
                    }
                }
                if(tudo == true)
                    break;
            }

            if(tudo == true) {
                matrizLimpa[i][j] = 1;
            } else {
                matrizLimpa[i][j] = 0;
            }
        }
    }
}

//dilatação morfologica direcionada horizontal
void dilatacaoMDH(unsigned char **matrizLimpa, unsigned char **matrizMapeada, unsigned int altura, unsigned int largura, int mY) {
    int val = mY / 2;

    for(unsigned int i = 0; i < altura; i++) {
        for(unsigned int j = 2; j < largura - 3; j++) {
            bool um = false;

            for(int x = -val; x < (mY - 1 - val); x++){
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

//dilatação morfologica direcionada vertical
void dilatacaoMDV(unsigned char **matrizLimpa, unsigned char **matrizMapeada, unsigned int altura, unsigned int largura, int mX) {
    int val = mX / 2;

    for(unsigned int i = 0; i < altura; i++) {
        for(unsigned int j = 2; j < largura - 3; j++) {
            bool um = true;

            for(int x = -val; x < (mX - 1 - val); x++){
                if(matrizLimpa[i][j+x] == 0){
                    um = false;
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

//faz a extração de componentes conexos 
void algoritmoRetangulos(unsigned char **matrizMapeada, unsigned int altura, unsigned int largura, Geral *listaPalavras) {
    // Fila estática declarada fora para não pesar a memória recriando toda hora
    unsigned int *filaX = malloc(altura * largura * sizeof(unsigned int)); 
    unsigned int *filaY = malloc(altura * largura * sizeof(unsigned int));

    for (unsigned int i = 1; i < altura - 1; i++) {
        for (unsigned int j = 1; j < largura - 1; j++) {
            
            //encontra um endereço da matriz igual a 1
            if (matrizMapeada[i][j] == 1) {
                
                unsigned int minX = i, maxX = i;        //registra os valores maximos e minimos
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

void destacandoPalavras(unsigned char **matrizLimpa, Geral *listaPalavras){
    unsigned int minX = 0;
    unsigned int minY = 0;
    unsigned int maxX = 0;
    unsigned int maxY = 0;

    for(unsigned int ind = 0; ind < listaPalavras->quant; ind++) {
        minX = listaPalavras->palavras[ind].minX;
        minY = listaPalavras->palavras[ind].minY;
        maxX = listaPalavras->palavras[ind].maxX;
        maxY = listaPalavras->palavras[ind].maxY;

        for(unsigned int j = minY; j <= maxY; j++){
            matrizLimpa[minX][j] = 1;
            matrizLimpa[maxX][j] = 1;
        }

        for(unsigned int i = minX + 1; i < maxX; i++) {
            matrizLimpa[i][minY] = 1;
            matrizLimpa[i][maxY] = 1;
        }
    }
}

void imprimirImagem(unsigned char **matrizLimpa, FILE* arqsaida, unsigned int altura, unsigned int largura) {
    fprintf(arqsaida, "P1\n");
    fprintf(arqsaida, "%u %u\n", altura, largura);

    for(unsigned int i = 0; i < altura; i++) {
        for(unsigned int j = 0; j < largura; j++) {
            fprintf(arqsaida, "%d ", matrizLimpa[i][j]);
        }
        fprintf(arqsaida, "\n");
    }
}

int main(int argc, char* argv[]) {
    
    //leitura do arquivo fonte passado via terminal e do arquivo que vai colocar a saída
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
    unsigned int altura = 0;
    unsigned int largura = 0;

    fgets(linha, sizeof(linha), arqfonte);
    fgets(linha, sizeof(linha), arqfonte);
    fscanf(arqfonte, "%u %u", &largura, &altura);

    unsigned char **matriz = (unsigned char **)malloc(altura * sizeof(unsigned char *));            //matriz lida do arquivo
    unsigned char **matrizErosao = (unsigned char **)malloc(altura * sizeof(unsigned char *));       //matriz depois de aplicar erosao
    unsigned char **matrizLimpa = (unsigned char **)malloc(altura * sizeof(unsigned char *));   //matriz depois de aplicar dilatacao
    unsigned char **matrizMapeada = (unsigned char **)malloc(altura * sizeof(unsigned char *));     //matriz depois de aplicar dilatacao MD
    unsigned char **matrizContagem = (unsigned char **)malloc(altura * sizeof(unsigned char *));    //matriz depois de aplicar a dilatacao para contagem de colunas e linhas

    
    for (unsigned int i = 0; i < altura; i++) {
        matriz[i] = (unsigned char *)malloc(largura * sizeof(unsigned char));
        matrizErosao[i] = (unsigned char *)malloc(largura * sizeof(unsigned char));
        matrizLimpa[i] = (unsigned char *)malloc(largura * sizeof(unsigned char));
        matrizMapeada[i] = (unsigned char *)malloc(largura * sizeof(unsigned char));
        matrizContagem[i] = (unsigned char *)malloc(largura * sizeof(unsigned char*));
    }

    //começa fazendo a leitura do arquivo e colocando todos os valores na matriz e na matriz limpa (que vai conservar as bordas)
    unsigned int a; 
    for(unsigned int i = 0; i < altura; i++) {
        for(unsigned int j = 0; j < largura; j++) {
            fscanf(arqfonte, "%u", &a); 
            matriz[i][j] = (unsigned char)a; // Faz o cast e salva na matriz
            if(i == 0 || i == altura - 1 || j == 0 || j == largura - 1){
                matrizErosao[i][j] = (unsigned char)a;
            }
        }
    }

    //pré-processamento - remoção de ruídos
    erosao(matriz, matrizErosao, altura, largura);
    dilatacao(matrizErosao, matrizLimpa, altura, largura, 3, 3);

    //mapeamento das palavras - transformamos as palavras em blocos sólidos
    dilatacaoMD(matrizLimpa, matrizMapeada, altura, largura);

    //estrutura para armazenar as informações do número de palavras
    Geral *listaPalavras = malloc(sizeof(Geral));
    listaPalavras->capacidade = 64;
    listaPalavras->palavras = malloc(sizeof(Retangulo) * listaPalavras->capacidade);
    listaPalavras->quant = 0;

    //utiliza o algoritmo flood fill adaptado para poder percorrer a matriz e identificar o número de palavras
    algoritmoRetangulos(matrizMapeada, altura, largura, listaPalavras);

    //vamos utilizar o tamanho da mascara no calculo da linhas como 20% do valor da altura da imagem 
    int tamanhoL = altura * 0.20;
    Geral *ContagemLinhas = malloc(sizeof(Geral));
    ContagemLinhas->capacidade = 64;
    ContagemLinhas->quant = 0;
    ContagemLinhas->palavras = malloc(sizeof(Retangulo) * ContagemLinhas->capacidade);
    dilatacaoMDH(matrizLimpa, matrizContagem, altura, largura, tamanhoL);
    algoritmoRetangulos(matrizContagem, altura, largura, ContagemLinhas);

    //limpando a matrizContagem para poder reutilizar ela na contagem de colunas
    for (unsigned int i = 0; i < altura; i++) {
        for (unsigned int j = 0; j < largura; j++) {
            matrizContagem[i][j] = 0;
        }
    }

    //vamos utilizar o tamanho da mascara no calculo das colunas como 20% do valor da largura da imagem 
    int tamanhoC = largura * 0.20;       
    Geral *ContagemColunas = malloc(sizeof(Geral));
    ContagemColunas->capacidade = 64;
    ContagemColunas->quant = 0;
    ContagemColunas->palavras = malloc(sizeof(Retangulo) * ContagemColunas->capacidade);
    dilatacaoMDV(matrizLimpa, matrizContagem, altura, largura, tamanhoC);
    algoritmoRetangulos(matrizContagem, altura, largura, ContagemColunas);

    destacandoPalavras(matrizLimpa, listaPalavras);
    imprimirImagem(matrizLimpa, arqsaida, altura, largura);

    free(listaPalavras->palavras);
    free(ContagemColunas->palavras);
    free(ContagemLinhas->palavras);
    free(listaPalavras);
    free(ContagemColunas);
    free(ContagemLinhas);
    fclose(arqfonte);
    fclose(arqsaida);
    return 0;
}