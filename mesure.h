#ifndef MESURE_H
#define MESURE_H

typedef struct {
    int    algo;     /* numero de l'algorithme (voir tri.h) */
    int    taille;   /* taille du tableau */
    double temps;    /* temps moyen en secondes */
} Resultat;

/* Mesure le temps moyen d'un tri sur 'repetitions' copies du tableau.
   *ok vaut 1 si le tableau etait bien trie a chaque fois. */
double mesure_temps(int algo, const double *original, int n, int repetitions, int *ok);

/* Ecrit les resultats dans un fichier CSV (retour 1 si ok) */
int mesure_exporter_csv(const char *fichier, const Resultat *res, int nb);

#endif
