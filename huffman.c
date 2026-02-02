#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

#define BITREADER_IMPLEMENTATION
#include "bitreader.h"

#define BITWRITER_IMPLEMENTATION
#include "bitwriter.h"

char *read_file(const char * file_name){
    long taille;
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Error opening the file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    taille = ftell(file);
    rewind(file);

    char *contenu = malloc(taille + 1);
    if (contenu == NULL) return NULL;

    fread(contenu, 1, taille, file);
    contenu[taille] = '\0';
    fclose(file);
    return contenu;
}

void count_frequencies(char *content, node freq[256])
{
    size_t i;
    for(i =0 ; content[i] != '\0'; i++){
        unsigned char index = (unsigned char)content[i];
        if (freq[index].freq == 0){
            freq[index].freq = 1;
            freq[index].symbol = content[i];
        }else{
            freq[index].freq++;
        }
    }
}

typedef struct{
    Tree **tab;
    int size;
}list;

Tree *build_huffman_tree(node tab[256])
{
    int n = 0;
    int i;
    for(i = 0; i<256; i++ ) {
        if (tab[i].freq != 0) n++;
    }
    list *liste = malloc(sizeof(list));
    liste->size = n;
    liste->tab = malloc(n * sizeof(Tree *));

    int index = 0;
    for(i = 0; i < 256; i++){
        if (tab[i].freq != 0){
            Tree *a = new_leaf(tab[i].symbol, tab[i].freq);
            liste->tab[index++] = a;
        }
    }

    while (liste->size > 1) {
        /*find two min*/
        int m1 = 0;
        int m2 = 1;
        for(i=2; i < liste->size ; i++){
            if ((liste->tab[i]->Node.freq < liste->tab[m1]->Node.freq)
                ||
                (liste->tab[i]->Node.freq  < liste->tab[m2]->Node.freq)){
                if (liste->tab[i]->Node.freq < liste->tab[m1]->Node.freq){
                    m1 = i;
                }else{
                    m2 = i;
                }
            }
        }

        Tree *new = new_node(liste->tab[m1], liste->tab[m2]);

        if (m1 > m2) { int t = m1; m1 = m2; m2 = t; }
        liste->tab[m1] = new;
        liste->tab[m2] = liste->tab[liste->size - 1];
        liste->size--;

    }

    Tree *res = liste->tab[0];

    free(liste->tab);
    free(liste);
    return res;
}

typedef struct {
    char symbol;
    char *bits;
} HuffmanCode;

void build_encoding_table(Tree *t, HuffmanCode tab[256], char *buf, int depth){
    if (!t) return;

    if ((!t->left) && (!t->right)) {
        tab[(unsigned char)t->Node.symbol].symbol = t->Node.symbol;
        tab[(unsigned char)t->Node.symbol].bits = malloc(depth + 1);
        for(int i = 0; i < depth; i++){
            tab[(unsigned char)t->Node.symbol].bits[i] = buf[i];
        }
        tab[(unsigned char)t->Node.symbol].bits[depth] = '\0';
        return;
    }

    buf[depth] = '0';
    build_encoding_table(t->left, tab, buf, depth + 1);
    buf[depth] = '1';
    build_encoding_table(t->right, tab, buf, depth + 1);
    return;
}


void compress_tree(Tree *t, bitwriter *bw){
    if (!t) return;

    if ((!t->left) && (!t->right)) {
        bw_write(bw, 1);
        bw_write_byte(bw, (unsigned char)t->Node.symbol);
        return;
    }
    bw_write(bw, 0);
    compress_tree(t->left, bw);
    compress_tree(t->right, bw);
    return;
}

int huffman_compress(const char *file_in,const char *file_out){
    FILE *f = fopen(file_out, "wb");
    if (f == NULL) {
        perror("Erreur opening output file");
        return 1;
    }

    char *content = read_file(file_in);
    if (content == NULL) return 1;

    node tab[256] = {0};
    count_frequencies(content, tab);

    Tree *t = build_huffman_tree(tab);
    char buffer[256];
    HuffmanCode map[256] = {0};
    build_encoding_table(t, map, buffer, 0);

    bitwriter bw;
    bw_init(&bw, f);
    uint32_t size = strlen(content);
    fwrite(&size, sizeof(uint32_t), 1, f);
    compress_tree(t, &bw);

    for (size_t i = 0; content[i] != '\0'; i++) {
        unsigned char c = content[i];
        char *code = map[c].bits;

        for (int j = 0; code[j]; j++) {
            bw_write(&bw, code[j] == '1');
        }
    }
    bw_flush(&bw);

    for (int i = 0; i < 256; i++) {
        free(map[i].bits);
    }
    free_tree(t);
    free(content);
    fclose(f);
    return 0;
}



Tree *read_Tree(bitreader *br){
    int bit = br_read(br);
    if (bit == 1){
        char c = (char)br_read_byte(br);
        return new_leaf(c, 1);
    }else{
        Tree *left = read_Tree(br);
        Tree *right = read_Tree(br);
        return new_node(left, right);
    }
}

void decode(Tree* a, bitreader *br,FILE *out, uint32_t size){
    for(uint32_t i=0; i<size; i++){
        Tree *cur = a;
        while (cur->left || cur->right){
            int bit = br_read(br);
            if (bit == -1) {
                fprintf(stderr,"Error: unexpected end of file while decoding (%u / %u)\n",i, size);
                return;
            }
            cur = bit ? cur->right : cur->left;
        }
        fputc(cur->Node.symbol, out);
    }
}

int huffman_decompress(const char *file_in,const char *file_out) {
    FILE *f = fopen(file_in, "rb");
    if (!f) {
        perror("Error opening input file");
        return 1;
    }

    FILE *out = fopen(file_out, "w");
    if (!out) {
        perror("Error opening output file");
        fclose(f);
        return 1;
    }

    bitreader br;
    init_br(&br, f);

    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, f);

    Tree *a = read_Tree(&br);
    decode(a, &br, out, size);

    fclose(f);
    fclose(out);
    free_tree(a);
    return 0;
}


static void print_usage(const char *prog)
{
    fprintf(stderr,
        "Usage:\n"
        "  %s -c <input> <output>   Compress file\n"
        "  %s -d <input> <output>   Decompress file\n",
        prog, prog);
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    const char *option = argv[1];
    const char *input  = argv[2];
    const char *output = argv[3];

    if (strcmp(option, "-c") == 0) {
        return huffman_compress(input, output);
    }

    if (strcmp(option, "-d") == 0) {
        return huffman_decompress(input, output);
    }

    fprintf(stderr, "Unknown option: %s\n", option);
    print_usage(argv[0]);
    return 1;
}
