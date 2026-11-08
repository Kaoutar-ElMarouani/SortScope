#include <math.h>
#include <stdio.h>
#include <pango/pangocairo.h>
#include "graphique.h"

#define PLANCHER 1e-7   /* temps minimal pour l'echelle log */

static const double COULEURS[NB_TRIS][3] = {
    {0.89, 0.39, 0.08},  /* selection */
    {0.05, 0.49, 0.53},  /* insertion */
    {0.56, 0.27, 0.68},  /* bulles */
    {0.18, 0.55, 0.34},  /* fusion */
    {0.75, 0.22, 0.17},  /* shell */
    {0.17, 0.24, 0.31}   /* rapide */
};

/* Ecrit un texte avec Pango (necessaire pour afficher l'arabe correctement).
   align : 0 = gauche, 1 = centre, 2 = droite */
static void ecrire(cairo_t *cr, const char *txt, double x, double y,
                   int taille, int gras, int align)
{
    char police[40];
    int w, h;
    PangoLayout *lay = pango_cairo_create_layout(cr);
    PangoFontDescription *fd;

    snprintf(police, sizeof police, "Sans %s%d", gras ? "Bold " : "", taille);
    fd = pango_font_description_from_string(police);
    pango_layout_set_font_description(lay, fd);
    pango_font_description_free(fd);
    pango_layout_set_text(lay, txt, -1);
    pango_layout_get_pixel_size(lay, &w, &h);

    if (align == 1) x -= w / 2.0;
    if (align == 2) x -= w;
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, lay);
    g_object_unref(lay);
}

/* Convertit une valeur selon l'echelle (lineaire ou log10) */
static double conv(double v, int elog)
{
    if (!elog) return v;
    if (v < PLANCHER) v = PLANCHER;
    return log10(v);
}

void graphique_dessiner(cairo_t *cr, int largeur, int hauteur,
                        const Resultat *res, int nb, int elog,
                        const char *titre, const char *axe_x, const char *axe_y,
                        const char *const *noms)
{
    double g = 90, d = 25, haut = 75, bas = 100;      /* marges */
    double w = largeur - g - d, h = hauteur - haut - bas;
    double xmin, xmax, ymin, ymax, x0, x1, y0, y1;
    int i, a, k;
    char buf[40];
    int present[NB_TRIS] = {0};

    /* fond */
    cairo_set_source_rgb(cr, 1.0, 0.99, 0.97);
    cairo_paint(cr);
    if (w < 60 || h < 60) return;

    /* titre et unite de l'axe Y */
    cairo_set_source_rgb(cr, 0.06, 0.28, 0.29);
    ecrire(cr, titre, g, 14, 12, 1, 0);
    cairo_set_source_rgb(cr, 0.4, 0.35, 0.25);
    ecrire(cr, axe_y, g, 42, 10, 0, 0);

    /* cadre */
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 0.85, 0.80, 0.70);
    cairo_rectangle(cr, g, haut, w, h);
    cairo_stroke(cr);
    if (nb == 0) return;

    /* bornes des donnees */
    xmin = xmax = res[0].taille;
    ymax = PLANCHER;
    ymin = 1e9;
    for (i = 0; i < nb; i++) {
        if (res[i].taille < xmin) xmin = res[i].taille;
        if (res[i].taille > xmax) xmax = res[i].taille;
        if (res[i].temps > ymax) ymax = res[i].temps;
        if (res[i].temps < ymin) ymin = res[i].temps;
        present[res[i].algo] = 1;
    }
    if (ymin < PLANCHER) ymin = PLANCHER;

    x0 = conv(xmin, elog);
    x1 = conv(xmax, elog);
    if (x1 <= x0) x1 = x0 + 1;
    if (elog) {
        y0 = floor(log10(ymin));
        y1 = ceil(log10(ymax));
        if (y1 <= y0) y1 = y0 + 1;
    } else {
        y0 = 0;
        y1 = ymax;
    }

    /* passage valeur -> pixel */
