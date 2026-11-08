#include <stdlib.h>
#include <string.h>
#include "tableau.h"

/* rand() donne seulement 0..32767 sous Windows, on le combine */
static int alea(int max)
{
    long grand = ((long)rand() << 15) | rand();
    return (int)(grand % max);
}

static int comparer(const void *a, const void *b)
{
    double x = *(const double *)a;
    double y = *(const double *)b;
    return (x > y) - (x < y);
}

double *tableau_generer(int n, TypeValeur type, OrdreInitial ordre)
{
    int i;
    double *t = malloc(n * sizeof(double));
    if (t == NULL) return NULL;

    for (i = 0; i < n; i++) {
        if (type == TYPE_INT)        t[i] = alea(100000);
        else if (type == TYPE_FLOAT) t[i] = (float)(alea(1000000) / 10.0);
        else if (type == TYPE_CHAR)  t[i] = 65 + alea(58);   /* 'A' a 'z' */
        else                         t[i] = alea(10000000) / 100.0;
    }

    if (ordre != ORDRE_ALEATOIRE) {
        qsort(t, n, sizeof(double), comparer);
        if (ordre == ORDRE_DECROISSANT) {
            for (i = 0; i < n / 2; i++) {
                double tmp = t[i];
                t[i] = t[n - 1 - i];
                t[n - 1 - i] = tmp;
            }
        }
    }
    return t;
}

double *tableau_copier(const double *source, int n)
{
    double *c = malloc(n * sizeof(double));
    if (c != NULL) memcpy(c, source, n * sizeof(double));
    return c;
}

int tableau_lire_texte(const char *texte, double **resultat, int *n)
{
    double tmp[100];
    int nb = 0;
    const char *p = texte;
    char *fin;

    while (*p) {
        double v;
        while (*p == ' ' || *p == ',' || *p == ';' || *p == '\t') p++;
        if (*p == '\0') break;

        v = strtod(p, &fin);
        if (fin == p) return 1;                                  /* pas un nombre */
        if (*fin != '\0' && strchr(" ,;\t", *fin) == NULL) return 1;

        if (nb < 100) tmp[nb] = v;
        nb++;
        p = fin;
    }

    if (nb < 10 || nb > 30) return 2;

    *resultat = malloc(nb * sizeof(double));
    if (*resultat == NULL) return 2;
    memcpy(*resultat, tmp, nb * sizeof(double));
    *n = nb;
    return 0;
}
