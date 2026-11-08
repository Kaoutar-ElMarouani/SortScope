#ifndef TRI_H
#define TRI_H

typedef enum {
    TRI_SELECTION, TRI_INSERTION, TRI_BULLES,
    TRI_FUSION, TRI_SHELL, TRI_RAPIDE,
    NB_TRIS
} AlgoTri;

void tri_executer(int algo, double *t, int n);   /* lance l'algorithme choisi */
int  tri_est_trie(const double *t, int n);       /* 1 si le tableau est trie */
const char *tri_nom(int algo);                   /* nom fixe (pour le CSV) */

#endif
