#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mantém o raster em um vetor contíguo, organizado linha a linha.
typedef struct {
    size_t largura;
    size_t altura;
    unsigned char *pixels;
} Imagem;

typedef struct {
    size_t inicio;
    size_t fim;
} Faixa;

typedef struct {
    Faixa *itens;
    size_t quantidade;
} ListaFaixas;

typedef struct {
    size_t topo;
    size_t base;
    size_t esquerda;
    size_t direita;
} Retangulo;

typedef struct {
    Retangulo *itens;
    size_t quantidade;
    size_t capacidade;
} ListaRetangulos;

typedef struct {
    size_t colunas;
    size_t linhas;
    ListaRetangulos palavras;
} Resultado;

typedef struct {
    const char *entrada;
    const char *saida;
    size_t espessura_negrito;
} Opcoes;

static size_t indice_pixel(const Imagem *imagem, size_t linha, size_t coluna) {
    return linha * imagem->largura + coluna;
}

static unsigned char obter_pixel(const Imagem *imagem, size_t linha, size_t coluna) {
    return imagem->pixels[indice_pixel(imagem, linha, coluna)];
}

static void definir_pixel(Imagem *imagem, size_t linha, size_t coluna, unsigned char valor) {
    imagem->pixels[indice_pixel(imagem, linha, coluna)] = valor;
}

static void liberar_imagem(Imagem *imagem) {
    free(imagem->pixels);
    imagem->pixels = NULL;
    imagem->largura = 0;
    imagem->altura = 0;
}

static bool criar_imagem(Imagem *imagem, size_t largura, size_t altura) {
    // Verifica o produto antes da alocação para evitar overflow de size_t.
    if (largura == 0 || altura == 0 || altura > SIZE_MAX / largura) {
        return false;
    }

    size_t quantidade = largura * altura;
    imagem->pixels = calloc(quantidade, sizeof(*imagem->pixels));
    if (imagem->pixels == NULL) {
        return false;
    }

    imagem->largura = largura;
    imagem->altura = altura;
    return true;
}

static void pular_comentario(FILE *arquivo) {
    int caractere;
    do {
        caractere = fgetc(arquivo);
    } while (caractere != EOF && caractere != '\n' && caractere != '\r');
}

// Lê um token do cabeçalho P1, ignorando espaços e comentários do arquivo.
static int ler_token_pbm(FILE *arquivo, char *token, size_t capacidade) {
    int caractere;

    if (capacidade < 2) {
        return -1;
    }

    for (;;) {
        caractere = fgetc(arquivo);
        if (caractere == EOF) {
            return 0;
        }
        if (isspace((unsigned char)caractere)) {
            continue;
        }
        if (caractere == '#') {
            pular_comentario(arquivo);
            continue;
        }
        break;
    }

    size_t tamanho = 0;
    while (caractere != EOF && !isspace((unsigned char)caractere) && caractere != '#') {
        if (tamanho + 1 >= capacidade) {
            return -1;
        }
        token[tamanho++] = (char)caractere;
        caractere = fgetc(arquivo);
    }

    if (caractere == '#') {
        pular_comentario(arquivo);
    }
    token[tamanho] = '\0';
    return 1;
}

static bool converter_dimensao(const char *texto, size_t *valor) {
    if (*texto == '\0') {
        return false;
    }
    for (const unsigned char *caractere = (const unsigned char *)texto;
         *caractere != '\0'; caractere++) {
        if (!isdigit(*caractere)) {
            return false;
        }
    }

    char *fim = NULL;
    errno = 0;
    uintmax_t convertido = strtoumax(texto, &fim, 10);

    if (errno != 0 || fim == texto || *fim != '\0' || convertido == 0 || convertido > SIZE_MAX) {
        return false;
    }

    *valor = (size_t)convertido;
    return true;
}