#define PX(v) (g + w * (conv((v), elog) - x0) / (x1 - x0))
#define PY(t) (haut + h - h * (conv((t), elog) - y0) / (y1 - y0))

    /* lignes horizontales + graduations Y */
    cairo_set_line_width(cr, 1);
    if (elog) {
        for (k = (int)y0; k <= (int)y1; k++) {
            double val = pow(10, k), y = PY(val);
            cairo_set_source_rgb(cr, 0.92, 0.88, 0.80);
            cairo_move_to(cr, g, y); cairo_line_to(cr, g + w, y); cairo_stroke(cr);
            snprintf(buf, sizeof buf, "%g", val);
            cairo_set_source_rgb(cr, 0.4, 0.35, 0.25);
            ecrire(cr, buf, g - 8, y - 8, 9, 0, 2);
        }
    } else {
        for (i = 0; i <= 5; i++) {
            double val = y1 * i / 5.0, y = PY(val);
            cairo_set_source_rgb(cr, 0.92, 0.88, 0.80);
            cairo_move_to(cr, g, y); cairo_line_to(cr, g + w, y); cairo_stroke(cr);
            snprintf(buf, sizeof buf, "%.4g", val);
            cairo_set_source_rgb(cr, 0.4, 0.35, 0.25);
            ecrire(cr, buf, g - 8, y - 8, 9, 0, 2);
        }
    }

    /* graduations X */
    cairo_set_source_rgb(cr, 0.4, 0.35, 0.25);
    if (elog) {
        for (k = (int)floor(x0); k <= (int)ceil(x1); k++) {
            if (k < x0 - 1e-9 || k > x1 + 1e-9) continue;
            snprintf(buf, sizeof buf, "%g", pow(10, k));
            ecrire(cr, buf, PX(pow(10, k)), haut + h + 6, 9, 0, 1);
        }
    } else {
        for (i = 0; i <= 5; i++) {
            double val = xmin + (xmax - xmin) * i / 5.0;
            snprintf(buf, sizeof buf, "%.0f", val);
            ecrire(cr, buf, PX(val), haut + h + 6, 9, 0, 1);
        }
    }
    ecrire(cr, axe_x, g + w / 2, haut + h + 24, 10, 0, 1);

    /* courbes : une couleur par algorithme */
    cairo_set_line_width(cr, 2.2);
    for (a = 0; a < NB_TRIS; a++) {
        int premier = 1;
        if (!present[a]) continue;
        cairo_set_source_rgb(cr, COULEURS[a][0], COULEURS[a][1], COULEURS[a][2]);
        for (i = 0; i < nb; i++) {
            if (res[i].algo != a) continue;
            if (premier) cairo_move_to(cr, PX(res[i].taille), PY(res[i].temps));
            else         cairo_line_to(cr, PX(res[i].taille), PY(res[i].temps));
            premier = 0;
        }
        cairo_stroke(cr);
        for (i = 0; i < nb; i++) {              /* les points */
            if (res[i].algo != a) continue;
            cairo_arc(cr, PX(res[i].taille), PY(res[i].temps), 3.5, 0, 2 * M_PI);
            cairo_fill(cr);
        }
    }

    /* legende (3 colonnes) */
    k = 0;
    for (a = 0; a < NB_TRIS; a++) {
        double lx, ly;
        if (!present[a]) continue;
        lx = g + (k % 3) * (w / 3);
        ly = haut + h + 50 + (k / 3) * 20;
        cairo_set_source_rgb(cr, COULEURS[a][0], COULEURS[a][1], COULEURS[a][2]);
        cairo_set_line_width(cr, 3);
        cairo_move_to(cr, lx, ly + 8); cairo_line_to(cr, lx + 22, ly + 8); cairo_stroke(cr);
        ecrire(cr, noms[a], lx + 30, ly, 10, 0, 0);
        k++;
    }
#undef PX
#undef PY
}

int graphique_exporter_png(const char *fichier,
                           const Resultat *res, int nb, int elog,
                           const char *titre, const char *axe_x, const char *axe_y,
                           const char *const *noms)
{
    int ok;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1000, 600);
    cairo_t *cr = cairo_create(s);

    graphique_dessiner(cr, 1000, 600, res, nb, elog, titre, axe_x, axe_y, noms);
    cairo_destroy(cr);
    ok = (cairo_surface_write_to_png(s, fichier) == CAIRO_STATUS_SUCCESS);
    cairo_surface_destroy(s);
    return ok;
}
