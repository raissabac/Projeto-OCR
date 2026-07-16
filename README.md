# Projeto OCR

Pré-processador de imagens PBM ASCII (`P1`) desenvolvido para a disciplina de
Processamento de Imagens. O programa remove ruído impulsivo, conta colunas,
linhas e palavras e gera uma nova imagem com cada palavra circunscrita por um
retângulo.

## Compilação

Em Linux, usando GCC:

```sh
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wformat=2 -Wformat-signedness main.c -o ocr
```

## Execução

```sh
./ocr [--negrito[=1..3]] entrada.pbm [saida.pbm]
```

O caminho de saída é opcional. Quando ele não é informado, o programa cria
`saida.pbm` no diretório atual.

Exemplo:

```sh
./ocr Teste/lorem_s12_c02.pbm resultado.pbm
./ocr --negrito Teste/lorem_s12_c02.pbm resultado_negrito.pbm
./ocr --negrito=2 Teste/lorem_s12_c02.pbm resultado_negrito_2.pbm
```

`--negrito` (ou `-b`) engrossa o texto em um pixel. A forma
`--negrito=N` permite escolher uma espessura entre 1 e 3. O efeito é aplicado
somente depois das contagens, portanto não altera os resultados da segmentação.

A contagem é exibida no terminal:

```text
Total de palavras encontradas: 583
Total de linhas encontradas: 99
Total de colunas encontradas: 2
```

O programa aceita comentários em qualquer parte válida do PBM P1 e encerra
com erro quando encontra formato, dimensões ou pixels inválidos.