static int ler_bit_pbm(FILE *arquivo) {
    for (;;) {
        int caractere = fgetc(arquivo);
        if (caractere == EOF) {
            return -1;
        }
        if (isspace((unsigned char)caractere)) {
            continue;
        }
        if (caractere == '#') {
            pular_comentario(arquivo);
            continue;
        }
        if (caractere == '0' || caractere == '1') {
            return caractere - '0';
        }
        return -2;
    }
}

static bool ler_imagem_pbm(const char *caminho, Imagem *imagem) {
    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s': %s.\n", caminho, strerror(errno));
        return false;
    }

    bool sucesso = false;
    char token[64];
    size_t largura = 0;
    size_t altura = 0;

    // Valida o identificador e as dimensões antes de reservar o raster.
    int estado = ler_token_pbm(arquivo, token, sizeof(token));
    if (estado != 1 || strcmp(token, "P1") != 0) {
        fprintf(stderr, "Erro: '%s' nao e uma imagem PBM ASCII P1 valida.\n", caminho);
        goto finalizar;
    }

    estado = ler_token_pbm(arquivo, token, sizeof(token));
    if (estado != 1 || !converter_dimensao(token, &largura)) {
        fprintf(stderr, "Erro: largura PBM ausente ou invalida em '%s'.\n", caminho);
        goto finalizar;
    }

    estado = ler_token_pbm(arquivo, token, sizeof(token));
    if (estado != 1 || !converter_dimensao(token, &altura)) {
        fprintf(stderr, "Erro: altura PBM ausente ou invalida em '%s'.\n", caminho);
        goto finalizar;
    }

    if (!criar_imagem(imagem, largura, altura)) {
        fprintf(stderr, "Erro: dimensoes excessivas ou memoria insuficiente para '%s'.\n", caminho);
        goto finalizar;
    }

    size_t quantidade = largura * altura;
    // No PBM P1, espaços, quebras de linha e comentários podem separar os bits.
    for (size_t indice = 0; indice < quantidade; indice++) {
        int bit = ler_bit_pbm(arquivo);
        if (bit < 0) {
            if (bit == -1) {
                fprintf(stderr, "Erro: dados de pixels incompletos em '%s'.\n", caminho);
            } else {
                fprintf(stderr, "Erro: caractere invalido nos pixels de '%s'.\n", caminho);
            }
            liberar_imagem(imagem);
            goto finalizar;
        }
        imagem->pixels[indice] = (unsigned char)bit;
    }

    sucesso = true;

finalizar:
    if (fclose(arquivo) != 0 && sucesso) {
        fprintf(stderr, "Erro ao fechar '%s': %s.\n", caminho, strerror(errno));
        liberar_imagem(imagem);
        sucesso = false;
    }
    return sucesso;
}

// Aplica um filtro majoritário em cruz para reduzir ruído impulsivo.
static bool remover_ruido_impulsivo(const Imagem *origem, Imagem *destino) {
    if (!criar_imagem(destino, origem->largura, origem->altura)) {
        return false;
    }

    // A saída separada impede que um pixel filtrado altere os cálculos seguintes.
    for (size_t linha = 0; linha < origem->altura; linha++) {
        for (size_t coluna = 0; coluna < origem->largura; coluna++) {
            unsigned int pretos = obter_pixel(origem, linha, coluna);

            if (linha > 0) {
                pretos += obter_pixel(origem, linha - 1, coluna);
            }
            if (linha + 1 < origem->altura) {
                pretos += obter_pixel(origem, linha + 1, coluna);
            }
            if (coluna > 0) {
                pretos += obter_pixel(origem, linha, coluna - 1);
            }
            if (coluna + 1 < origem->largura) {
                pretos += obter_pixel(origem, linha, coluna + 1);
            }

            definir_pixel(destino, linha, coluna, pretos >= 3 ? 1U : 0U);
        }
    }

    return true;
}

