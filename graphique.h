#ifndef GRAPHIQUE_H
#define GRAPHIQUE_H

#include <cairo.h>
#include "mesure.h"
#include "tri.h"

/* Dessine les courbes dans le contexte cairo 'cr' (taille largeur x hauteur).
   noms[algo] = nom affiche dans la legende. elog = 1 pour l'echelle logarithmique. */
void graphique_dessiner(cairo_t *cr, int largeur, int hauteur,
                        const Resultat *res, int nb, int elog,
                        const char *titre, const char *axe_x, const char *axe_y,
                        const char *const *noms);

/* Meme dessin, enregistre dans une image PNG (retour 1 si ok) */
int graphique_exporter_png(const char *fichier,
                           const Resultat *res, int nb, int elog,
                           const char *titre, const char *axe_x, const char *axe_y,
                           const char *const *noms);

#endif
