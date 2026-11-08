#include <stdlib.h>
#include "tri.h"

static void echanger(double *a, double *b)
{
    double tmp = *a;
    *a = *b;
    *b = tmp;
}

static void tri_selection(double *t, int n)
{
    int i, j, min;
    for (i = 0; i < n - 1; i++) {
        min = i;
        for (j = i + 1; j < n; j++)
            if (t[j] < t[min]) min = j;
        if (min != i) echanger(&t[i], &t[min]);
    }
}

static void tri_insertion(double *t, int n)
{
    int i, j;
    double cle;
    for (i = 1; i < n; i++) {
        cle = t[i];
        j = i - 1;
        while (j >= 0 && t[j] > cle) {
            t[j + 1] = t[j];
            j--;
        }
        t[j + 1] = cle;
    }
}

static void tri_bulles(double *t, int n)
{
    int i, j, echange;
    for (i = 0; i < n - 1; i++) {
        echange = 0;
        for (j = 0; j < n - 1 - i; j++) {
            if (t[j] > t[j + 1]) {
                echanger(&t[j], &t[j + 1]);
                echange = 1;
            }
        }
        if (!echange) break;   /* deja trie */
    }
}

/* ---- fusion ---- */
static void fusionner(double *t, double *tmp, int debut, int milieu, int fin)
{
    int i = debut, j = milieu, k = debut;
    while (i < milieu && j < fin) {
        if (t[i] <= t[j]) tmp[k++] = t[i++];
        else              tmp[k++] = t[j++];
    }
    while (i < milieu) tmp[k++] = t[i++];
    while (j < fin)    tmp[k++] = t[j++];
    for (k = debut; k < fin; k++) t[k] = tmp[k];
}

static void fusion_rec(double *t, double *tmp, int debut, int fin)
{
    int milieu;
    if (fin - debut < 2) return;
    milieu = (debut + fin) / 2;
    fusion_rec(t, tmp, debut, milieu);
    fusion_rec(t, tmp, milieu, fin);
    fusionner(t, tmp, debut, milieu, fin);
}

static void tri_fusion(double *t, int n)
{
    double *tmp = malloc(n * sizeof(double));
    if (tmp == NULL) return;
    fusion_rec(t, tmp, 0, n);
    free(tmp);
}

static void tri_shell(double *t, int n)
{
    int ecart, i, j;
    double cle;
    for (ecart = n / 2; ecart > 0; ecart /= 2) {
        for (i = ecart; i < n; i++) {
            cle = t[i];
            j = i;
            while (j >= ecart && t[j - ecart] > cle) {
                t[j] = t[j - ecart];
                j -= ecart;
            }
            t[j] = cle;
        }
    }
}

/* ---- rapide (pivot au milieu) ---- */
static void rapide_rec(double *t, int bas, int haut)
{
    while (bas < haut) {
        double pivot = t[(bas + haut) / 2];
        int i = bas, j = haut;
        while (i <= j) {
            while (t[i] < pivot) i++;
            while (t[j] > pivot) j--;
            if (i <= j) {
                echanger(&t[i], &t[j]);
                i++;
                j--;
            }
        }
        /* on traite le petit cote en recursif, le grand dans la boucle */
        if (j - bas < haut - i) {
            rapide_rec(t, bas, j);
            bas = i;
        } else {
            rapide_rec(t, i, haut);
            haut = j;
        }
    }
}

static void tri_rapide(double *t, int n)
{
    rapide_rec(t, 0, n - 1);
}

/* ---- fonctions publiques ---- */
void tri_executer(int algo, double *t, int n)
{
    switch (algo) {
        case TRI_SELECTION: tri_selection(t, n); break;
        case TRI_INSERTION: tri_insertion(t, n); break;
        case TRI_BULLES:    tri_bulles(t, n);    break;
        case TRI_FUSION:    tri_fusion(t, n);    break;
        case TRI_SHELL:     tri_shell(t, n);     break;
        case TRI_RAPIDE:    tri_rapide(t, n);    break;
    }
}

int tri_est_trie(const double *t, int n)
{
    int i;
    for (i = 1; i < n; i++)
        if (t[i - 1] > t[i]) return 0;
    return 1;
}

const char *tri_nom(int algo)
{
    static const char *noms[NB_TRIS] = {
        "selection", "insertion", "bulles", "fusion", "shell", "rapide"
    };
    return noms[algo];
}