// Dilata a imagem com uma máscara em losango sem modificar a origem.
static bool aplicar_negrito(const Imagem *origem, Imagem *destino, size_t espessura) {
    if (espessura == 0 || !criar_imagem(destino, origem->largura, origem->altura)) {
        return false;
    }

    for (size_t linha = 0; linha < origem->altura; linha++) {
        for (size_t coluna = 0; coluna < origem->largura; coluna++) {
            if (obter_pixel(origem, linha, coluna) == 0) {
                continue;
            }

            size_t topo = linha > espessura ? linha - espessura : 0;
            size_t espaco_abaixo = origem->altura - 1 - linha;
            size_t base = espaco_abaixo < espessura
                              ? origem->altura - 1
                              : linha + espessura;

            // O alcance horizontal diminui com a distância vertical, formando o losango.
            for (size_t y = topo; y <= base; y++) {
                size_t distancia_vertical = y > linha ? y - linha : linha - y;
                size_t alcance_horizontal = espessura - distancia_vertical;
                size_t esquerda = coluna > alcance_horizontal
                                      ? coluna - alcance_horizontal
                                      : 0;
                size_t espaco_direita = origem->largura - 1 - coluna;
                size_t direita = espaco_direita < alcance_horizontal
                                     ? origem->largura - 1
                                     : coluna + alcance_horizontal;

                memset(&destino->pixels[indice_pixel(destino, y, esquerda)], 1,
                       direita - esquerda + 1);
            }
        }
    }

    return true;
}

static void liberar_faixas(ListaFaixas *lista) {
    free(lista->itens);
    lista->itens = NULL;
    lista->quantidade = 0;
}

// Junta trechos ativos separados por, no máximo, maximo_vazio posições.
static bool encontrar_faixas(const unsigned char *ativo, size_t tamanho,
                             size_t maximo_vazio, ListaFaixas *resultado) {
    resultado->itens = NULL;
    resultado->quantidade = 0;

    if (tamanho == 0) {
        return true;
    }
    if (tamanho > SIZE_MAX / sizeof(*resultado->itens)) {
        return false;
    }

    Faixa *faixas = malloc(tamanho * sizeof(*faixas));
    if (faixas == NULL) {
        return false;
    }

    size_t posicao = 0;
    while (posicao < tamanho) {
        while (posicao < tamanho && ativo[posicao] == 0) {
            posicao++;
        }
        if (posicao == tamanho) {
            break;
        }

        size_t inicio = posicao;
        size_t fim = posicao;
        posicao++;

        for (;;) {
            while (posicao < tamanho && ativo[posicao] != 0) {
                fim = posicao;
                posicao++;
            }

            size_t inicio_vazio = posicao;
            while (posicao < tamanho && ativo[posicao] == 0) {
                posicao++;
            }

            // Uma lacuna maior que a tolerância encerra a faixa atual.
            if (posicao == tamanho || posicao - inicio_vazio > maximo_vazio) {
                break;
            }

            fim = posicao;
            posicao++;
        }

        faixas[resultado->quantidade].inicio = inicio;
        faixas[resultado->quantidade].fim = fim;
        resultado->quantidade++;
    }

    if (resultado->quantidade == 0) {
        free(faixas);
        return true;
    }

    Faixa *ajustado = realloc(faixas, resultado->quantidade * sizeof(*faixas));
    resultado->itens = ajustado != NULL ? ajustado : faixas;
    return true;
}

static bool adicionar_retangulo(ListaRetangulos *lista, Retangulo retangulo) {
    if (lista->quantidade == lista->capacidade) {
        // O crescimento geométrico reduz a quantidade de realocações da lista.
        size_t nova_capacidade = lista->capacidade == 0 ? 64 : lista->capacidade * 2;
        if (nova_capacidade < lista->capacidade ||
            nova_capacidade > SIZE_MAX / sizeof(*lista->itens)) {
            return false;
        }

        Retangulo *novos = realloc(lista->itens, nova_capacidade * sizeof(*novos));
        if (novos == NULL) {
            return false;
        }
        lista->itens = novos;
        lista->capacidade = nova_capacidade;
    }

    lista->itens[lista->quantidade++] = retangulo;
    return true;
}

