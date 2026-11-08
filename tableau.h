#ifndef TABLEAU_H
#define TABLEAU_H

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_CHAR, TYPE_DOUBLE } TypeValeur;
typedef enum { ORDRE_ALEATOIRE, ORDRE_CROISSANT, ORDRE_DECROISSANT } OrdreInitial;

/* Cree un tableau de n valeurs (a liberer avec free) */
double *tableau_generer(int n, TypeValeur type, OrdreInitial ordre);

/* Copie un tableau (a liberer avec free) */
double *tableau_copier(const double *source, int n);

/* Lit des nombres ecrits dans un texte.
   Retour : 0 = ok, 1 = nombre invalide, 2 = il faut entre 10 et 30 valeurs */
int tableau_lire_texte(const char *texte, double **resultat, int *n);

#endif
