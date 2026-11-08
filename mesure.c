#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mesure.h"
#include "tri.h"

double mesure_temps(int algo, const double *original, int n, int repetitions, int *ok)
{
    int r;
    double total = 0;
    double *copie = malloc(n * sizeof(double));

    *ok = 1;
    if (copie == NULL) { *ok = 0; return 0; }

    for (r = 0; r < repetitions; r++) {
        gint64 debut, fin;
        memcpy(copie, original, n * sizeof(double));   /* meme tableau a chaque fois */

        debut = g_get_monotonic_time();                /* microsecondes */
        tri_executer(algo, copie, n);                  /* seul le tri est chronometre */
        fin = g_get_monotonic_time();

        total += (fin - debut) / 1000000.0;
        if (!tri_est_trie(copie, n)) *ok = 0;
    }
    free(copie);
    return total / repetitions;
}

int mesure_exporter_csv(const char *fichier, const Resultat *res, int nb)
{
    int i;
    FILE *f = g_fopen(fichier, "w");
    if (f == NULL) return 0;

    fprintf(f, "algorithme;taille;temps_secondes\n");
    for (i = 0; i < nb; i++)
        fprintf(f, "%s;%d;%.9f\n", tri_nom(res[i].algo), res[i].taille, res[i].temps);

    fclose(f);
    return 1;
}