static void liberar_resultado(Resultado *resultado) {
    free(resultado->palavras.itens);
    resultado->palavras.itens = NULL;
    resultado->palavras.quantidade = 0;
    resultado->palavras.capacidade = 0;
    resultado->colunas = 0;
    resultado->linhas = 0;
}

static size_t somar_tinta(const Imagem *imagem, size_t topo, size_t base,
                          size_t esquerda, size_t direita) {
    size_t total = 0;
    for (size_t linha = topo; linha <= base; linha++) {
        for (size_t coluna = esquerda; coluna <= direita; coluna++) {
            total += obter_pixel(imagem, linha, coluna);
        }
    }
    return total;
}

static int comparar_tamanhos(const void *primeiro, const void *segundo) {
    size_t a = *(const size_t *)primeiro;
    size_t b = *(const size_t *)segundo;
    return (a > b) - (a < b);
}

static bool estimar_altura_texto(const Imagem *imagem, size_t *altura_estimada) {
    unsigned char *ativo = calloc(imagem->altura, sizeof(*ativo));
    if (ativo == NULL) {
        return false;
    }

    for (size_t linha = 0; linha < imagem->altura; linha++) {
        size_t tinta = 0;
        for (size_t coluna = 0; coluna < imagem->largura; coluna++) {
            tinta += obter_pixel(imagem, linha, coluna);
        }
        ativo[linha] = tinta >= 2 ? 1U : 0U;
    }

    ListaFaixas faixas = {0};
    bool sucesso = encontrar_faixas(ativo, imagem->altura, 2, &faixas);
    free(ativo);
    if (!sucesso) {
        return false;
    }

    if (faixas.quantidade == 0) {
        *altura_estimada = 12;
        return true;
    }

    size_t *alturas = malloc(faixas.quantidade * sizeof(*alturas));
    if (alturas == NULL) {
        liberar_faixas(&faixas);
        return false;
    }

    size_t quantidade = 0;
    for (size_t i = 0; i < faixas.quantidade; i++) {
        size_t altura = faixas.itens[i].fim - faixas.itens[i].inicio + 1;
        if (altura >= 3) {
            alturas[quantidade++] = altura;
        }
    }
    liberar_faixas(&faixas);

    if (quantidade == 0) {
        *altura_estimada = 12;
    } else {
        // A mediana reduz a influência de faixas excepcionalmente altas ou baixas.
        qsort(alturas, quantidade, sizeof(*alturas), comparar_tamanhos);
        *altura_estimada = alturas[quantidade / 2];
        if (*altura_estimada < 12) {
            *altura_estimada = 12;
        }
    }

    free(alturas);
    return true;
}

