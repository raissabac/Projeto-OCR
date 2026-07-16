# Projeto OCR

Pré-processador de imagens PBM ASCII (`P1`) desenvolvido para a disciplina de
Processamento de Imagens. O programa remove ruído impulsivo, conta colunas,
linhas e palavras e gera uma nova imagem com cada palavra circunscrita por um
retângulo. Opcionalmente, também reconhece os caracteres e grava a leitura em
um arquivo TXT.

## Compilação

Em Linux, usando GCC:

```sh
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic main.c -o ocr
```

## Execução

```sh
./ocr [--negrito[=1..3]] [--reconhecer=texto.txt] entrada.pbm [saida.pbm]
```

O caminho de saída é opcional. Quando ele não é informado, o programa cria
`saida.pbm` no diretório atual.

Exemplo:

```sh
./ocr Teste/lorem_s12_c02.pbm resultado.pbm
./ocr --negrito Teste/lorem_s12_c02.pbm resultado_negrito.pbm
./ocr --negrito=2 Teste/lorem_s12_c02.pbm resultado_negrito_2.pbm
./ocr --reconhecer=resultado.txt Teste/lorem_s12_c02.pbm resultado.pbm
./ocr --negrito=2 --reconhecer=resultado.txt \
    Teste/lorem_s12_c02.pbm resultado_negrito_2.pbm
```

`--negrito` (ou `-b`) engrossa o texto em um pixel. A forma
`--negrito=N` permite escolher uma espessura entre 1 e 3. O efeito é aplicado
somente depois das contagens, portanto não altera os resultados da segmentação.

`--reconhecer=arquivo.txt` ativa o OCR. Também é aceita a forma
`--reconhecer arquivo.txt`. O TXT preserva a ordem de leitura: espaços entre
palavras, quebras entre linhas e uma linha vazia entre colunas. O reconhecimento
é executado sobre a imagem filtrada, antes do negrito e dos retângulos.

Os modelos embutidos cobrem letras ASCII maiúsculas e minúsculas, dígitos e os
sinais `.,;:!?"'()-/`. Eles foram calibrados para Arial regular de 12 pontos
rasterizada a 200 DPI, configuração dos documentos de teste. No ambiente Linux
usado para gerar os arquivos, Arial foi substituída pela compatível Liberation
Sans. Outras fontes, tamanhos e caracteres podem reduzir a precisão ou produzir
`?`.

A contagem é exibida no terminal:

```text
Total de palavras encontradas: 583
Total de linhas encontradas: 99
Total de colunas encontradas: 2
```

O programa aceita comentários em qualquer parte válida do PBM P1 e encerra
com erro quando encontra formato, dimensões ou pixels inválidos.

Uma descrição das técnicas e dos parâmetros está em [RELATORIO.md](RELATORIO.md).
