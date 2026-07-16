#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    Retangulo caixa;
    size_t coluna;
    size_t linha;
} PalavraSegmentada;

typedef struct {
    PalavraSegmentada *itens;
    size_t quantidade;
    size_t capacidade;
} ListaPalavras;

typedef struct {
    size_t colunas;
    size_t linhas;
    ListaPalavras palavras;
} Resultado;

typedef struct {
    const char *entrada;
    const char *saida;
    const char *saida_texto;
    size_t espessura_negrito;
} Opcoes;

#define LARGURA_MODELO 20U
#define ALTURA_MODELO 28U

typedef struct {
    unsigned int caractere;
    unsigned int largura_original;
    unsigned int altura_original;
    uint32_t linhas[ALTURA_MODELO];
} ModeloGlifo;

/*
 * Modelos da fonte Arial regular, 12 pontos a 200 DPI. No ambiente Linux de
 * geracao, Arial foi resolvida para a substituta compativel Liberation Sans,
 * que tambem ficou incorporada nos PDFs fornecidos. Cada glifo foi submetido
 * ao mesmo filtro de ruido usado na imagem e normalizado para uma grade de
 * 20 x 28. As dimensoes originais ajudam a distinguir formas muito parecidas,
 * como maiusculas, minusculas e sinais de pontuacao.
 */