static bool localizar_colunas(const Imagem *imagem, ListaFaixas *colunas) {
    size_t altura_texto = 0;
    if (!estimar_altura_texto(imagem, &altura_texto)) {
        return false;
    }

    size_t *projecao = calloc(imagem->largura, sizeof(*projecao));
    unsigned char *ativo = calloc(imagem->largura, sizeof(*ativo));
    if (projecao == NULL || ativo == NULL) {
        free(projecao);
        free(ativo);
        return false;
    }

    // A projeção vertical acumula a tinta de cada coluna da página.
    for (size_t linha = 0; linha < imagem->altura; linha++) {
        for (size_t coluna = 0; coluna < imagem->largura; coluna++) {
            projecao[coluna] += obter_pixel(imagem, linha, coluna);
        }
    }

    size_t minimo_tinta = imagem->altura / 1000;
    if (minimo_tinta < 2) {
        minimo_tinta = 2;
    }
    for (size_t coluna = 0; coluna < imagem->largura; coluna++) {
        ativo[coluna] = projecao[coluna] >= minimo_tinta ? 1U : 0U;
    }

    size_t uniao_interna = imagem->largura / 300;
    if (uniao_interna < altura_texto) {
        uniao_interna = altura_texto;
    }

    // A tolerância une partes próximas da mesma coluna sem alterar a imagem.
    ListaFaixas candidatas = {0};
    bool sucesso = encontrar_faixas(ativo, imagem->largura, uniao_interna, &candidatas);
    free(projecao);
    free(ativo);
    if (!sucesso) {
        return false;
    }

    if (candidatas.quantidade == 0) {
        *colunas = candidatas;
        return true;
    }

    Faixa *validas = malloc(candidatas.quantidade * sizeof(*validas));
    if (validas == NULL) {
        liberar_faixas(&candidatas);
        return false;
    }

    size_t quantidade = 0;
    for (size_t i = 0; i < candidatas.quantidade; i++) {
        Faixa faixa = candidatas.itens[i];
        size_t tinta = somar_tinta(imagem, 0, imagem->altura - 1, faixa.inicio, faixa.fim);
        size_t largura = faixa.fim - faixa.inicio + 1;

        if (tinta >= 8 && largura >= 2) {
            validas[quantidade++] = faixa;
        }
    }
    liberar_faixas(&candidatas);

    if (quantidade == 0) {
        free(validas);
        colunas->itens = NULL;
        colunas->quantidade = 0;
        return true;
    }

    Faixa *ajustado = realloc(validas, quantidade * sizeof(*validas));
    colunas->itens = ajustado != NULL ? ajustado : validas;
    colunas->quantidade = quantidade;
    return true;
}

static bool localizar_linhas(const Imagem *imagem, Faixa coluna, ListaFaixas *linhas) {
    size_t *projecao = calloc(imagem->altura, sizeof(*projecao));
    unsigned char *ativo = calloc(imagem->altura, sizeof(*ativo));
    if (projecao == NULL || ativo == NULL) {
        free(projecao);
        free(ativo);
        return false;
    }

    // A projeção horizontal é calculada apenas dentro da coluna atual.
    for (size_t linha = 0; linha < imagem->altura; linha++) {
        for (size_t x = coluna.inicio; x <= coluna.fim; x++) {
            projecao[linha] += obter_pixel(imagem, linha, x);
        }
        ativo[linha] = projecao[linha] >= 2 ? 1U : 0U;
    }

    ListaFaixas candidatas = {0};
    bool sucesso = encontrar_faixas(ativo, imagem->altura, 2, &candidatas);
    free(ativo);
    if (!sucesso) {
        free(projecao);
        return false;
    }

    if (candidatas.quantidade == 0) {
        free(projecao);
        *linhas = candidatas;
        return true;
    }

    Faixa *validas = malloc(candidatas.quantidade * sizeof(*validas));
    if (validas == NULL) {
        free(projecao);
        liberar_faixas(&candidatas);
        return false;
    }

    size_t quantidade = 0;
    for (size_t i = 0; i < candidatas.quantidade; i++) {
        Faixa faixa = candidatas.itens[i];
        size_t altura = faixa.fim - faixa.inicio + 1;
        size_t tinta = 0;
        for (size_t y = faixa.inicio; y <= faixa.fim; y++) {
            tinta += projecao[y];
        }

        if (altura >= 3 && tinta >= 8) {
            validas[quantidade++] = faixa;
        }
    }

    free(projecao);
    liberar_faixas(&candidatas);

    if (quantidade == 0) {
        free(validas);
        linhas->itens = NULL;
        linhas->quantidade = 0;
        return true;
    }

    Faixa *ajustado = realloc(validas, quantidade * sizeof(*validas));
    linhas->itens = ajustado != NULL ? ajustado : validas;
    linhas->quantidade = quantidade;
    return true;
}

