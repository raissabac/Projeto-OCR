# Relatório técnico — Pré-processamento para OCR

## 1. Trabalho desenvolvido

O programa recebe uma imagem binária PBM ASCII (`P1`), remove ruído impulsivo,
segmenta o texto e informa o número de colunas, linhas e palavras. Em seguida,
produz outra imagem PBM com um retângulo ao redor de cada palavra. Quando
solicitado, reconhece os caracteres e grava o conteúdo em um arquivo TXT.

Opcionalmente, o texto da imagem de saída pode receber negrito com espessura de
um a três pixels. Esse recurso é aplicado apenas depois da análise e não altera
as contagens.

A segmentação é hierárquica: primeiro são identificadas as colunas; as linhas
são procuradas separadamente dentro de cada coluna; por fim, as palavras são
separadas dentro de cada linha. Dessa forma, linhas que ocupam a mesma posição
vertical, mas pertencem a colunas diferentes, são contadas separadamente.

## 2. Técnicas e parâmetros

### Leitura da imagem

O leitor interpreta o cabeçalho e o raster P1 por tokens, ignorando espaços e
comentários. O número mágico, as dimensões, os pixels e possíveis estouros de
tamanho são validados antes do processamento.

### Remoção de ruído

Foi empregado um filtro majoritário com elemento estruturante em cruz de cinco
pixels. O pixel de saída é preto quando pelo menos três posições da cruz são
pretas. Esse filtro remove pontos pretos isolados e preenche pequenos pontos
brancos dentro dos traços, atendendo ao caso de ruído sal e pimenta de um pixel.

### Detecção de colunas

É calculada a projeção vertical da imagem filtrada. Uma posição vertical é
considerada ocupada quando possui pelo menos `max(2, altura/1000)` pixels
pretos. Pequenas lacunas internas são unidas usando como referência a altura
mediana estimada do texto, respeitando a hipótese de letras com altura mínima
de 12 pixels. Grandes faixas verticais vazias permanecem como separadores de
colunas.

### Detecção de linhas

Para cada coluna é calculada uma projeção horizontal. Linhas com pelo menos
dois pixels pretos são consideradas ocupadas, lacunas internas de até dois
pixels são unidas e componentes residuais muito pequenos são descartados.

### Detecção de palavras

Dentro de cada linha é feita outra projeção vertical. Lacunas com largura igual
ou superior a `max(4, ceil(altura_da_linha/4))` separam palavras. Em fontes
próximas da altura mínima é aceito um pixel preto por posição da projeção; em
fontes maiores são exigidos dois, evitando que um ponto residual de ruído una
duas palavras.

Os limites reais dos pixels de cada palavra são armazenados sem margem. No modo
normal, o retângulo é desenhado um pixel além desses limites; com negrito, a
margem cresce junto com a espessura.

### Reconhecimento de caracteres

Cada palavra preserva os índices da coluna e da linha em que foi encontrada.
Dentro dela, uma projeção vertical fornece os candidatos a caractere. O
algoritmo compara também candidatos vizinhos separados por uma coluna vazia,
reunindo fragmentos quando o glifo unido apresenta erro menor. Pares que se
encostam por kerning, como `ff`, são separados por componentes conexos ou por
uma busca de cortes com pequena sobreposição.

Cada candidato é recortado em seus limites reais e redimensionado, com a
proporção preservada, para uma grade binária de 20 por 28 pixels. A grade é
comparada com modelos embutidos no próprio `main.c`. A pontuação combina a
diferença de forma, pequenos deslocamentos de até um pixel e as dimensões do
glifo antes da normalização.

Os modelos representam Arial regular, 12 pontos a 200 DPI, fonte e escala
definidas nos documentos originais. O conjunto reconhecido contém letras
ASCII, dígitos e os sinais `.,;:!?"'()-/`. Essa calibração evita dependências
externas em tempo de execução; fontes, escalas ou símbolos diferentes podem
exigir novos modelos.

#### Criação dos modelos de glifo

A inspeção do `content.xml` dos arquivos ODT confirmou que o texto foi
configurado em Arial. Entretanto, o ambiente Linux usado na exportação não
possuía a fonte proprietária instalada: `fc-match Arial` resolveu o nome para
Liberation Sans e `pdffonts` confirmou que essa substituta compatível foi
incorporada aos PDFs. Portanto, Arial é a fonte original dos documentos, e
Liberation Sans é apenas a forma efetivamente rasterizada nos arquivos de
teste. A resolução foi obtida pela relação entre o tamanho A4 das páginas e os
PBMs de 1653 por 2338 pixels, correspondente a aproximadamente 200 DPI.

Em seguida, um gerador auxiliar percorreu o alfabeto ASCII maiúsculo e
minúsculo, os dígitos e os sinais suportados. Cada símbolo foi renderizado
isoladamente pelo ImageMagick com a substituta de Arial realmente presente nos
PDFs, em 12 pontos e 200 DPI. Por exemplo, a geração da letra `A` usou uma
chamada equivalente a:

```sh
magick -density 200 -background white -fill black \
    -font Liberation-Sans -pointsize 12 'label:A' \
    -threshold 50% -trim +repage -compress none glifo_A.pbm
```

O limiar de 50% converteu a renderização em uma imagem estritamente binária.
Depois disso, cada glifo passou pelas mesmas etapas aplicadas pelo programa aos
documentos:

1. aplicação do filtro majoritário em cruz, mantendo o pixel preto quando ao
   menos três das cinco posições da vizinhança eram pretas;
2. remoção das linhas e colunas brancas externas;
3. registro da largura e da altura resultantes, antes do redimensionamento;
4. ajuste proporcional dentro de uma área útil de 18 por 26 pixels, centralizada
   com uma margem na grade final de 20 por 28;
5. amostragem pelo centro dos pixels e codificação de cada linha da grade como
   uma máscara de 20 bits em um `uint32_t`.

O resultado de cada símbolo foi inserido no vetor constante `MODELOS_GLIFOS`
como o código ASCII, as dimensões originais e 28 máscaras binárias. Durante a
validação, observou-se que a letra `t` assumia duas pequenas variações conforme
seu alinhamento subpixel no PDF. Exemplos dessas rasterizações reais, já
filtrados e normalizados, foram acrescentados como modelos alternativos para o
mesmo caractere.

O gerador foi necessário apenas para construir os dados. Na execução normal,
os modelos já estão incorporados ao `main.c`; portanto, o OCR não depende de
ImageMagick, das ferramentas de PDF nem de arquivos externos de treinamento.
Por fim, a tabela foi validada comparando o TXT reconhecido com o texto de
referência extraído dos PDFs por `pdftotext`.

O texto é reconstruído na ordem coluna, linha e palavra. São inseridos espaços
entre palavras, quebras entre linhas e uma linha vazia entre colunas. O OCR usa
a imagem filtrada antes de qualquer negrito ou desenho de retângulos.

### Negrito opcional

Como o PBM não armazena informações de fonte, o negrito é produzido por uma
dilatação morfológica em forma de losango, equivalente a repetir uma máscara em
cruz. A opção `--negrito` usa raio 1; `--negrito=N` aceita raios de 1 a 3.

A dilatação usa a imagem filtrada como origem imutável, impedindo que pixels
criados durante uma passagem aumentem a espessura além do parâmetro escolhido.
Ela ocorre depois da segmentação e antes dos retângulos. Na imagem final, a
margem de cada retângulo é `espessura + 1`, preservando um pixel branco entre o
texto engrossado e a borda.

## 3. Soluções para os principais problemas

- A contagem de linhas deixou de usar uma dilatação horizontal sobre a página
  inteira e passou a ser realizada dentro de cada coluna.
- Pontos de letras, sinais e partes desconectadas não são mais contados como
  palavras independentes, porque a decisão é feita pelas lacunas da linha.
- O filtro atua nas duas polaridades do ruído impulsivo.
- As máscaras que percorriam centenas de vizinhos por pixel foram substituídas
  por projeções. O custo principal passou a ser linear no número de pixels,
  `O(altura × largura)`.
- A imagem usa um buffer contíguo. São mantidas apenas a imagem original e a
  filtrada, além de vetores de projeção e dos retângulos.
- Todas as alocações e operações de arquivo são verificadas, e `realloc` usa um
  ponteiro temporário para não perder a alocação anterior em caso de falha.

## 4. Compilação e execução no Linux

```sh
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic main.c -o ocr
./ocr entrada.pbm saida.pbm
./ocr --negrito entrada.pbm saida_negrito.pbm
./ocr --negrito=2 entrada.pbm saida_negrito_2.pbm
./ocr --reconhecer=texto.txt entrada.pbm saida.pbm
./ocr --negrito=2 --reconhecer=texto.txt entrada.pbm saida_negrito_2.pbm
```

O segundo argumento é opcional; sem ele, a saída é gravada em `saida.pbm`.

## 5. Validação

| Família de teste | Colunas | Linhas | Palavras |
|---|---:|---:|---:|
| `lorem_s12_c02` | 2 | 99 | 583 |
| `lorem_s12_c02_just` | 2 | 99 | 583 |
| `lorem_s12_c02_espacos` | 2 | 81 | 476 |
| `lorem_s12_c03` | 3 | 150 | 557 |
| `lorem_s12_c03_just` | 3 | 150 | 557 |

As mesmas contagens foram obtidas nas versões correspondentes com ruído.
Nos cinco documentos limpos, o conteúdo ASCII reconhecido coincidiu com o texto
extraído dos PDFs de referência; nas versões justificadas, as quebras seguem as
linhas físicas detectadas na imagem. As versões com ruído conservaram a
estrutura e apresentaram apenas erros residuais isolados de caracteres.