static const ModeloGlifo MODELOS_GLIFOS[] = {
    {0x41U, 21U, 23U, {0x00000U,0x00000U,0x00000U,0x00000U,0x00700U,0x00f00U,0x00f80U,0x01d80U,0x01dc0U,0x039c0U,0x038e0U,0x038e0U,0x070e0U,0x07070U,0x0e030U,0x0e030U,0x0fff0U,0x0fff8U,0x1c018U,0x1800cU,0x1800cU,0x3000eU,0x7000eU,0x70006U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x42U, 17U, 23U, {0x00000U,0x00000U,0x03ffeU,0x0fffeU,0x1fffeU,0x3c00eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x1c00eU,0x0f80eU,0x0fffeU,0x1fffeU,0x1fffeU,0x3c00eU,0x7800eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7800eU,0x3fffeU,0x1fffeU,0x07ffeU,0x00000U,0x00000U}},
    {0x43U, 20U, 23U, {0x00000U,0x00000U,0x00000U,0x03f80U,0x0ffe0U,0x1fff0U,0x3c078U,0x78018U,0x0001cU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0001eU,0x6001cU,0x78038U,0x3c078U,0x1fff0U,0x0ffe0U,0x03f80U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x44U, 19U, 23U, {0x00000U,0x00000U,0x00000U,0x007feU,0x03ffeU,0x0fffeU,0x1f00eU,0x3c00eU,0x3800eU,0x7800eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7800eU,0x3c00eU,0x1e00eU,0x1f00eU,0x0fffeU,0x03ffeU,0x00ffeU,0x00000U,0x00000U,0x00000U}},
    {0x45U, 18U, 23U, {0x00000U,0x00000U,0x3fffeU,0x3fffeU,0x3fffeU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x1fffeU,0x1fffeU,0x1fffeU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x3fffeU,0x7fffeU,0x7fffeU,0x00000U,0x00000U,0x00000U}},
    {0x46U, 16U, 23U, {0x00000U,0x7fffeU,0x7fffeU,0x7fffeU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x7fffeU,0x7fffeU,0x7fffeU,0x3fffeU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x00000U}},
    {0x47U, 21U, 23U, {0x00000U,0x00000U,0x00000U,0x00000U,0x03f80U,0x0ffe0U,0x1fff0U,0x3001cU,0x3000cU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x7f80eU,0x7f80eU,0x6000eU,0x6000eU,0x6000cU,0x6001cU,0x70038U,0x3fff0U,0x0ffe0U,0x03f80U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x48U, 18U, 23U, {0x00000U,0x00000U,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7fffeU,0x7fffeU,0x7fffeU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x00000U,0x00000U,0x00000U}},
    {0x49U, 3U, 23U, {0x00000U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00000U}},
    {0x4aU, 13U, 23U, {0x00000U,0x1fe00U,0x1fe00U,0x1fe00U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c000U,0x1c01cU,0x1c01cU,0x0f07cU,0x0f07cU,0x0fff8U,0x07ff0U,0x00f80U,0x00000U}},
    {0x4bU, 18U, 23U, {0x00000U,0x00000U,0x1c00eU,0x1e00eU,0x0f00eU,0x0700eU,0x0380eU,0x01c0eU,0x00e0eU,0x0070eU,0x0038eU,0x001ceU,0x001feU,0x003feU,0x007feU,0x00f1eU,0x01e0eU,0x01c0eU,0x0380eU,0x0780eU,0x0f00eU,0x1e00eU,0x3c00eU,0x7800eU,0x7000eU,0x00000U,0x00000U,0x00000U}},
    {0x4cU, 14U, 23U, {0x00000U,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x3fffcU,0x3fffcU,0x3fffcU,0x00000U}},
    {0x4dU, 22U, 23U, {0x00000U,0x00000U,0x00000U,0x00000U,0x7000eU,0x7800eU,0x7801eU,0x7c03eU,0x7c03eU,0x7c036U,0x6e076U,0x67066U,0x670e6U,0x630e6U,0x630c6U,0x630c6U,0x61986U,0x60986U,0x60906U,0x60f06U,0x60f06U,0x60f06U,0x60606U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x4eU, 18U, 23U, {0x00000U,0x00000U,0x6001eU,0x6001eU,0x6003eU,0x6007eU,0x6007eU,0x600e6U,0x600e6U,0x601c6U,0x6038eU,0x6038eU,0x6070eU,0x60f0eU,0x60e0eU,0x61c0eU,0x61c0eU,0x6380eU,0x6700eU,0x6700eU,0x7e00eU,0x7e00eU,0x7c00eU,0x7800eU,0x7800eU,0x00000U,0x00000U,0x00000U}},
    {0x4fU, 22U, 23U, {0x00000U,0x00000U,0x00000U,0x00000U,0x01f80U,0x07fe0U,0x1e078U,0x1801cU,0x3800cU,0x3000eU,0x60006U,0x60006U,0x60006U,0x60006U,0x60006U,0x60006U,0x60006U,0x3000eU,0x3801cU,0x1801cU,0x1e078U,0x07fe0U,0x01f80U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x50U, 17U, 23U, {0x00000U,0x00000U,0x07ffeU,0x1fffeU,0x3fffeU,0x7800eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7800eU,0x3f00eU,0x3f00eU,0x1fffeU,0x07ffeU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x00000U,0x00000U}},
    {0x51U, 22U, 29U, {0x00000U,0x00000U,0x01f80U,0x07fe0U,0x1e078U,0x1801cU,0x3800cU,0x3000eU,0x70006U,0x60006U,0x60006U,0x60006U,0x60006U,0x60006U,0x60006U,0x3000eU,0x3000cU,0x1801cU,0x1e078U,0x07ff0U,0x01f80U,0x00e00U,0x00e00U,0x01c00U,0x1f800U,0x1f000U,0x00000U,0x00000U}},
    {0x52U, 19U, 23U, {0x00000U,0x00000U,0x00000U,0x03ffeU,0x0fffeU,0x1fffeU,0x3c00eU,0x3800eU,0x7800eU,0x7800eU,0x7800eU,0x3800eU,0x3c00eU,0x1e00eU,0x07ffeU,0x03c0eU,0x0380eU,0x0780eU,0x0700eU,0x0f00eU,0x1e00eU,0x1c00eU,0x3c00eU,0x7800eU,0x7000eU,0x00000U,0x00000U,0x00000U}},
    {0x53U, 19U, 23U, {0x00000U,0x00000U,0x00000U,0x01fc0U,0x0fff0U,0x1f8f8U,0x1c03cU,0x1801cU,0x0001cU,0x0001cU,0x0001cU,0x0003cU,0x001f8U,0x01ff0U,0x1fc00U,0x3e000U,0x38000U,0x78000U,0x78000U,0x7800eU,0x3800eU,0x3c03eU,0x1f9fcU,0x0fff0U,0x01fc0U,0x00000U,0x00000U,0x00000U}},
    {0x54U, 19U, 23U, {0x00000U,0x00000U,0x00000U,0x7fffeU,0x7fffeU,0x3fffeU,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00000U,0x00000U,0x00000U}},
    {0x55U, 19U, 23U, {0x00000U,0x00000U,0x00000U,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x7800eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x3c01eU,0x1e03cU,0x0fff8U,0x07ff0U,0x01fc0U,0x00000U,0x00000U,0x00000U}},
    {0x56U, 20U, 23U, {0x00000U,0x00000U,0x00000U,0x6000eU,0x7000eU,0x7000eU,0x7801cU,0x3801cU,0x1c018U,0x1c018U,0x1c030U,0x0c030U,0x0c070U,0x06060U,0x06060U,0x060e0U,0x030e0U,0x030e0U,0x039c0U,0x03f80U,0x01f80U,0x01f80U,0x00f00U,0x00f00U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x57U, 30U, 23U, {0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x60606U,0x60606U,0x20f06U,0x20f04U,0x3090cU,0x3890cU,0x1898cU,0x19898U,0x19898U,0x1b0d8U,0x0f0d8U,0x0f0f0U,0x07070U,0x06060U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x58U, 18U, 23U, {0x00000U,0x00000U,0x7000cU,0x7801cU,0x3803cU,0x1c038U,0x1e070U,0x0e0f0U,0x070e0U,0x039c0U,0x03f80U,0x01f80U,0x00f00U,0x00f00U,0x01f80U,0x03f80U,0x079c0U,0x070e0U,0x0e0f0U,0x1e070U,0x1c038U,0x3803cU,0x7801cU,0x7000eU,0x6000eU,0x00000U,0x00000U,0x00000U}},
    {0x59U, 19U, 23U, {0x00000U,0x00000U,0x00000U,0x70006U,0x7800eU,0x3c01eU,0x1c01cU,0x1e038U,0x0e078U,0x07070U,0x078e0U,0x039e0U,0x01dc0U,0x01f80U,0x00700U,0x00700U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00600U,0x00000U,0x00000U,0x00000U}},
    {0x5aU, 18U, 23U, {0x00000U,0x00000U,0x7fffcU,0x7fffcU,0x3fff8U,0x3c000U,0x1c000U,0x0e000U,0x07000U,0x07800U,0x03c00U,0x01c00U,0x00e00U,0x00700U,0x00780U,0x00380U,0x001c0U,0x000e0U,0x000f0U,0x00070U,0x00038U,0x0003cU,0x7fffeU,0x7fffeU,0x7fffeU,0x00000U,0x00000U,0x00000U}},
    {0x61U, 17U, 18U, {0x00000U,0x00000U,0x00000U,0x00000U,0x01fc0U,0x07ff0U,0x0f078U,0x1e038U,0x1c018U,0x1c000U,0x1c000U,0x1ffe0U,0x1fff8U,0x1c03cU,0x1c03cU,0x1c01cU,0x1c01cU,0x1e01eU,0x1e01eU,0x1f01cU,0x3fe3cU,0x78ff8U,0x781f0U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x62U, 15U, 24U, {0x00000U,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x03f1cU,0x03f1cU,0x0fffcU,0x1f0fcU,0x1c07cU,0x3c03cU,0x3803cU,0x3801cU,0x3801cU,0x3801cU,0x3801cU,0x3801cU,0x3801cU,0x3803cU,0x3803cU,0x3c03cU,0x1c07cU,0x1e0fcU,0x0fffcU,0x03f1cU,0x00000U}},
    {0x63U, 14U, 18U, {0x00000U,0x00000U,0x07f80U,0x3ffe0U,0x3ffe0U,0x7e3f0U,0x78070U,0x7803cU,0x0003cU,0x0003cU,0x0003cU,0x0003cU,0x0001eU,0x0001eU,0x0001eU,0x0003cU,0x0003cU,0x7003cU,0x7003cU,0x7803cU,0x78070U,0x7e3f0U,0x3ffe0U,0x3ffe0U,0x07f80U,0x00000U,0x00000U,0x00000U}},
    {0x64U, 15U, 24U, {0x00000U,0x38000U,0x38000U,0x38000U,0x38000U,0x38000U,0x38000U,0x39fc0U,0x39fc0U,0x3ffe0U,0x3e0f0U,0x3c078U,0x38038U,0x38038U,0x38038U,0x3803cU,0x3801cU,0x3801cU,0x3803cU,0x38038U,0x38038U,0x38038U,0x38038U,0x3c078U,0x3f0f0U,0x3ffe0U,0x31fc0U,0x00000U}},
    {0x65U, 16U, 18U, {0x00000U,0x00000U,0x00000U,0x00000U,0x01f80U,0x0fff0U,0x1f0f8U,0x3c078U,0x3c01cU,0x3c01cU,0x7001cU,0x7001cU,0x7fffcU,0x7fffeU,0x0001eU,0x0001cU,0x0001cU,0x0001cU,0x3001cU,0x3001cU,0x3c078U,0x3f1f8U,0x1fff0U,0x03f80U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x66U, 9U, 24U, {0x00000U,0x07800U,0x07e00U,0x01f00U,0x00f00U,0x00f00U,0x00f00U,0x07fe0U,0x07fe0U,0x07fe0U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00000U}},
    {0x67U, 15U, 25U, {0x00000U,0x30fc0U,0x3ffe0U,0x3f0f0U,0x3c078U,0x3c038U,0x38038U,0x38038U,0x3801cU,0x3801cU,0x3801cU,0x3801cU,0x38038U,0x38038U,0x38038U,0x3c038U,0x3c078U,0x3f0f0U,0x3ffe0U,0x38fc0U,0x38000U,0x38000U,0x38030U,0x3c070U,0x1e0f0U,0x0ffe0U,0x03f80U,0x00000U}},
    {0x68U, 14U, 24U, {0x00000U,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x07f1cU,0x07f1cU,0x0ff9cU,0x1f0fcU,0x1c07cU,0x1c03cU,0x1c03cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x1c01cU,0x00000U}},
    {0x69U, 3U, 24U, {0x00000U,0x00700U,0x00700U,0x00700U,0x00000U,0x00000U,0x00000U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00000U}},
    {0x6aU, 6U, 31U, {0x00000U,0x00e00U,0x00e00U,0x00e00U,0x00000U,0x00000U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00780U,0x00380U,0x00000U}},
    {0x6bU, 14U, 24U, {0x00000U,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0e01cU,0x0e01cU,0x0e01cU,0x0701cU,0x0381cU,0x01e1cU,0x00f1cU,0x0079cU,0x007fcU,0x007fcU,0x007fcU,0x00f3cU,0x01e1cU,0x03e1cU,0x03e1cU,0x0381cU,0x0701cU,0x0f01cU,0x1e01cU,0x1c01cU,0x00000U}},
    {0x6cU, 3U, 24U, {0x00000U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00000U}},
    {0x6dU, 24U, 18U, {0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x1e1e6U,0x1f3f6U,0x30f1eU,0x3070eU,0x60706U,0x60706U,0x60706U,0x60706U,0x60706U,0x60706U,0x60706U,0x60706U,0x60706U,0x60706U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x6eU, 14U, 18U, {0x00000U,0x00000U,0x0fe1eU,0x3ff9eU,0x3ff9eU,0x7e1feU,0x7807eU,0x7803eU,0x7803eU,0x7803eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x00000U,0x00000U,0x00000U}},
    {0x6fU, 16U, 18U, {0x00000U,0x00000U,0x00000U,0x00000U,0x03f80U,0x0fff0U,0x1f0f8U,0x3c078U,0x3c01cU,0x3c01cU,0x7001cU,0x7001cU,0x7001eU,0x7000eU,0x7000eU,0x7001eU,0x7001cU,0x7001cU,0x3c01cU,0x3c01cU,0x3c078U,0x1f0f8U,0x0fff0U,0x01f80U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x70U, 15U, 25U, {0x00000U,0x03f1cU,0x0fffcU,0x1f0fcU,0x1c07cU,0x3c03cU,0x3803cU,0x3801cU,0x3801cU,0x3801cU,0x3801cU,0x3801cU,0x3801cU,0x3803cU,0x3803cU,0x3c03cU,0x1c07cU,0x1e0fcU,0x0fffcU,0x03f1cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x0001cU,0x00000U}},
    {0x71U, 15U, 25U, {0x00000U,0x31fc0U,0x3ffe0U,0x3f0f0U,0x3c078U,0x38038U,0x38038U,0x38038U,0x3803cU,0x3801cU,0x3801cU,0x3803cU,0x38038U,0x38038U,0x38038U,0x3c038U,0x3c078U,0x3f0f0U,0x3ffe0U,0x39fc0U,0x38000U,0x38000U,0x38000U,0x38000U,0x38000U,0x38000U,0x38000U,0x00000U}},
    {0x72U, 9U, 18U, {0x00000U,0x0fc78U,0x0fe78U,0x0fe78U,0x07ff8U,0x003f8U,0x003f8U,0x001f8U,0x001f8U,0x001f8U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00078U,0x00000U}},
    {0x73U, 14U, 18U, {0x00000U,0x00000U,0x07fc0U,0x3fff0U,0x3fff0U,0x3e07cU,0x3803cU,0x0003cU,0x0003cU,0x0003cU,0x0007cU,0x007f0U,0x07fe0U,0x3ff80U,0x3ff80U,0x7e000U,0x78000U,0x78000U,0x78000U,0x7801eU,0x7801eU,0x7c07cU,0x3fff0U,0x3fff0U,0x07fc0U,0x00000U,0x00000U,0x00000U}},
    {0x74U, 9U, 22U, {0x00000U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x07ff0U,0x07ff0U,0x007c0U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00300U,0x00700U,0x01f00U,0x01f00U,0x07f00U,0x07e00U,0x00000U}},
    {0x75U, 14U, 18U, {0x00000U,0x00000U,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7801eU,0x7c03eU,0x7f87cU,0x7fff0U,0x7fff0U,0x707e0U,0x00000U,0x00000U,0x00000U}},
    {0x76U, 15U, 18U, {0x00000U,0x00000U,0x00000U,0x78006U,0x7801eU,0x7801eU,0x7801eU,0x3c01eU,0x3c03cU,0x3c03cU,0x1e03cU,0x1e03cU,0x1e078U,0x06078U,0x07078U,0x070e0U,0x030e0U,0x030e0U,0x038c0U,0x039c0U,0x01fc0U,0x01f80U,0x01f80U,0x01f80U,0x00f80U,0x00000U,0x00000U,0x00000U}},
    {0x77U, 24U, 18U, {0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x60706U,0x60706U,0x3070cU,0x30f0cU,0x3090cU,0x1891cU,0x1999cU,0x19898U,0x19898U,0x1f8d8U,0x0f0f0U,0x0f0f0U,0x0f0f0U,0x06060U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x78U, 14U, 18U, {0x00000U,0x00000U,0x7801cU,0x7803cU,0x7803cU,0x3c03cU,0x3e070U,0x0e1e0U,0x079e0U,0x079e0U,0x03fc0U,0x03f80U,0x01f80U,0x01f80U,0x01f80U,0x03fc0U,0x07fc0U,0x0e1e0U,0x0e1e0U,0x0e070U,0x3c070U,0x7c03cU,0x7801eU,0x7801eU,0x7001eU,0x00000U,0x00000U,0x00000U}},
    {0x79U, 15U, 25U, {0x00000U,0x3800cU,0x3801cU,0x3801cU,0x1c03cU,0x1c038U,0x1c038U,0x0e070U,0x0e070U,0x06070U,0x070e0U,0x070e0U,0x031c0U,0x039c0U,0x039c0U,0x03fc0U,0x01f80U,0x01f80U,0x00f00U,0x00f00U,0x00f00U,0x00780U,0x00780U,0x001c0U,0x001e0U,0x000fcU,0x0003cU,0x00000U}},
    {0x7aU, 14U, 18U, {0x00000U,0x00000U,0x7fffcU,0x7fffcU,0x7fffcU,0x3e000U,0x3e000U,0x0e000U,0x07800U,0x07800U,0x07c00U,0x03e00U,0x01e00U,0x00780U,0x00780U,0x007c0U,0x003e0U,0x001e0U,0x001e0U,0x00070U,0x0007cU,0x0007cU,0x7fffeU,0x7fffeU,0x7fffeU,0x00000U,0x00000U,0x00000U}},
    {0x30U, 16U, 23U, {0x00000U,0x01f80U,0x0ffe0U,0x1f9f0U,0x3e078U,0x3e078U,0x3c01cU,0x3c01cU,0x7001cU,0x7001cU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7000eU,0x7001cU,0x7001cU,0x3c01cU,0x3c07cU,0x1e078U,0x1e078U,0x1f9f0U,0x0ffe0U,0x01f80U,0x00000U}},
    {0x31U, 14U, 23U, {0x00000U,0x00700U,0x007e0U,0x007f0U,0x007f8U,0x007f8U,0x0071cU,0x0070cU,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x1fffcU,0x3fffcU,0x3fffcU,0x00000U}},
    {0x32U, 15U, 23U, {0x00000U,0x01fc0U,0x07ff8U,0x0fffcU,0x1f03cU,0x1f03cU,0x1e00eU,0x1e00eU,0x1e000U,0x1e000U,0x1e000U,0x0f000U,0x0f000U,0x07c00U,0x07c00U,0x01e00U,0x00f00U,0x00780U,0x001c0U,0x000f0U,0x00078U,0x0003cU,0x0003eU,0x0003eU,0x1fffeU,0x3fffeU,0x3fffeU,0x00000U}},
    {0x33U, 15U, 23U, {0x00000U,0x01fc0U,0x07ff8U,0x0fffcU,0x1f03eU,0x1f03eU,0x1e00eU,0x1e00eU,0x1e000U,0x1e000U,0x1f000U,0x07f80U,0x07f80U,0x07f80U,0x07f80U,0x1f800U,0x1e000U,0x38000U,0x38000U,0x38000U,0x3800eU,0x3e00eU,0x1e03eU,0x1e03eU,0x1fcfcU,0x0fff8U,0x01fc0U,0x00000U}},
    {0x34U, 17U, 23U, {0x00000U,0x00000U,0x07000U,0x07800U,0x07800U,0x07e00U,0x07f00U,0x06700U,0x06780U,0x061c0U,0x060c0U,0x060e0U,0x06070U,0x06030U,0x06030U,0x06038U,0x0601cU,0x0600cU,0x0f00eU,0x7fffeU,0x7fffeU,0x06000U,0x06000U,0x06000U,0x06000U,0x06000U,0x00000U,0x00000U}},
    {0x35U, 15U, 23U, {0x00000U,0x1fffcU,0x1fffcU,0x0fffcU,0x0000cU,0x0000cU,0x0000eU,0x0000eU,0x0000eU,0x0000eU,0x01f8eU,0x07ffeU,0x1fcfeU,0x1f03eU,0x1f03eU,0x3e000U,0x38000U,0x38000U,0x38000U,0x38000U,0x38006U,0x3e00eU,0x1f03eU,0x1f03eU,0x0fcfcU,0x07ff8U,0x00fc0U,0x00000U}},
    {0x36U, 15U, 23U, {0x00000U,0x01f80U,0x0fff0U,0x0f9f8U,0x1e078U,0x1e078U,0x1803cU,0x0003cU,0x0000eU,0x0000eU,0x07f8eU,0x0fffeU,0x1f07eU,0x1e03eU,0x1e03eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x3e03cU,0x1e07cU,0x1e07cU,0x0f9f8U,0x07ff0U,0x01f80U,0x00000U}},
    {0x37U, 15U, 23U, {0x00000U,0x3fffeU,0x3fffeU,0x3fffeU,0x1e000U,0x1e000U,0x1e000U,0x0f000U,0x07800U,0x07800U,0x01c00U,0x01c00U,0x00e00U,0x00e00U,0x00e00U,0x00700U,0x00700U,0x00300U,0x00380U,0x00380U,0x00380U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x00000U}},
    {0x38U, 15U, 23U, {0x00000U,0x01fc0U,0x07ff8U,0x0f87cU,0x1e03eU,0x1e03eU,0x1e00eU,0x1e00eU,0x1e00eU,0x1e00eU,0x1e03cU,0x0f878U,0x07ff8U,0x07ff8U,0x07ff8U,0x1f07cU,0x1e00eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x1e03eU,0x1e03eU,0x1f87cU,0x0fff8U,0x01fc0U,0x00000U}},
    {0x39U, 15U, 23U, {0x00000U,0x00fc0U,0x07ff8U,0x0fcfcU,0x0f03eU,0x0f03eU,0x1e00eU,0x1e00eU,0x3800eU,0x3800eU,0x3800eU,0x3800eU,0x3e00eU,0x3f03eU,0x3f03eU,0x3fcfcU,0x3fff8U,0x387c0U,0x38000U,0x1e000U,0x1e000U,0x1f00eU,0x0f03eU,0x0f03eU,0x07cfcU,0x01ff8U,0x007c0U,0x00000U}},
    {0x2eU, 3U, 4U, {0x00000U,0x00000U,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x00000U,0x00000U}},
    {0x2cU, 3U, 8U, {0x00000U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07fe0U,0x07f00U,0x07f00U,0x07f00U,0x07f00U,0x07f00U,0x07f00U,0x07f00U,0x07f00U,0x07f00U,0x07f00U,0x00f00U,0x00f00U,0x00f00U,0x00000U}},
    {0x3bU, 3U, 22U, {0x00000U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00f00U,0x00f00U,0x00f00U,0x00e00U,0x00e00U,0x00e00U,0x00e00U,0x00600U,0x00000U}},
    {0x3aU, 3U, 18U, {0x00000U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00f00U,0x00f00U,0x00f00U,0x00f00U,0x00000U}},
    {0x21U, 3U, 23U, {0x00000U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00700U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00700U,0x00700U,0x00700U,0x00000U}},
    {0x3fU, 15U, 23U, {0x00000U,0x01fc0U,0x0fff8U,0x1fffcU,0x3e03eU,0x3e03eU,0x3800eU,0x3800eU,0x38006U,0x38000U,0x38000U,0x1e000U,0x1f000U,0x0f800U,0x0f800U,0x01e00U,0x00f00U,0x00700U,0x00380U,0x00380U,0x00000U,0x00000U,0x00000U,0x00000U,0x00380U,0x00380U,0x00380U,0x00000U}},
    {0x22U, 9U, 7U, {0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x7e07eU,0x7e07eU,0x7e07eU,0x7e07eU,0x78078U,0x78078U,0x78078U,0x78078U,0x78078U,0x78078U,0x78078U,0x78078U,0x78078U,0x78078U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x27U, 3U, 7U, {0x00000U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x07ff0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x007f0U,0x00000U}},
    {0x28U, 8U, 31U, {0x00000U,0x01800U,0x01c00U,0x00e00U,0x00600U,0x00300U,0x00300U,0x00180U,0x00180U,0x00180U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x001c0U,0x00180U,0x00180U,0x00180U,0x00300U,0x00300U,0x00600U,0x00e00U,0x01c00U,0x01800U,0x00000U}},
    {0x29U, 8U, 31U, {0x00000U,0x000c0U,0x001c0U,0x00180U,0x00300U,0x00600U,0x00600U,0x00e00U,0x00e00U,0x01c00U,0x01c00U,0x01c00U,0x01c00U,0x01c00U,0x01c00U,0x01c00U,0x01c00U,0x01c00U,0x01c00U,0x00e00U,0x00e00U,0x00600U,0x00600U,0x00300U,0x00180U,0x001c0U,0x000c0U,0x00000U}},
    {0x2dU, 9U, 3U, {0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x1fff8U,0x1fff8U,0x7fffeU,0x7fffeU,0x7fffeU,0x7fffeU,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U,0x00000U}},
    {0x2fU, 9U, 24U, {0x00000U,0x06000U,0x07000U,0x07000U,0x03000U,0x03800U,0x03800U,0x01800U,0x01800U,0x01800U,0x01e00U,0x01e00U,0x00e00U,0x00f00U,0x00f00U,0x00f00U,0x00700U,0x00780U,0x00780U,0x00180U,0x00180U,0x00180U,0x001c0U,0x001c0U,0x000c0U,0x000e0U,0x000e0U,0x00000U}},
    /* Variantes causadas pelo alinhamento subpixel na rasterizacao do PDF. */
    {0x74U, 8U, 21U, {0x00000U,0x00380U,0x00380U,0x00780U,0x00780U,0x03fe0U,0x03fe0U,0x00fc0U,0x003c0U,0x003c0U,0x003c0U,0x00380U,0x007c0U,0x00380U,0x00380U,0x007c0U,0x00380U,0x007c0U,0x00380U,0x00380U,0x007c0U,0x00780U,0x00780U,0x00780U,0x00780U,0x07f80U,0x07e00U,0x00000U}},
    {0x74U, 8U, 21U, {0x00000U,0x00380U,0x00380U,0x00780U,0x00780U,0x03fe0U,0x07fe0U,0x00fe0U,0x007c0U,0x007c0U,0x00380U,0x007c0U,0x00380U,0x007c0U,0x007c0U,0x00380U,0x007c0U,0x00380U,0x007c0U,0x007c0U,0x00380U,0x007c0U,0x00780U,0x00780U,0x00780U,0x07f80U,0x07e00U,0x00000U}},
};

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

/*
 * Lê um token do cabeçalho P1. Comentários podem aparecer entre quaisquer
 * tokens, e largura e altura não precisam estar na mesma linha.
 */
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

/*
 * Filtro majoritario com vizinhanca em cruz. Ele remove impulsos pretos
 * isolados e preenche impulsos brancos dentro dos tracos sem exigir buffers
 * intermediarios para erosao e dilatacao.
 */
static bool remover_ruido_impulsivo(const Imagem *origem, Imagem *destino) {
    if (!criar_imagem(destino, origem->largura, origem->altura)) {
        return false;
    }

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

/*
 * Dilatação em forma de losango, equivalente a repetir uma máscara em cruz.
 * A origem permanece intacta para que a espessura seja controlada exatamente.
 */
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

/* Junta trechos ativos separados por, no maximo, maximo_vazio posições. */
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

static bool adicionar_palavra(ListaPalavras *lista, Retangulo caixa,
                              size_t coluna, size_t linha) {
    if (lista->quantidade == lista->capacidade) {
        size_t nova_capacidade = lista->capacidade == 0 ? 64 : lista->capacidade * 2;
        if (nova_capacidade < lista->capacidade ||
            nova_capacidade > SIZE_MAX / sizeof(*lista->itens)) {
            return false;
        }

        PalavraSegmentada *novos = realloc(lista->itens,
                                           nova_capacidade * sizeof(*novos));
        if (novos == NULL) {
            return false;
        }
        lista->itens = novos;
        lista->capacidade = nova_capacidade;
    }

    lista->itens[lista->quantidade++] = (PalavraSegmentada){
        .caixa = caixa,
        .coluna = coluna,
        .linha = linha,
    };
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
                                        Faixa linha, size_t indice_coluna,
                                        size_t indice_linha, ListaPalavras *palavras) {
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
        /* Um unico pixel residual nao deve criar uma ponte entre palavras. */
        ativo[deslocamento] = tinta_vertical >= minimo_tinta_vertical ? 1U : 0U;
    }

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
            !adicionar_palavra(palavras, retangulo, indice_coluna, indice_linha)) {
            liberar_faixas(&grupos);
            return false;
        }
    }

    liberar_faixas(&grupos);
    return true;
}

static bool analisar_imagem(const Imagem *imagem, Resultado *resultado) {
    memset(resultado, 0, sizeof(*resultado));

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
                                             indice_coluna, indice_linha,
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

typedef struct {
    char *dados;
    size_t tamanho;
    size_t capacidade;
} BufferTexto;

static void liberar_buffer_texto(BufferTexto *buffer) {
    free(buffer->dados);
    buffer->dados = NULL;
    buffer->tamanho = 0;
    buffer->capacidade = 0;
}

static bool reservar_buffer_texto(BufferTexto *buffer, size_t adicional) {
    if (adicional > SIZE_MAX - buffer->tamanho - 1) {
        return false;
    }

    size_t necessario = buffer->tamanho + adicional + 1;
    if (necessario <= buffer->capacidade) {
        return true;
    }

    size_t nova_capacidade = buffer->capacidade == 0 ? 256 : buffer->capacidade;
    while (nova_capacidade < necessario) {
        if (nova_capacidade > SIZE_MAX / 2) {
            nova_capacidade = necessario;
            break;
        }
        nova_capacidade *= 2;
    }

    char *novos_dados = realloc(buffer->dados, nova_capacidade);
    if (novos_dados == NULL) {
        return false;
    }
    buffer->dados = novos_dados;
    buffer->capacidade = nova_capacidade;
    return true;
}

static bool adicionar_caractere_texto(BufferTexto *buffer, char caractere) {
    if (!reservar_buffer_texto(buffer, 1)) {
        return false;
    }
    buffer->dados[buffer->tamanho++] = caractere;
    buffer->dados[buffer->tamanho] = '\0';
    return true;
}

static bool adicionar_texto(BufferTexto *buffer, const char *texto) {
    size_t tamanho = strlen(texto);
    if (!reservar_buffer_texto(buffer, tamanho)) {
        return false;
    }
    memcpy(buffer->dados + buffer->tamanho, texto, tamanho);
    buffer->tamanho += tamanho;
    buffer->dados[buffer->tamanho] = '\0';
    return true;
}

static size_t contar_bits(uint32_t valor) {
    size_t quantidade = 0;
    while (valor != 0U) {
        valor &= valor - 1U;
        quantidade++;
    }
    return quantidade;
}

static size_t mapear_centro(size_t destino, size_t tamanho_destino,
                            size_t tamanho_origem) {
    long double numerador = (long double)(2 * destino + 1) *
                            (long double)tamanho_origem;
    size_t origem = (size_t)(numerador / (long double)(2 * tamanho_destino));
    return origem < tamanho_origem ? origem : tamanho_origem - 1;
}

static void normalizar_glifo(const Imagem *imagem, Retangulo caixa,
                             uint32_t linhas[ALTURA_MODELO]) {
    memset(linhas, 0, ALTURA_MODELO * sizeof(*linhas));

    size_t largura = caixa.direita - caixa.esquerda + 1;
    size_t altura = caixa.base - caixa.topo + 1;
    size_t largura_disponivel = LARGURA_MODELO - 2U;
    size_t altura_disponivel = ALTURA_MODELO - 2U;
    size_t nova_largura;
    size_t nova_altura;

    long double escala_horizontal = (long double)largura_disponivel /
                                     (long double)largura;
    long double escala_vertical = (long double)altura_disponivel /
                                   (long double)altura;
    if (escala_horizontal <= escala_vertical) {
        nova_largura = largura_disponivel;
        nova_altura = (size_t)((long double)altura * escala_horizontal + 0.5L);
        if (nova_altura == 0) {
            nova_altura = 1;
        }
    } else {
        nova_altura = altura_disponivel;
        nova_largura = (size_t)((long double)largura * escala_vertical + 0.5L);
        if (nova_largura == 0) {
            nova_largura = 1;
        }
    }

    size_t margem_esquerda = (LARGURA_MODELO - nova_largura) / 2;
    size_t margem_superior = (ALTURA_MODELO - nova_altura) / 2;
    for (size_t y = 0; y < nova_altura; y++) {
        size_t origem_y = caixa.topo + mapear_centro(y, nova_altura, altura);
        for (size_t x = 0; x < nova_largura; x++) {
            size_t origem_x = caixa.esquerda + mapear_centro(x, nova_largura, largura);
            if (obter_pixel(imagem, origem_y, origem_x) != 0) {
                linhas[margem_superior + y] |= UINT32_C(1) << (margem_esquerda + x);
            }
        }
    }
}

static double erro_forma(const uint32_t glifo[ALTURA_MODELO],
                         const ModeloGlifo *modelo, int deslocamento_x,
                         int deslocamento_y) {
    size_t uniao = 0;
    size_t diferenca = 0;
    const uint32_t mascara = (UINT32_C(1) << LARGURA_MODELO) - UINT32_C(1);

    for (int y = 0; y < (int)ALTURA_MODELO; y++) {
        int origem_y = y - deslocamento_y;
        uint32_t linha_modelo = 0;
        if (origem_y >= 0 && origem_y < (int)ALTURA_MODELO) {
            linha_modelo = modelo->linhas[origem_y];
            if (deslocamento_x < 0) {
                linha_modelo >>= (unsigned int)(-deslocamento_x);
            } else if (deslocamento_x > 0) {
                linha_modelo <<= (unsigned int)deslocamento_x;
            }
            linha_modelo &= mascara;
        }

        uint32_t linha_glifo = glifo[y];
        uniao += contar_bits(linha_glifo | linha_modelo);
        diferenca += contar_bits(linha_glifo ^ linha_modelo);
    }

    return uniao == 0 ? 1.0 : (double)diferenca / (double)uniao;
}

static double classificar_glifo(const Imagem *imagem, Retangulo caixa,
                                char *caractere) {
    uint32_t glifo[ALTURA_MODELO];
    normalizar_glifo(imagem, caixa, glifo);

    size_t largura = caixa.direita - caixa.esquerda + 1;
    size_t altura = caixa.base - caixa.topo + 1;
    double melhor_erro = 10.0;
    unsigned int melhor_caractere = (unsigned int)'?';

    size_t quantidade_modelos = sizeof(MODELOS_GLIFOS) / sizeof(MODELOS_GLIFOS[0]);
    for (size_t indice = 0; indice < quantidade_modelos; indice++) {
        const ModeloGlifo *modelo = &MODELOS_GLIFOS[indice];
        double menor_erro_forma = 1.0;
        for (int deslocamento_y = -1; deslocamento_y <= 1; deslocamento_y++) {
            for (int deslocamento_x = -1; deslocamento_x <= 1; deslocamento_x++) {
                double atual = erro_forma(glifo, modelo, deslocamento_x,
                                          deslocamento_y);
                if (atual < menor_erro_forma) {
                    menor_erro_forma = atual;
                }
            }
        }

        size_t diferenca_largura = largura > modelo->largura_original
                                       ? largura - modelo->largura_original
                                       : modelo->largura_original - largura;
        size_t maior_largura = largura > modelo->largura_original
                                   ? largura
                                   : modelo->largura_original;
        size_t diferenca_altura = altura > modelo->altura_original
                                      ? altura - modelo->altura_original
                                      : modelo->altura_original - altura;
        size_t maior_altura = altura > modelo->altura_original
                                  ? altura
                                  : modelo->altura_original;
        double erro_dimensao = (double)diferenca_largura / (double)maior_largura +
                               (double)diferenca_altura / (double)maior_altura;
        double erro = menor_erro_forma + 0.50 * erro_dimensao;
        if (erro < melhor_erro) {
            melhor_erro = erro;
            melhor_caractere = modelo->caractere;
        }
    }

    *caractere = melhor_erro <= 1.05 ? (char)melhor_caractere : '?';
    return melhor_erro;
}

static bool caixa_de_tinta(const Imagem *imagem, Retangulo limite,
                           Retangulo *caixa) {
    bool encontrou = false;
    caixa->topo = limite.base;
    caixa->base = limite.topo;
    caixa->esquerda = limite.direita;
    caixa->direita = limite.esquerda;

    for (size_t y = limite.topo; y <= limite.base; y++) {
        for (size_t x = limite.esquerda; x <= limite.direita; x++) {
            if (obter_pixel(imagem, y, x) == 0) {
                continue;
            }
            if (!encontrou || y < caixa->topo) {
                caixa->topo = y;
            }
            if (!encontrou || y > caixa->base) {
                caixa->base = y;
            }
            if (!encontrou || x < caixa->esquerda) {
                caixa->esquerda = x;
            }
            if (!encontrou || x > caixa->direita) {
                caixa->direita = x;
            }
            encontrou = true;
        }
    }
    return encontrou;
}

typedef struct {
    Retangulo caixa;
    size_t area;
} ComponenteTinta;

static bool caixas_se_sobrepoem_verticalmente(Retangulo primeira,
                                               Retangulo segunda) {
    size_t topo = primeira.topo > segunda.topo ? primeira.topo : segunda.topo;
    size_t base = primeira.base < segunda.base ? primeira.base : segunda.base;
    return topo <= base;
}

static void expandir_retangulo(Retangulo *destino, Retangulo origem) {
    if (origem.topo < destino->topo) {
        destino->topo = origem.topo;
    }
    if (origem.base > destino->base) {
        destino->base = origem.base;
    }
    if (origem.esquerda < destino->esquerda) {
        destino->esquerda = origem.esquerda;
    }
    if (origem.direita > destino->direita) {
        destino->direita = origem.direita;
    }
}

/* Tenta separar glifos que se sobrepoem no eixo X, mas nao estao conectados. */
static bool dividir_por_componentes(const Imagem *imagem, Retangulo grupo,
                                    char *primeiro, char *segundo,
                                    double *erro_divisao) {
    size_t largura = grupo.direita - grupo.esquerda + 1;
    size_t altura = grupo.base - grupo.topo + 1;
    if (altura > SIZE_MAX / largura) {
        return false;
    }
    size_t quantidade_pixels = largura * altura;
    if (quantidade_pixels > SIZE_MAX / sizeof(size_t) ||
        quantidade_pixels > SIZE_MAX / sizeof(ComponenteTinta)) {
        return false;
    }

    unsigned char *visitado = calloc(quantidade_pixels, sizeof(*visitado));
    size_t *fila = malloc(quantidade_pixels * sizeof(*fila));
    ComponenteTinta *componentes = malloc(quantidade_pixels * sizeof(*componentes));
    if (visitado == NULL || fila == NULL || componentes == NULL) {
        free(visitado);
        free(fila);
        free(componentes);
        return false;
    }

    size_t quantidade_componentes = 0;
    size_t maior_area = 0;
    for (size_t y = 0; y < altura; y++) {
        for (size_t x = 0; x < largura; x++) {
            size_t inicio = y * largura + x;
            if (visitado[inicio] != 0 ||
                obter_pixel(imagem, grupo.topo + y, grupo.esquerda + x) == 0) {
                continue;
            }

            size_t cabeca = 0;
            size_t cauda = 0;
            fila[cauda++] = inicio;
            visitado[inicio] = 1;
            Retangulo caixa = {
                .topo = grupo.topo + y,
                .base = grupo.topo + y,
                .esquerda = grupo.esquerda + x,
                .direita = grupo.esquerda + x,
            };
            size_t area = 0;

            while (cabeca < cauda) {
                size_t atual = fila[cabeca++];
                size_t atual_y = atual / largura;
                size_t atual_x = atual % largura;
                area++;

                size_t inicio_y = atual_y == 0 ? 0 : atual_y - 1;
                size_t fim_y = atual_y + 1 < altura ? atual_y + 1 : altura - 1;
                size_t inicio_x = atual_x == 0 ? 0 : atual_x - 1;
                size_t fim_x = atual_x + 1 < largura ? atual_x + 1 : largura - 1;
                for (size_t vizinho_y = inicio_y; vizinho_y <= fim_y; vizinho_y++) {
                    for (size_t vizinho_x = inicio_x; vizinho_x <= fim_x; vizinho_x++) {
                        size_t vizinho = vizinho_y * largura + vizinho_x;
                        if (visitado[vizinho] != 0 ||
                            obter_pixel(imagem, grupo.topo + vizinho_y,
                                       grupo.esquerda + vizinho_x) == 0) {
                            continue;
                        }
                        visitado[vizinho] = 1;
                        fila[cauda++] = vizinho;

                        size_t global_y = grupo.topo + vizinho_y;
                        size_t global_x = grupo.esquerda + vizinho_x;
                        if (global_y < caixa.topo) {
                            caixa.topo = global_y;
                        }
                        if (global_y > caixa.base) {
                            caixa.base = global_y;
                        }
                        if (global_x < caixa.esquerda) {
                            caixa.esquerda = global_x;
                        }
                        if (global_x > caixa.direita) {
                            caixa.direita = global_x;
                        }
                    }
                }
            }

            componentes[quantidade_componentes++] = (ComponenteTinta){
                .caixa = caixa,
                .area = area,
            };
            if (area > maior_area) {
                maior_area = area;
            }
        }
    }
    free(visitado);
    free(fila);

    size_t principais[2] = {0, 0};
    size_t quantidade_principais = 0;
    size_t area_minima_principal = maior_area / 3 + (maior_area % 3 != 0 ? 1 : 0);
    for (size_t indice = 0; indice < quantidade_componentes; indice++) {
        if (componentes[indice].area < area_minima_principal) {
            continue;
        }
        if (quantidade_principais == 2) {
            quantidade_principais++;
            break;
        }
        principais[quantidade_principais++] = indice;
    }

    bool sucesso = false;
    if (quantidade_principais == 2) {
        Retangulo primeira_caixa = componentes[principais[0]].caixa;
        Retangulo segunda_caixa = componentes[principais[1]].caixa;
        size_t centro_primeiro = primeira_caixa.esquerda +
                                 (primeira_caixa.direita - primeira_caixa.esquerda) / 2;
        size_t centro_segundo = segunda_caixa.esquerda +
                                (segunda_caixa.direita - segunda_caixa.esquerda) / 2;
        if (centro_segundo < centro_primeiro) {
            Retangulo temporario = primeira_caixa;
            primeira_caixa = segunda_caixa;
            segunda_caixa = temporario;
            size_t indice_temporario = principais[0];
            principais[0] = principais[1];
            principais[1] = indice_temporario;
            size_t centro_temporario = centro_primeiro;
            centro_primeiro = centro_segundo;
            centro_segundo = centro_temporario;
        }

        if (centro_primeiro < centro_segundo &&
            caixas_se_sobrepoem_verticalmente(primeira_caixa, segunda_caixa)) {
            for (size_t indice = 0; indice < quantidade_componentes; indice++) {
                if (indice == principais[0] || indice == principais[1]) {
                    continue;
                }
                Retangulo satelite = componentes[indice].caixa;
                size_t centro_satelite = satelite.esquerda +
                                         (satelite.direita - satelite.esquerda) / 2;
                size_t distancia_primeiro = centro_satelite > centro_primeiro
                                                ? centro_satelite - centro_primeiro
                                                : centro_primeiro - centro_satelite;
                size_t distancia_segundo = centro_satelite > centro_segundo
                                               ? centro_satelite - centro_segundo
                                               : centro_segundo - centro_satelite;
                if (distancia_primeiro <= distancia_segundo) {
                    expandir_retangulo(&primeira_caixa, satelite);
                } else {
                    expandir_retangulo(&segunda_caixa, satelite);
                }
            }

            double erro_primeiro = classificar_glifo(imagem, primeira_caixa,
                                                      primeiro);
            double erro_segundo = classificar_glifo(imagem, segunda_caixa,
                                                     segundo);
            *erro_divisao = (erro_primeiro + erro_segundo) / 2.0 + 0.05;
            sucesso = true;
        }
    }

    free(componentes);
    return sucesso;
}

static char ajustar_pontuacao_vertical(char caractere, Retangulo glifo,
                                       Retangulo palavra) {
    if (caractere != '\'' && caractere != ',') {
        return caractere;
    }

    size_t centro_glifo = glifo.topo - palavra.topo +
                          (glifo.base - glifo.topo) / 2;
    size_t centro_palavra = (palavra.base - palavra.topo) / 2;
    return centro_glifo > centro_palavra ? ',' : '\'';
}

/*
 * Alguns pares com kerning (por exemplo, "ff" e "ax") nao possuem uma
 * coluna totalmente branca entre os glifos. Nesses casos, compara-se o grupo
 * inteiro com todas as divisoes plausiveis e a divisao so vence quando reduz
 * claramente o erro, incluindo uma penalidade contra separacoes artificiais.
 */
static bool reconhecer_grupo(const Imagem *imagem, Retangulo grupo,
                             Retangulo palavra, char caractere_inteiro,
                             double erro_inteiro, BufferTexto *texto) {
    double melhor_erro_divisao = 10.0;
    char primeiro_melhor = '?';
    char segundo_melhor = '?';
    size_t largura = grupo.direita - grupo.esquerda + 1;
    size_t altura = grupo.base - grupo.topo + 1;

    if (largura >= 7 && largura <= 64 && altura <= 64 && erro_inteiro >= 0.50) {
        char primeiro_componente;
        char segundo_componente;
        double erro_componentes;
        if (dividir_por_componentes(imagem, grupo, &primeiro_componente,
                                    &segundo_componente, &erro_componentes)) {
            melhor_erro_divisao = erro_componentes;
            primeiro_melhor = primeiro_componente;
            segundo_melhor = segundo_componente;
        }

        for (size_t divisao = grupo.esquerda + 1; divisao < grupo.direita;
             divisao++) {
            Retangulo nucleo_esquerdo = grupo;
            Retangulo nucleo_direito = grupo;
            nucleo_esquerdo.direita = divisao;
            nucleo_direito.esquerda = divisao + 1;
            Retangulo caixa_nucleo_esquerdo;
            Retangulo caixa_nucleo_direito;
            if (!caixa_de_tinta(imagem, nucleo_esquerdo,
                                &caixa_nucleo_esquerdo) ||
                !caixa_de_tinta(imagem, nucleo_direito,
                                &caixa_nucleo_direito)) {
                continue;
            }

            for (size_t sobreposicao = 0; sobreposicao <= 2; sobreposicao++) {
                if (sobreposicao > grupo.direita - divisao) {
                    continue;
                }

                Retangulo limite_esquerdo = grupo;
                Retangulo limite_direito = grupo;
                limite_esquerdo.direita = divisao + sobreposicao;
                limite_direito.esquerda = divisao + 1 > sobreposicao
                                              ? divisao + 1 - sobreposicao
                                              : grupo.esquerda;
                limite_esquerdo.topo = caixa_nucleo_esquerdo.topo;
                limite_esquerdo.base = caixa_nucleo_esquerdo.base;
                limite_direito.topo = caixa_nucleo_direito.topo;
                limite_direito.base = caixa_nucleo_direito.base;

                Retangulo caixa_esquerda;
                Retangulo caixa_direita;
                if (!caixa_de_tinta(imagem, limite_esquerdo, &caixa_esquerda) ||
                    !caixa_de_tinta(imagem, limite_direito, &caixa_direita)) {
                    continue;
                }
                if (caixa_esquerda.direita - caixa_esquerda.esquerda + 1 < 2 ||
                    caixa_direita.direita - caixa_direita.esquerda + 1 < 2) {
                    continue;
                }

                char primeiro;
                char segundo;
                double erro_primeiro = classificar_glifo(imagem, caixa_esquerda,
                                                          &primeiro);
                double erro_segundo = classificar_glifo(imagem, caixa_direita,
                                                         &segundo);
                double erro_divisao = (erro_primeiro + erro_segundo) / 2.0 + 0.10;
                if (erro_divisao < melhor_erro_divisao) {
                    melhor_erro_divisao = erro_divisao;
                    primeiro_melhor = primeiro;
                    segundo_melhor = segundo;
                }
            }
        }
    }

    if (melhor_erro_divisao + 0.02 < erro_inteiro) {
        return adicionar_caractere_texto(texto, primeiro_melhor) &&
               adicionar_caractere_texto(texto, segundo_melhor);
    }

    caractere_inteiro = ajustar_pontuacao_vertical(caractere_inteiro, grupo,
                                                   palavra);
    return adicionar_caractere_texto(texto, caractere_inteiro);
}

static bool reconhecer_palavra(const Imagem *imagem, Retangulo palavra,
                               BufferTexto *texto) {
    size_t largura = palavra.direita - palavra.esquerda + 1;
    unsigned char *ativo = calloc(largura, sizeof(*ativo));
    if (ativo == NULL) {
        return false;
    }

    for (size_t deslocamento = 0; deslocamento < largura; deslocamento++) {
        size_t x = palavra.esquerda + deslocamento;
        for (size_t y = palavra.topo; y <= palavra.base; y++) {
            if (obter_pixel(imagem, y, x) != 0) {
                ativo[deslocamento] = 1;
                break;
            }
        }
    }

    ListaFaixas caracteres = {0};
    bool sucesso = encontrar_faixas(ativo, largura, 0, &caracteres);
    free(ativo);
    if (!sucesso) {
        return false;
    }
    if (caracteres.quantidade == 0) {
        liberar_faixas(&caracteres);
        return true;
    }

    if (caracteres.quantidade > SIZE_MAX / sizeof(Retangulo) ||
        caracteres.quantidade > SIZE_MAX / sizeof(double)) {
        liberar_faixas(&caracteres);
        return false;
    }
    Retangulo *caixas = malloc(caracteres.quantidade * sizeof(*caixas));
    double *erros = malloc(caracteres.quantidade * sizeof(*erros));
    char *classes = malloc(caracteres.quantidade * sizeof(*classes));
    if (caixas == NULL || erros == NULL || classes == NULL) {
        free(caixas);
        free(erros);
        free(classes);
        liberar_faixas(&caracteres);
        return false;
    }

    for (size_t indice = 0; indice < caracteres.quantidade; indice++) {
        Retangulo limite = {
            .topo = palavra.topo,
            .base = palavra.base,
            .esquerda = palavra.esquerda + caracteres.itens[indice].inicio,
            .direita = palavra.esquerda + caracteres.itens[indice].fim,
        };
        if (!caixa_de_tinta(imagem, limite, &caixas[indice])) {
            free(caixas);
            free(erros);
            free(classes);
            liberar_faixas(&caracteres);
            return false;
        }
        erros[indice] = classificar_glifo(imagem, caixas[indice], &classes[indice]);
    }

    size_t indice = 0;
    while (indice < caracteres.quantidade) {
        size_t quantidade_unida = 1;
        double melhor_ganho = 0.0;
        char melhor_classe_unida = '?';
        Retangulo melhor_caixa_unida = caixas[indice];
        double soma_erros = erros[indice];

        for (size_t quantidade = 2;
             quantidade <= 3 && indice + quantidade <= caracteres.quantidade;
             quantidade++) {
            size_t atual = indice + quantidade - 1;
            size_t anterior = atual - 1;
            size_t lacuna = caracteres.itens[atual].inicio -
                            caracteres.itens[anterior].fim - 1;
            if (lacuna > 1) {
                break;
            }

            soma_erros += erros[atual];
            Retangulo limite_unido = palavra;
            limite_unido.esquerda = caixas[indice].esquerda;
            limite_unido.direita = caixas[atual].direita;
            Retangulo caixa_unida;
            if (!caixa_de_tinta(imagem, limite_unido, &caixa_unida)) {
                continue;
            }

            char classe_unida;
            double erro_unido = classificar_glifo(imagem, caixa_unida,
                                                   &classe_unida);
            double erro_separado = soma_erros / (double)quantidade;
            double penalidade = 0.04 * (double)(quantidade - 1);
            double ganho = erro_separado - erro_unido - penalidade;
            if (ganho > melhor_ganho) {
                melhor_ganho = ganho;
                quantidade_unida = quantidade;
                melhor_classe_unida = classe_unida;
                melhor_caixa_unida = caixa_unida;
            }
        }

        bool adicionou;
        if (quantidade_unida > 1) {
            melhor_classe_unida = ajustar_pontuacao_vertical(melhor_classe_unida,
                                                             melhor_caixa_unida,
                                                             palavra);
            adicionou = adicionar_caractere_texto(texto, melhor_classe_unida);
        } else {
            adicionou = reconhecer_grupo(imagem, caixas[indice], palavra,
                                         classes[indice], erros[indice], texto);
        }
        if (!adicionou) {
            free(caixas);
            free(erros);
            free(classes);
            liberar_faixas(&caracteres);
            return false;
        }
        indice += quantidade_unida;
    }

    free(caixas);
    free(erros);
    free(classes);
    liberar_faixas(&caracteres);
    return true;
}

static bool reconhecer_documento(const Imagem *imagem, const Resultado *resultado,
                                 BufferTexto *texto) {
    memset(texto, 0, sizeof(*texto));

    for (size_t indice = 0; indice < resultado->palavras.quantidade; indice++) {
        const PalavraSegmentada *palavra = &resultado->palavras.itens[indice];
        if (indice > 0) {
            const PalavraSegmentada *anterior = &resultado->palavras.itens[indice - 1];
            const char *separador;
            if (palavra->coluna != anterior->coluna) {
                separador = "\n\n";
            } else if (palavra->linha != anterior->linha) {
                separador = "\n";
            } else {
                separador = " ";
            }
            if (!adicionar_texto(texto, separador)) {
                liberar_buffer_texto(texto);
                return false;
            }
        }

        if (!reconhecer_palavra(imagem, palavra->caixa, texto)) {
            liberar_buffer_texto(texto);
            return false;
        }
    }

    if (!adicionar_caractere_texto(texto, '\n')) {
        liberar_buffer_texto(texto);
        return false;
    }
    return true;
}

static bool escrever_texto(const char *caminho, const BufferTexto *texto) {
    FILE *arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s': %s.\n",
                caminho, strerror(errno));
        return false;
    }

    bool sucesso = fwrite(texto->dados, 1, texto->tamanho, arquivo) == texto->tamanho;
    if (fclose(arquivo) != 0) {
        sucesso = false;
    }
    if (!sucesso) {
        fprintf(stderr, "Erro ao escrever o texto reconhecido em '%s'.\n", caminho);
    }
    return sucesso;
}

static void desenhar_retangulos(Imagem *imagem, const ListaPalavras *retangulos,
                                size_t margem) {
    for (size_t indice = 0; indice < retangulos->quantidade; indice++) {
        Retangulo original = retangulos->itens[indice].caixa;
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
            "Uso: %s [--negrito[=1..3]] [--reconhecer=texto.txt] "
            "<entrada.pbm> [saida.pbm]\n",
            programa);
}

static bool ler_opcoes(int argc, char *argv[], Opcoes *opcoes) {
    memset(opcoes, 0, sizeof(*opcoes));
    opcoes->saida = "saida.pbm";

    bool fim_das_opcoes = false;
    bool negrito_definido = false;
    bool reconhecimento_definido = false;
    size_t quantidade_caminhos = 0;

    for (int indice = 1; indice < argc; indice++) {
        const char *argumento = argv[indice];

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

        bool opcao_reconhecer = !fim_das_opcoes &&
                                 strcmp(argumento, "--reconhecer") == 0;
        bool opcao_reconhecer_com_valor = !fim_das_opcoes &&
                                           strncmp(argumento, "--reconhecer=", 13) == 0;
        if (opcao_reconhecer || opcao_reconhecer_com_valor) {
            if (reconhecimento_definido) {
                fprintf(stderr,
                        "Erro: a opcao de reconhecimento foi informada mais de uma vez.\n");
                return false;
            }

            const char *caminho_texto;
            if (opcao_reconhecer) {
                if (indice + 1 >= argc) {
                    fprintf(stderr,
                            "Erro: informe o arquivo TXT depois de --reconhecer.\n");
                    return false;
                }
                caminho_texto = argv[++indice];
            } else {
                caminho_texto = argumento + 13;
            }
            if (*caminho_texto == '\0') {
                fprintf(stderr, "Erro: o caminho da saida de texto esta vazio.\n");
                return false;
            }

            opcoes->saida_texto = caminho_texto;
            reconhecimento_definido = true;
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
    if (opcoes->saida_texto != NULL &&
        (strcmp(opcoes->saida_texto, opcoes->entrada) == 0 ||
         strcmp(opcoes->saida_texto, opcoes->saida) == 0)) {
        fprintf(stderr,
                "Erro: as saidas PBM e TXT devem usar caminhos diferentes da entrada e entre si.\n");
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
    BufferTexto texto_reconhecido = {0};
    int codigo = EXIT_FAILURE;

    if (!ler_imagem_pbm(opcoes.entrada, &original)) {
        goto finalizar;
    }
    if (!remover_ruido_impulsivo(&original, &limpa)) {
        fprintf(stderr, "Erro: memoria insuficiente durante a remocao de ruido.\n");
        goto finalizar;
    }
    liberar_imagem(&original);

    if (!analisar_imagem(&limpa, &resultado)) {
        fprintf(stderr, "Erro: memoria insuficiente durante a segmentacao.\n");
        goto finalizar;
    }

    printf("Total de palavras encontradas: %zu\n", resultado.palavras.quantidade);
    printf("Total de linhas encontradas: %zu\n", resultado.linhas);
    printf("Total de colunas encontradas: %zu\n", resultado.colunas);

    if (opcoes.saida_texto != NULL) {
        if (!reconhecer_documento(&limpa, &resultado, &texto_reconhecido)) {
            fprintf(stderr, "Erro: memoria insuficiente durante o reconhecimento.\n");
            goto finalizar;
        }
        if (!escrever_texto(opcoes.saida_texto, &texto_reconhecido)) {
            goto finalizar;
        }
    }

    Imagem *imagem_saida = &limpa;
    if (opcoes.espessura_negrito > 0) {
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
    liberar_buffer_texto(&texto_reconhecido);
    liberar_resultado(&resultado);
    liberar_imagem(&negrito);
    liberar_imagem(&limpa);
    liberar_imagem(&original);
    return codigo;
}