static bool caixa_da_palavra(const Imagem *imagem, Faixa linha, Faixa palavra,
                             Retangulo *retangulo) {
    size_t topo = imagem->altura;
    size_t base = 0;
    size_t esquerda = imagem->largura;
    size_t direita = 0;
    bool encontrou = false;

    // Ajusta a faixa aproximada aos limites dos pixels realmente pretos.
    for (size_t y = linha.inicio; y <= linha.fim; y++) {
        for (size_t x = palavra.inicio; x <= palavra.fim; x++) {
            if (obter_pixel(imagem, y, x) == 0) {
                continue;
            }
            encontrou = true;
            if (y < topo) {
                topo = y;
            }
            if (y > base) {
                base = y;
            }
            if (x < esquerda) {
                esquerda = x;
            }
            if (x > direita) {
                direita = x;
            }
        }
    }

    if (!encontrou) {
        return false;
    }

    retangulo->topo = topo;
    retangulo->base = base;
    retangulo->esquerda = esquerda;
    retangulo->direita = direita;
    return true;
}

static bool localizar_palavras_na_linha(const Imagem *imagem, Faixa coluna,
                                        Faixa linha, ListaRetangulos *palavras) {
    size_t largura_coluna = coluna.fim - coluna.inicio + 1;
    size_t altura_linha = linha.fim - linha.inicio + 1;
    size_t minimo_tinta_vertical = altura_linha < 18 ? 1 : 2;
    unsigned char *ativo = calloc(largura_coluna, sizeof(*ativo));
    if (ativo == NULL) {
        return false;
    }

    for (size_t deslocamento = 0; deslocamento < largura_coluna; deslocamento++) {
        size_t x = coluna.inicio + deslocamento;
        size_t tinta_vertical = 0;
        for (size_t y = linha.inicio; y <= linha.fim; y++) {
            tinta_vertical += obter_pixel(imagem, y, x);
        }
        // Em linhas altas, um único pixel residual não deve unir duas palavras.
        ativo[deslocamento] = tinta_vertical >= minimo_tinta_vertical ? 1U : 0U;
    }

    // A separação mínima cresce com a altura estimada da linha de texto.
    size_t lacuna_palavra = (altura_linha + 3) / 4;
    if (lacuna_palavra < 4) {
        lacuna_palavra = 4;
    }

    ListaFaixas grupos = {0};
    bool sucesso = encontrar_faixas(ativo, largura_coluna, lacuna_palavra - 1, &grupos);
    free(ativo);
    if (!sucesso) {
        return false;
    }

    for (size_t i = 0; i < grupos.quantidade; i++) {
        Faixa palavra = {
            .inicio = coluna.inicio + grupos.itens[i].inicio,
            .fim = coluna.inicio + grupos.itens[i].fim,
        };
        Retangulo retangulo;
        if (caixa_da_palavra(imagem, linha, palavra, &retangulo) &&
            somar_tinta(imagem, linha.inicio, linha.fim, palavra.inicio, palavra.fim) >= 3 &&
            !adicionar_retangulo(palavras, retangulo)) {
            liberar_faixas(&grupos);
            return false;
        }
    }

    liberar_faixas(&grupos);
    return true;
}

static bool analisar_imagem(const Imagem *imagem, Resultado *resultado) {
    memset(resultado, 0, sizeof(*resultado));

    // Segmenta hierarquicamente a página em colunas, linhas e palavras.
    ListaFaixas colunas = {0};
    if (!localizar_colunas(imagem, &colunas)) {
        return false;
    }

    resultado->colunas = colunas.quantidade;
    for (size_t indice_coluna = 0; indice_coluna < colunas.quantidade; indice_coluna++) {
        ListaFaixas linhas = {0};
        if (!localizar_linhas(imagem, colunas.itens[indice_coluna], &linhas)) {
            liberar_faixas(&colunas);
            liberar_resultado(resultado);
            return false;
        }

        resultado->linhas += linhas.quantidade;
        for (size_t indice_linha = 0; indice_linha < linhas.quantidade; indice_linha++) {
            if (!localizar_palavras_na_linha(imagem, colunas.itens[indice_coluna],
                                             linhas.itens[indice_linha],
                                             &resultado->palavras)) {
                liberar_faixas(&linhas);
                liberar_faixas(&colunas);
                liberar_resultado(resultado);
                return false;
            }
        }
        liberar_faixas(&linhas);
    }

    liberar_faixas(&colunas);
    return true;
}

