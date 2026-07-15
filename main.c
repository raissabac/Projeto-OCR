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

            if(matriz[i-1][j] == 0 || matriz[i+1][j] == 0 || matriz[i][j-1] == 0 || matriz[i][j] == 0 || matriz[i][j+1] == 0){
                tudo = false;
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
void dilatacao(unsigned char **matriz2, unsigned char **matrizLimpa, unsigned int altura, unsigned int largura, int m) {
    int val = m/2;

    for (unsigned int i = 1; i < altura-1; i++) {
        for(unsigned int j = 1; j < largura-1; j++){

            bool tudo = false;

            for(int x = val; x >= 0; x--) {
                if(x == 0 && matriz2[i][j] == 1) {
                    tudo = true;
                }
                else if(matriz2[i-x][j] == 1 || matriz2[i+x][j] == 1 || matriz2[i][j-x] == 1 || matriz2[i][j+x] == 1){
                    tudo = true;
                }
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
        for(unsigned int j = val; j < largura - val; j++) {
            bool um = false;

            for(int x = -val; x <= val; x++){
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

    // Corrigido: loop externo percorre colunas (largura) e interno percorre linhas (altura)
    for(unsigned int j = 0; j < largura; j++) {
        for(unsigned int i = val; i < altura - val; i++) {
            bool um = false;

            for(int x = -val; x <= val; x++){
                if(matrizLimpa[i+x][j] == 1){
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

//faz a extração de componentes conexos 
void algoritmoRetangulos(unsigned char **matrizMapeada, unsigned int altura, unsigned int largura, Geral *listaPalavras) {
    unsigned int capacidadeFila = altura * largura;
    unsigned int *filaX = malloc(capacidadeFila * sizeof(unsigned int)); 
    unsigned int *filaY = malloc(capacidadeFila * sizeof(unsigned int));
    if (!filaX || !filaY) {
        fprintf(stderr, "Erro: falha ao alocar fila BFS.\n");
        free(filaX); free(filaY);
        return;
    }

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
                    if (atualX > 0 && matrizMapeada[atualX - 1][atualY] == 1 && (unsigned int)fim < capacidadeFila) {
                        filaX[fim] = atualX - 1;
                        filaY[fim] = atualY;
                        fim++;
                        matrizMapeada[atualX - 1][atualY] = 0; 
                    }
                    // Olha vizinho de BAIXO
                    if (atualX + 1 < altura && matrizMapeada[atualX + 1][atualY] == 1 && (unsigned int)fim < capacidadeFila) {
                        filaX[fim] = atualX + 1;
                        filaY[fim] = atualY;
                        fim++;
                        matrizMapeada[atualX + 1][atualY] = 0; 
                    }
                    // Olha vizinho da ESQUERDA
                    if (atualY > 0 && matrizMapeada[atualX][atualY - 1] == 1 && (unsigned int)fim < capacidadeFila) {
                        filaX[fim] = atualX;
                        filaY[fim] = atualY - 1;
                        fim++;
                        matrizMapeada[atualX][atualY - 1] = 0; 
                    }
                    // Olha vizinho da DIREITA
                    if (atualY + 1 < largura && matrizMapeada[atualX][atualY + 1] == 1 && (unsigned int)fim < capacidadeFila) {
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
    fprintf(arqsaida, "%u %u\n", largura, altura);

    // Formato PBM P1 padrão: pixels sem espaço, quebra de linha a cada 70 caracteres
    unsigned int cont = 0;
    for(unsigned int i = 0; i < altura; i++) {
        for(unsigned int j = 0; j < largura; j++) {
            fputc(matrizLimpa[i][j] ? '1' : '0', arqsaida);
            cont++;
            if(cont >= 70) {
                fputc('\n', arqsaida);
                cont = 0;
            }
        }
    }
    // Garante quebra de linha no final do arquivo
    if(cont > 0) {
        fputc('\n', arqsaida);
    }
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Uso: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }
    
    //leitura do arquivo fonte passado via terminal e do arquivo que vai colocar a saída
    FILE* arqfonte = fopen(argv[1], "r");
    if (!arqfonte) {
        fprintf(stderr, "Erro ao abrir arquivo de entrada.\n");
        return 1;
    }


    FILE* arqsaida = fopen("saida.pbm", "w");
    if (arqsaida == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    char linha[150];
    unsigned int altura = 0;
    unsigned int largura = 0;

    // Lê o cabeçalho PBM: primeira linha é o magic number ("P1")
    fgets(linha, sizeof(linha), arqfonte);
    // Pula linhas de comentário (começam com '#') até chegar nas dimensões
    do {
        fgets(linha, sizeof(linha), arqfonte);
    } while (linha[0] == '#');
    // A linha atual já contém as dimensões
    sscanf(linha, "%u %u", &largura, &altura);

    unsigned char **matriz = (unsigned char **)malloc(altura * sizeof(unsigned char *));            //matriz lida do arquivo
    unsigned char **matrizErosao = (unsigned char **)malloc(altura * sizeof(unsigned char *));       //matriz depois de aplicar erosao
    unsigned char **matrizLimpa = (unsigned char **)malloc(altura * sizeof(unsigned char *));   //matriz depois de aplicar dilatacao
    unsigned char **matrizLimpa2 = (unsigned char **)malloc(altura * sizeof(unsigned char *));   //matriz depois de aplicar dilatacao
    unsigned char **matrizMapeada = (unsigned char **)malloc(altura * sizeof(unsigned char *));     //matriz depois de aplicar dilatacao MD
    unsigned char **matrizContagem = (unsigned char **)malloc(altura * sizeof(unsigned char *));    //matriz depois de aplicar a dilatacao para contagem de colunas e linhas

    
    // calloc inicializa tudo com 0 — bordas não ficam com lixo de memória
    for (unsigned int i = 0; i < altura; i++) {
        matriz[i] = (unsigned char *)calloc(largura, sizeof(unsigned char));
        matrizErosao[i] = (unsigned char *)calloc(largura, sizeof(unsigned char));
        matrizLimpa[i] = (unsigned char *)calloc(largura, sizeof(unsigned char));
        matrizLimpa2[i] = (unsigned char *)calloc(largura, sizeof(unsigned char));
        matrizMapeada[i] = (unsigned char *)calloc(largura, sizeof(unsigned char));
        matrizContagem[i]= (unsigned char *)calloc(largura, sizeof(unsigned char));
    }

    //começa fazendo a leitura do arquivo e colocando todos os valores na matriz
    for(unsigned int i = 0; i < altura; i++) {
        for(unsigned int j = 0; j < largura; j++) {
            int c;
            // Pula espaços em branco (espaço, tabulação, quebras de linha)
            do {
                c = fgetc(arqfonte);
            } while (c != EOF && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));

            if (c == EOF) {
                fprintf(stderr, "Erro: fim de arquivo prematuro ao ler pixel na posicao (%u, %u).\n", i, j);
                // Preenche com 0 em caso de erro para evitar lixo
                matriz[i][j] = 0;
            } else if (c == '0') {
                matriz[i][j] = 0;
            } else if (c == '1') {
                matriz[i][j] = 1;
            } else {
                // Caso apareça algum outro caractere inesperado
                matriz[i][j] = 0;
            }

            // Inicializa as bordas da matrizErosao com os pixels originais
            if(i == 0 || i == altura - 1 || j == 0 || j == largura - 1){
                matrizErosao[i][j] = matriz[i][j];
            }
        }
    }

    //pré-processamento - remoção de ruídos (abertura morfológica: erosão + dilatação com mesma máscara)
    erosao(matriz, matrizErosao, altura, largura);
    dilatacao(matrizErosao, matrizLimpa, altura, largura, 3);

    // Copia as bordas da imagem original para matrizLimpa
    // (erosão e dilatação ignoram a borda 1px; preservamos o valor original para não perder informação)
    for (unsigned int j = 0; j < largura; j++) {
        matrizLimpa[0][j] = matriz[0][j];
        matrizLimpa[altura-1][j] = matriz[altura-1][j];
    }
    for (unsigned int i = 0; i < altura; i++) {
        matrizLimpa[i][0] = matriz[i][0];
        matrizLimpa[i][largura-1] = matriz[i][largura-1];
    }

    //copia da matriz limpa para mais alterações
    for(unsigned int i = 0; i < altura; i++) {
        //matrizLimpa2[i] = malloc(largura * sizeof(unsigned char));
        for(unsigned int j = 0; j < largura; j++) {
            // Copia o pixel exato da Limpa para a Cópia
            matrizLimpa2[i][j] = matrizLimpa[i][j];
        }
    }

    int tamMascara = (int)(largura * 0.005);
    //garante que o valor da mascara não seja menor que 5 - já que a altura minima é 12
    if(tamMascara < 5) {
        tamMascara = 5;
    }
    //garante que a mascara seja impar
    if(tamMascara % 2 == 0) {
        tamMascara++;
    }
    //mapeamento das palavras - transformamos as palavras em blocos sólidos
    dilatacaoMDH(matrizLimpa, matrizMapeada, altura, largura, tamMascara);

    //estrutura para armazenar as informações do número de palavras
    Geral *listaPalavras = malloc(sizeof(Geral));
    listaPalavras->capacidade = 64;
    listaPalavras->palavras = malloc(sizeof(Retangulo) * listaPalavras->capacidade);
    listaPalavras->quant = 0;

    //utiliza o algoritmo flood fill adaptado para poder percorrer a matriz e identificar o número de palavras
    algoritmoRetangulos(matrizMapeada, altura, largura, listaPalavras);

    // Máscara horizontal de linhas: usa LARGURA (não altura) e garante que seja ímpar
    int tamanhoL = (int)(largura * 0.20);
    if(tamanhoL < 5) tamanhoL = 5;
    if(tamanhoL % 2 == 0) tamanhoL++;
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

    // Máscara vertical de colunas: usa ALTURA e garante que seja ímpar
    int tamanhoC = (int)(altura * 0.20);
    if(tamanhoC < 5) tamanhoC = 5;
    if(tamanhoC % 2 == 0) tamanhoC++;
    Geral *ContagemColunas = malloc(sizeof(Geral));
    ContagemColunas->capacidade = 64;
    ContagemColunas->quant = 0;
    ContagemColunas->palavras = malloc(sizeof(Retangulo) * ContagemColunas->capacidade);
    dilatacaoMDV(matrizLimpa, matrizContagem, altura, largura, tamanhoC);
    algoritmoRetangulos(matrizContagem, altura, largura, ContagemColunas);

    printf("\n\nTotal de palavras encontradas: %d", listaPalavras->quant);
    printf("\nTotal de linhas encontradas: %d", ContagemLinhas->quant);
    printf("\nTotal de colunas encontradas: %d", ContagemColunas->quant);
    printf("\n\n");

    destacandoPalavras(matrizLimpa, listaPalavras);
    imprimirImagem(matrizLimpa, arqsaida, altura, largura);

    for(unsigned int i = 0; i < altura; i++) {
        free(matriz[i]);
        free(matrizErosao[i]);
        free(matrizLimpa[i]);
        free(matrizLimpa2[i]);
        free(matrizMapeada[i]);
        free(matrizContagem[i]);
    }
    free(matriz);
    free(matrizErosao);
    free(matrizLimpa);
    free(matrizLimpa2);
    free(matrizMapeada);
    free(matrizContagem);
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