static void desenhar_retangulos(Imagem *imagem, const ListaRetangulos *retangulos,
                                size_t margem) {
    for (size_t indice = 0; indice < retangulos->quantidade; indice++) {
        Retangulo original = retangulos->itens[indice];
        // Expande a caixa solicitada e recorta suas coordenadas nas bordas.
        Retangulo retangulo = {
            .topo = original.topo > margem ? original.topo - margem : 0,
            .base = imagem->altura - 1 - original.base < margem
                        ? imagem->altura - 1
                        : original.base + margem,
            .esquerda = original.esquerda > margem ? original.esquerda - margem : 0,
            .direita = imagem->largura - 1 - original.direita < margem
                           ? imagem->largura - 1
                           : original.direita + margem,
        };

        for (size_t x = retangulo.esquerda; x <= retangulo.direita; x++) {
            definir_pixel(imagem, retangulo.topo, x, 1);
            definir_pixel(imagem, retangulo.base, x, 1);
        }
        for (size_t y = retangulo.topo; y <= retangulo.base; y++) {
            definir_pixel(imagem, y, retangulo.esquerda, 1);
            definir_pixel(imagem, y, retangulo.direita, 1);
        }
    }
}

static bool escrever_imagem_pbm(const char *caminho, const Imagem *imagem) {
    FILE *arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s': %s.\n", caminho, strerror(errno));
        return false;
    }

    bool sucesso = fprintf(arquivo, "P1\n%zu %zu\n", imagem->largura, imagem->altura) >= 0;
    size_t caracteres_na_linha = 0;

    // Limita as linhas textuais sem alterar a ordem dos pixels do raster.
    for (size_t linha = 0; sucesso && linha < imagem->altura; linha++) {
        for (size_t coluna = 0; coluna < imagem->largura; coluna++) {
            if (caracteres_na_linha == 70) {
                sucesso = fputc('\n', arquivo) != EOF;
                caracteres_na_linha = 0;
                if (!sucesso) {
                    break;
                }
            }
            sucesso = fputc(obter_pixel(imagem, linha, coluna) != 0 ? '1' : '0', arquivo) != EOF;
            caracteres_na_linha++;
            if (!sucesso) {
                break;
            }
        }
    }

    if (sucesso && caracteres_na_linha != 0) {
        sucesso = fputc('\n', arquivo) != EOF;
    }
    if (fclose(arquivo) != 0) {
        sucesso = false;
    }

    if (!sucesso) {
        fprintf(stderr, "Erro ao escrever a imagem PBM em '%s'.\n", caminho);
    }
    return sucesso;
}

static void imprimir_uso(const char *programa) {
    fprintf(stderr,
            "Uso: %s [--negrito[=1..3]] <entrada.pbm> [saida.pbm]\n",
            programa);
}

static bool ler_opcoes(int argc, char *argv[], Opcoes *opcoes) {
    memset(opcoes, 0, sizeof(*opcoes));
    opcoes->saida = "saida.pbm";

    bool fim_das_opcoes = false;
    bool negrito_definido = false;
    size_t quantidade_caminhos = 0;

    for (int indice = 1; indice < argc; indice++) {
        const char *argumento = argv[indice];

        // Após "--", argumentos iniciados por hífen também são tratados como caminhos.
        if (!fim_das_opcoes && strcmp(argumento, "--") == 0) {
            fim_das_opcoes = true;
            continue;
        }

        bool opcao_negrito = !fim_das_opcoes &&
                              (strcmp(argumento, "--negrito") == 0 ||
                               strcmp(argumento, "-b") == 0);
        bool opcao_negrito_com_valor = !fim_das_opcoes &&
                                        strncmp(argumento, "--negrito=", 10) == 0;

        if (opcao_negrito || opcao_negrito_com_valor) {
            if (negrito_definido) {
                fprintf(stderr, "Erro: a opcao de negrito foi informada mais de uma vez.\n");
                return false;
            }

            size_t espessura = 1;
            if (opcao_negrito_com_valor &&
                !converter_dimensao(argumento + 10, &espessura)) {
                fprintf(stderr, "Erro: espessura de negrito invalida.\n");
                return false;
            }
            if (espessura > 3) {
                fprintf(stderr, "Erro: a espessura do negrito deve estar entre 1 e 3.\n");
                return false;
            }

            opcoes->espessura_negrito = espessura;
            negrito_definido = true;
            continue;
        }

        if (!fim_das_opcoes && argumento[0] == '-' && argumento[1] != '\0') {
            fprintf(stderr, "Erro: opcao desconhecida '%s'.\n", argumento);
            return false;
        }

        if (quantidade_caminhos == 0) {
            opcoes->entrada = argumento;
        } else if (quantidade_caminhos == 1) {
            opcoes->saida = argumento;
        } else {
            fprintf(stderr, "Erro: foram informados caminhos em excesso.\n");
            return false;
        }
        quantidade_caminhos++;
    }

    if (opcoes->entrada == NULL) {
        fprintf(stderr, "Erro: informe uma imagem PBM de entrada.\n");
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    Opcoes opcoes;
    if (!ler_opcoes(argc, argv, &opcoes)) {
        imprimir_uso(argv[0]);
        return EXIT_FAILURE;
    }

    Imagem original = {0};
    Imagem limpa = {0};
    Imagem negrito = {0};
    Resultado resultado = {0};
    int codigo = EXIT_FAILURE;

    if (!ler_imagem_pbm(opcoes.entrada, &original)) {
        goto finalizar;
    }
    if (!remover_ruido_impulsivo(&original, &limpa)) {
        fprintf(stderr, "Erro: memoria insuficiente durante a remocao de ruido.\n");
        goto finalizar;
    }
    liberar_imagem(&original);

    // A segmentação usa a imagem limpa e ocorre antes de qualquer dilatação.
    if (!analisar_imagem(&limpa, &resultado)) {
        fprintf(stderr, "Erro: memoria insuficiente durante a segmentacao.\n");
        goto finalizar;
    }

    printf("Total de palavras encontradas: %zu\n", resultado.palavras.quantidade);
    printf("Total de linhas encontradas: %zu\n", resultado.linhas);
    printf("Total de colunas encontradas: %zu\n", resultado.colunas);

    Imagem *imagem_saida = &limpa;
    if (opcoes.espessura_negrito > 0) {
        // O negrito afeta somente a representação visual de saída.
        if (!aplicar_negrito(&limpa, &negrito, opcoes.espessura_negrito)) {
            fprintf(stderr, "Erro: memoria insuficiente ao aplicar o negrito.\n");
            goto finalizar;
        }
        imagem_saida = &negrito;
    }

    desenhar_retangulos(imagem_saida, &resultado.palavras,
                        opcoes.espessura_negrito + 1);
    if (!escrever_imagem_pbm(opcoes.saida, imagem_saida)) {
        goto finalizar;
    }

    codigo = EXIT_SUCCESS;

finalizar:
    liberar_resultado(&resultado);
    liberar_imagem(&negrito);
    liberar_imagem(&limpa);
    liberar_imagem(&original);
    return codigo;
}
