#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"
#include "tableau.h"
#include "tri.h"
#include "mesure.h"
#include "langue.h"
#include "graphique.h"

#define MAX_RES    100
#define NB_TAILLES 7
#define NB_REPS    4

static const int TAILLES[NB_TAILLES] = {100, 500, 1000, 5000, 10000, 50000, 100000};
static const int REPS[NB_REPS] = {1, 3, 5, 10};

#define AJOUTER(boite, w) gtk_box_pack_start(GTK_BOX(boite), (w), FALSE, FALSE, 0)

/* ================= DONNEES DU PROGRAMME ================= */
static double    *donnees = NULL;        /* le tableau genere */
static int        nb_donnees = 0;
static TypeValeur type_donnees = TYPE_INT;
static int        index_depart = 0;      /* premiere valeur affichee a droite */

static Resultat resultats[MAX_RES];      /* mesures de la comparaison */
static int      nb_resultats = 0;
static int      calcul_en_cours = 0;
static int      erreur_tri = 0;

/* ================= WIDGETS ================= */
static GtkWidget *fenetre, *lbl_titre, *lbl_sous_titre, *lbl_langue, *combo_langue;

/* panneau 1 : donnees */
static GtkWidget *lbl_p1, *lbl_type, *lbl_nombre, *lbl_ordre;
static GtkWidget *combo_type, *combo_nombre, *combo_ordre, *btn_generer;
static GtkWidget *lbl_manuel, *entry_manuel, *btn_manuel;
/* panneau 2 : algorithmes */
static GtkWidget *lbl_p2, *check_tris[NB_TRIS], *lbl_rep, *combo_rep;
static GtkWidget *btn_lancer, *progression, *lbl_statut;
/* panneau 3 : performances */
static GtkWidget *lbl_p3, *lbl_echelle, *combo_echelle, *btn_csv, *btn_img;
static GtkWidget *notebook, *tab_courbes, *tab_tableau, *tab_aide;
static GtkWidget *zone_dessin = NULL, *vue_resultats, *lbl_aide;
static GtkListStore *store_resultats;
static GtkTreeViewColumn *col_algo, *col_taille, *col_temps;
/* panneau 4 : valeurs */
static GtkWidget *lbl_p4, *lbl_index, *entry_index, *btn_afficher, *check_tries;
static GtkWidget *vue_valeurs, *btn_prec, *btn_suiv, *lbl_plage;
static GtkListStore *store_valeurs;
static GtkTreeViewColumn *col_indice, *col_valeur;

static void appliquer_langue(void);

/* ================= PETITES AIDES ================= */
static void ajouter_classe(GtkWidget *w, const char *classe)
{
    gtk_style_context_add_class(gtk_widget_get_style_context(w), classe);
}

static GtkWidget *nouveau_label(const char *classe)
{
    GtkWidget *l = gtk_label_new("");
    gtk_widget_set_halign(l, GTK_ALIGN_START);
    if (classe) ajouter_classe(l, classe);
    return l;
}

static GtkWidget *nouveau_bouton(const char *classe)
{
    GtkWidget *b = gtk_button_new();
    ajouter_classe(b, classe);
    return b;
}

static GtkWidget *nouvelle_carte(void)
{
    GtkWidget *c = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    ajouter_classe(c, "carte");
    return c;
}

static void statut(const char *message)
{
    gtk_label_set_text(GTK_LABEL(lbl_statut), message);
}

static void statut_format(const char *format, int a)
{
    char *m = g_strdup_printf(format, a);
    statut(m);
    g_free(m);
}

static int echelle_log(void)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(combo_echelle)) == 1;
}

/* ================= PANNEAU 4 : AFFICHER LES VALEURS ================= */
static void texte_valeur(double v, char *buf, size_t taille)
{
    if (type_donnees == TYPE_CHAR)     snprintf(buf, taille, "%c  (%d)", (int)v, (int)v);
    else if (type_donnees == TYPE_INT) snprintf(buf, taille, "%d", (int)v);
    else                               snprintf(buf, taille, "%g", v);
}

static void afficher_valeurs(void)
{
    int i, fin;
    char buf[32], *txt;
    GtkTreeIter it;
    double *source = donnees, *copie = NULL;

    gtk_list_store_clear(store_valeurs);
    if (donnees == NULL) {
        gtk_label_set_text(GTK_LABEL(lbl_plage), "");
        return;
    }
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_tries))) {
        copie = tableau_copier(donnees, nb_donnees);
        tri_executer(TRI_RAPIDE, copie, nb_donnees);
        source = copie;
    }

    fin = index_depart + 100;
    if (fin > nb_donnees) fin = nb_donnees;
    for (i = index_depart; i < fin; i++) {
        texte_valeur(source[i], buf, sizeof buf);
        gtk_list_store_append(store_valeurs, &it);
        gtk_list_store_set(store_valeurs, &it, 0, i, 1, buf, -1);
    }
    free(copie);

    txt = g_strdup_printf(TXT(T_PLAGE), index_depart, fin - 1, nb_donnees);
    gtk_label_set_text(GTK_LABEL(lbl_plage), txt);
    g_free(txt);
}

static void maj_champ_index(void)
{
    char buf[16];
    snprintf(buf, sizeof buf, "%d", index_depart);
    gtk_entry_set_text(GTK_ENTRY(entry_index), buf);
}

/* ================= BOUTONS DU PANNEAU 1 ================= */
static void on_generer(GtkWidget *b, gpointer d)
{
    int n = TAILLES[gtk_combo_box_get_active(GTK_COMBO_BOX(combo_nombre))];
    TypeValeur type = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_type));
    OrdreInitial ordre = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_ordre));

    free(donnees);
    donnees = tableau_generer(n, type, ordre);
    nb_donnees = (donnees != NULL) ? n : 0;
    type_donnees = type;
    index_depart = 0;
    maj_champ_index();
    afficher_valeurs();
    statut_format(TXT(T_MSG_GENERE), nb_donnees);
}

static void on_manuel(GtkWidget *b, gpointer d)
{
    double *tab = NULL;
    int n = 0;
    int code = tableau_lire_texte(gtk_entry_get_text(GTK_ENTRY(entry_manuel)), &tab, &n);

    if (code == 1) { statut(TXT(T_ERR_SAISIE)); return; }
    if (code == 2) { statut(TXT(T_ERR_NB)); return; }

    free(donnees);
    donnees = tab;
    nb_donnees = n;
    type_donnees = TYPE_DOUBLE;
    index_depart = 0;
    maj_champ_index();
    afficher_valeurs();
    statut_format(TXT(T_MSG_MANUEL), n);
}

/* ================= BOUTONS DU PANNEAU 4 ================= */
static void on_afficher(GtkWidget *b, gpointer d)
{
    const char *texte = gtk_entry_get_text(GTK_ENTRY(entry_index));
    char *fin;
    long v;

    if (donnees == NULL) { statut(TXT(T_ERR_PAS_DONNEES)); return; }
    v = strtol(texte, &fin, 10);
    if (fin == texte || *fin != '\0' || v < 0 || v >= nb_donnees) {
        statut(TXT(T_ERR_INDEX));
        return;
    }
    index_depart = (int)v;
    afficher_valeurs();
}

static void on_precedentes(GtkWidget *b, gpointer d)
{
    if (donnees == NULL) return;
    index_depart -= 100;
    if (index_depart < 0) index_depart = 0;
    maj_champ_index();
    afficher_valeurs();
}

static void on_suivantes(GtkWidget *b, gpointer d)
{
    if (donnees == NULL) return;
    if (index_depart + 100 < nb_donnees) index_depart += 100;
    maj_champ_index();
    afficher_valeurs();
}

static void on_check_tries(GtkWidget *b, gpointer d)
{
    afficher_valeurs();
}

/* ================= CALCUL EN ARRIERE-PLAN =================
   Le calcul tourne dans un thread pour que la fenetre ne se bloque pas.
   Le thread ne touche jamais aux widgets : il envoie ses resultats
   a la fenetre avec g_idle_add. */
typedef struct {
    int tris[NB_TRIS];
    int nb_tris;
    int repetitions;
    int taille_max;
    TypeValeur type;
    OrdreInitial ordre;
} Config;

typedef struct {
    Resultat res;
    int ok;
    int fait, total;
} Avancement;

static gboolean maj_avancement(gpointer p)
{
    Avancement *a = p;
    GtkTreeIter it;

    if (nb_resultats < MAX_RES) {
        resultats[nb_resultats++] = a->res;
        gtk_list_store_append(store_resultats, &it);
        gtk_list_store_set(store_resultats, &it, 0, a->res.algo, 1, a->res.taille,
                           2, a->res.temps, -1);
    }
    if (!a->ok) erreur_tri = 1;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progression),
                                  (double)a->fait / a->total);
    gtk_widget_queue_draw(zone_dessin);
    g_free(a);
    return G_SOURCE_REMOVE;
}

static gboolean fin_calcul(gpointer p)
{
    calcul_en_cours = 0;
    gtk_widget_set_sensitive(btn_lancer, TRUE);
    if (erreur_tri) statut(TXT(T_MSG_TRI_FAUX));
    else            statut_format(TXT(T_MSG_TERMINE), nb_resultats);
    return G_SOURCE_REMOVE;
}

static gpointer travail(gpointer arg)
{
    Config *c = arg;
    int s, k, fait = 0, nb_t = 0;

    for (s = 0; s < NB_TAILLES; s++)
        if (TAILLES[s] <= c->taille_max) nb_t++;

    for (s = 0; s < NB_TAILLES && TAILLES[s] <= c->taille_max; s++) {
        double *t = tableau_generer(TAILLES[s], c->type, c->ordre);
        if (t == NULL) break;
        for (k = 0; k < c->nb_tris; k++) {
            Avancement *a = g_new0(Avancement, 1);
            a->res.algo = c->tris[k];
            a->res.taille = TAILLES[s];
            a->res.temps = mesure_temps(c->tris[k], t, TAILLES[s], c->repetitions, &a->ok);
            a->fait = ++fait;
            a->total = nb_t * c->nb_tris;
            g_idle_add(maj_avancement, a);
        }
        free(t);
    }
    g_idle_add(fin_calcul, NULL);
    g_free(c);
    return NULL;
}

static void on_lancer(GtkWidget *b, gpointer d)
{
    int i;
    Config *c;

    if (calcul_en_cours) return;

    c = g_new0(Config, 1);
    for (i = 0; i < NB_TRIS; i++)
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_tris[i])))
            c->tris[c->nb_tris++] = i;
    if (c->nb_tris == 0) {
        g_free(c);
        statut(TXT(T_ERR_AUCUN_TRI));
        return;
    }
    c->repetitions = REPS[gtk_combo_box_get_active(GTK_COMBO_BOX(combo_rep))];
    c->taille_max = TAILLES[gtk_combo_box_get_active(GTK_COMBO_BOX(combo_nombre))];
    c->type = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_type));
    c->ordre = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_ordre));

    /* on efface les anciens resultats */
    nb_resultats = 0;
    erreur_tri = 0;
    gtk_list_store_clear(store_resultats);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progression), 0);
    gtk_widget_queue_draw(zone_dessin);

    calcul_en_cours = 1;
    gtk_widget_set_sensitive(btn_lancer, FALSE);
    statut(TXT(T_MSG_EN_COURS));
    g_thread_unref(g_thread_new("tri", travail, c));
}

/* ================= GRAPHIQUE ET EXPORTS ================= */
static void preparer_noms(const char *noms[NB_TRIS])
{
    int i;
    for (i = 0; i < NB_TRIS; i++) noms[i] = TXT(T_SELECTION + i);
}

static gboolean on_draw(GtkWidget *w, cairo_t *cr, gpointer d)
{
    const char *noms[NB_TRIS];
    preparer_noms(noms);
    graphique_dessiner(cr, gtk_widget_get_allocated_width(w),
                       gtk_widget_get_allocated_height(w),
                       resultats, nb_resultats, echelle_log(),
                       TXT(T_TITRE_GRAPH), TXT(T_AXE_X), TXT(T_AXE_Y), noms);
    return FALSE;
}

static void on_echelle(GtkWidget *w, gpointer d)
{
    if (zone_dessin != NULL) gtk_widget_queue_draw(zone_dessin);
}

/* Boite "Enregistrer sous" : renvoie le chemin choisi (a liberer avec g_free) */
static char *demander_fichier(const char *nom_defaut)
{
    char *chemin = NULL;
    GtkWidget *dialogue = gtk_file_chooser_dialog_new(
        TXT(T_BTN_ENREGISTRER), GTK_WINDOW(fenetre), GTK_FILE_CHOOSER_ACTION_SAVE,
        TXT(T_BTN_ANNULER), GTK_RESPONSE_CANCEL,
        TXT(T_BTN_ENREGISTRER), GTK_RESPONSE_ACCEPT, NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialogue), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialogue), nom_defaut);
    if (gtk_dialog_run(GTK_DIALOG(dialogue)) == GTK_RESPONSE_ACCEPT)
        chemin = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialogue));
    gtk_widget_destroy(dialogue);
    return chemin;
}

static void on_csv(GtkWidget *b, gpointer d)
{
    char *chemin;
    if (nb_resultats == 0) { statut(TXT(T_ERR_AUCUNE_MESURE)); return; }
    chemin = demander_fichier("resultats.csv");
    if (chemin == NULL) return;
    statut(mesure_exporter_csv(chemin, resultats, nb_resultats)
           ? TXT(T_MSG_EXPORT_OK) : TXT(T_ERR_EXPORT));
    g_free(chemin);
}

static void on_img(GtkWidget *b, gpointer d)
{
    const char *noms[NB_TRIS];
    char *chemin;
    if (nb_resultats == 0) { statut(TXT(T_ERR_AUCUNE_MESURE)); return; }
    chemin = demander_fichier("courbes.png");
    if (chemin == NULL) return;
    preparer_noms(noms);
    statut(graphique_exporter_png(chemin, resultats, nb_resultats, echelle_log(),
                                  TXT(T_TITRE_GRAPH), TXT(T_AXE_X), TXT(T_AXE_Y), noms)
           ? TXT(T_MSG_EXPORT_OK) : TXT(T_ERR_EXPORT));
    g_free(chemin);
}

/* ================= STYLE (theme.css) ================= */
static void charger_theme(void)
{
    const char *chemins[] = { "theme.css", "../theme.css", "../../theme.css" };
    int i;
    GtkCssProvider *fournisseur = gtk_css_provider_new();
    int trouve = 0;

    for (i = 0; i < 3 && !trouve; i++) {
        if (g_file_test(chemins[i], G_FILE_TEST_EXISTS)) {
            gtk_css_provider_load_from_path(fournisseur, chemins[i], NULL);
            gtk_style_context_add_provider_for_screen(
                gdk_screen_get_default(), GTK_STYLE_PROVIDER(fournisseur),
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            trouve = 1;
        }
    }
    if (!trouve) g_warning("theme.css introuvable (verifier le dossier d'execution)");
    g_object_unref(fournisseur);
}

/* ================= TABLEAU DES MESURES ================= */
static void rendu_algo(GtkTreeViewColumn *col, GtkCellRenderer *cell,
                       GtkTreeModel *m, GtkTreeIter *it, gpointer d)
{
    int a;
    gtk_tree_model_get(m, it, 0, &a, -1);
    g_object_set(cell, "text", TXT(T_SELECTION + a), NULL);
}

static void rendu_temps(GtkTreeViewColumn *col, GtkCellRenderer *cell,
                        GtkTreeModel *m, GtkTreeIter *it, gpointer d)
{
    double t;
    char buf[32];
    gtk_tree_model_get(m, it, 2, &t, -1);
    snprintf(buf, sizeof buf, "%.6f", t);
    g_object_set(cell, "text", buf, NULL);
}

/* ================= CHANGEMENT DE LANGUE ================= */
/* Remplit un combo avec n textes consecutifs a partir de premier_id */
static void remplir_combo(GtkWidget *combo, int premier_id, int n)
{
    int i, actif = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    if (actif < 0) actif = 0;
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));
    for (i = 0; i < n; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), TXT(premier_id + i));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), actif);
}

/* Arabe = ecriture de droite a gauche : on le fait pour tous les widgets */
static void definir_direction(GtkWidget *w, gpointer dir)
{
    gtk_widget_set_direction(w, (GtkTextDirection)GPOINTER_TO_INT(dir));
    if (GTK_IS_CONTAINER(w))
        gtk_container_forall(GTK_CONTAINER(w), definir_direction, dir);
}

static void appliquer_langue(void)
{
    int i;
    GtkTextDirection dir = (langue_actuelle() == LANGUE_AR) ? GTK_TEXT_DIR_RTL
                                                            : GTK_TEXT_DIR_LTR;

    gtk_window_set_title(GTK_WINDOW(fenetre), TXT(T_TITRE_APP));
    gtk_label_set_text(GTK_LABEL(lbl_titre), TXT(T_TITRE_APP));
    gtk_label_set_text(GTK_LABEL(lbl_sous_titre), TXT(T_SOUS_TITRE));
    gtk_label_set_text(GTK_LABEL(lbl_langue), TXT(T_LANGUE));

    /* panneau 1 */
    gtk_label_set_text(GTK_LABEL(lbl_p1), TXT(T_P1));
    gtk_label_set_text(GTK_LABEL(lbl_type), TXT(T_TYPE));
    gtk_label_set_text(GTK_LABEL(lbl_nombre), TXT(T_NOMBRE));
    gtk_label_set_text(GTK_LABEL(lbl_ordre), TXT(T_ORDRE));
    remplir_combo(combo_ordre, T_ORD_ALEA, 3);
    gtk_button_set_label(GTK_BUTTON(btn_generer), TXT(T_GENERER));
    gtk_label_set_text(GTK_LABEL(lbl_manuel), TXT(T_MANUEL));
    gtk_button_set_label(GTK_BUTTON(btn_manuel), TXT(T_UTILISER));

    /* panneau 2 */
    gtk_label_set_text(GTK_LABEL(lbl_p2), TXT(T_P2));
    for (i = 0; i < NB_TRIS; i++)
        gtk_button_set_label(GTK_BUTTON(check_tris[i]), TXT(T_SELECTION + i));
    gtk_label_set_text(GTK_LABEL(lbl_rep), TXT(T_REPETITIONS));
    gtk_button_set_label(GTK_BUTTON(btn_lancer), TXT(T_LANCER));
    if (!calcul_en_cours) statut(TXT(T_PRET));

    /* panneau 3 */
    gtk_label_set_text(GTK_LABEL(lbl_p3), TXT(T_P3));
    gtk_label_set_text(GTK_LABEL(lbl_echelle), TXT(T_ECHELLE));
    remplir_combo(combo_echelle, T_LINEAIRE, 2);
    gtk_button_set_label(GTK_BUTTON(btn_csv), TXT(T_EXPORT_CSV));
    gtk_button_set_label(GTK_BUTTON(btn_img), TXT(T_EXPORT_IMG));
    gtk_label_set_text(GTK_LABEL(tab_courbes), TXT(T_ONGLET_COURBES));
    gtk_label_set_text(GTK_LABEL(tab_tableau), TXT(T_ONGLET_TABLEAU));
    gtk_label_set_text(GTK_LABEL(tab_aide), TXT(T_ONGLET_AIDE));
    gtk_label_set_text(GTK_LABEL(lbl_aide), TXT(T_AIDE));
    gtk_tree_view_column_set_title(col_algo, TXT(T_COL_ALGO));
    gtk_tree_view_column_set_title(col_taille, TXT(T_COL_TAILLE));
    gtk_tree_view_column_set_title(col_temps, TXT(T_COL_TEMPS));
    gtk_widget_queue_draw(vue_resultats);
    gtk_widget_queue_draw(zone_dessin);

    /* panneau 4 */
    gtk_label_set_text(GTK_LABEL(lbl_p4), TXT(T_P4));
    gtk_label_set_text(GTK_LABEL(lbl_index), TXT(T_INDEX));
    gtk_button_set_label(GTK_BUTTON(btn_afficher), TXT(T_AFFICHER));
    gtk_button_set_label(GTK_BUTTON(check_tries), TXT(T_VOIR_TRIES));
    gtk_tree_view_column_set_title(col_indice, TXT(T_COL_INDICE));
    gtk_tree_view_column_set_title(col_valeur, TXT(T_COL_VALEUR));
    gtk_button_set_label(GTK_BUTTON(btn_prec), TXT(T_PRECEDENTES));
    gtk_button_set_label(GTK_BUTTON(btn_suiv), TXT(T_SUIVANTES));
    afficher_valeurs();

    gtk_widget_set_default_direction(dir);
    definir_direction(fenetre, GINT_TO_POINTER(dir));
}

static void on_langue(GtkComboBox *combo, gpointer d)
{
    langue_definir(gtk_combo_box_get_active(combo));
    appliquer_langue();
}

/* ================= CONSTRUCTION DE LA FENETRE ================= */
GtkWidget *interface_creer(void)
{
    int i;
    char buf[16];
    GtkWidget *racine, *entete, *bloc_titre, *corps, *gauche, *centre, *droite;
    GtkWidget *grille, *ligne, *sw;
    GtkCellRenderer *r;

    charger_theme();

    fenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(fenetre), 1280, 780);
    g_signal_connect(fenetre, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    racine = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(fenetre), racine);

    /* ---------- en-tete : titre + choix de la langue ---------- */
    entete = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    ajouter_classe(entete, "entete");
    bloc_titre = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    lbl_titre = nouveau_label("titre-app");
    lbl_sous_titre = nouveau_label("sous-titre");
    AJOUTER(bloc_titre, lbl_titre);
    AJOUTER(bloc_titre, lbl_sous_titre);
    gtk_box_pack_start(GTK_BOX(entete), bloc_titre, TRUE, TRUE, 0);

    lbl_langue = nouveau_label("sous-titre");
    gtk_widget_set_valign(lbl_langue, GTK_ALIGN_CENTER);
    combo_langue = gtk_combo_box_text_new();
    gtk_widget_set_valign(combo_langue, GTK_ALIGN_CENTER);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_langue), "Français");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_langue), "English");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_langue), "العربية");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_langue), 0);
    AJOUTER(entete, lbl_langue);
    AJOUTER(entete, combo_langue);
    AJOUTER(racine, entete);

    /* ---------- corps : 3 panneaux dans la meme fenetre ---------- */
    corps = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
    gtk_container_set_border_width(GTK_CONTAINER(corps), 14);
    gtk_box_pack_start(GTK_BOX(racine), corps, TRUE, TRUE, 0);

    /* ===== panneau de gauche : donnees + algorithmes ===== */
    gauche = nouvelle_carte();
    gtk_widget_set_size_request(gauche, 320, -1);

    lbl_p1 = nouveau_label("titre-section");
    AJOUTER(gauche, lbl_p1);

    lbl_type = nouveau_label(NULL);
    combo_type = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "int");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "float");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "char");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "double");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), 0);
    AJOUTER(gauche, lbl_type);
    AJOUTER(gauche, combo_type);

    lbl_nombre = nouveau_label(NULL);
    combo_nombre = gtk_combo_box_text_new();
    for (i = 0; i < NB_TAILLES; i++) {
        snprintf(buf, sizeof buf, "%d", TAILLES[i]);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_nombre), buf);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_nombre), 3);   /* 5000 */
    AJOUTER(gauche, lbl_nombre);
    AJOUTER(gauche, combo_nombre);

    lbl_ordre = nouveau_label(NULL);
    combo_ordre = gtk_combo_box_text_new();   /* rempli dans appliquer_langue */
    AJOUTER(gauche, lbl_ordre);
    AJOUTER(gauche, combo_ordre);

    btn_generer = nouveau_bouton("secondaire");
    g_signal_connect(btn_generer, "clicked", G_CALLBACK(on_generer), NULL);
    AJOUTER(gauche, btn_generer);

    lbl_manuel = nouveau_label(NULL);
    AJOUTER(gauche, lbl_manuel);
    ligne = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    entry_manuel = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_manuel), "5 3 8 1 9 2 7 4 6 10");
    btn_manuel = nouveau_bouton("discret");
    g_signal_connect(btn_manuel, "clicked", G_CALLBACK(on_manuel), NULL);
    gtk_box_pack_start(GTK_BOX(ligne), entry_manuel, TRUE, TRUE, 0);
    AJOUTER(ligne, btn_manuel);
    AJOUTER(gauche, ligne);

    lbl_p2 = nouveau_label("titre-section");
    gtk_widget_set_margin_top(lbl_p2, 10);
    AJOUTER(gauche, lbl_p2);

    grille = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grille), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grille), 6);
    for (i = 0; i < NB_TRIS; i++) {
        check_tris[i] = gtk_check_button_new();
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_tris[i]), TRUE);
        gtk_grid_attach(GTK_GRID(grille), check_tris[i], i % 2, i / 2, 1, 1);
    }
    AJOUTER(gauche, grille);

    ligne = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    lbl_rep = nouveau_label(NULL);
    combo_rep = gtk_combo_box_text_new();
    for (i = 0; i < NB_REPS; i++) {
        snprintf(buf, sizeof buf, "%d", REPS[i]);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_rep), buf);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_rep), 1);      /* 3 repetitions */
    AJOUTER(ligne, lbl_rep);
    AJOUTER(ligne, combo_rep);
    AJOUTER(gauche, ligne);

    btn_lancer = nouveau_bouton("principal");
    g_signal_connect(btn_lancer, "clicked", G_CALLBACK(on_lancer), NULL);
    AJOUTER(gauche, btn_lancer);

    progression = gtk_progress_bar_new();
    AJOUTER(gauche, progression);
    lbl_statut = nouveau_label("statut");
    gtk_label_set_line_wrap(GTK_LABEL(lbl_statut), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(lbl_statut), 38);
    AJOUTER(gauche, lbl_statut);

    /* ===== panneau du centre : courbes / tableau / aide ===== */
    centre = nouvelle_carte();
    gtk_widget_set_hexpand(centre, TRUE);

    lbl_p3 = nouveau_label("titre-section");
    AJOUTER(centre, lbl_p3);

    ligne = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    lbl_echelle = nouveau_label(NULL);
    combo_echelle = gtk_combo_box_text_new();   /* rempli dans appliquer_langue */
    btn_csv = nouveau_bouton("discret");
    btn_img = nouveau_bouton("discret");
    g_signal_connect(btn_csv, "clicked", G_CALLBACK(on_csv), NULL);
    g_signal_connect(btn_img, "clicked", G_CALLBACK(on_img), NULL);
    AJOUTER(ligne, lbl_echelle);
    AJOUTER(ligne, combo_echelle);
    gtk_box_pack_start(GTK_BOX(ligne), gtk_label_new(""), TRUE, TRUE, 0);
    AJOUTER(ligne, btn_csv);
    AJOUTER(ligne, btn_img);
    AJOUTER(centre, ligne);

    notebook = gtk_notebook_new();

    /* onglet 1 : le graphique */
    zone_dessin = gtk_drawing_area_new();
    gtk_widget_set_size_request(zone_dessin, 420, 340);
    gtk_widget_set_hexpand(zone_dessin, TRUE);
    gtk_widget_set_vexpand(zone_dessin, TRUE);
    g_signal_connect(zone_dessin, "draw", G_CALLBACK(on_draw), NULL);
    tab_courbes = gtk_label_new("");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), zone_dessin, tab_courbes);

    /* onglet 2 : le tableau des mesures */
    store_resultats = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_INT, G_TYPE_DOUBLE);
    vue_resultats = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_resultats));

    col_algo = gtk_tree_view_column_new();
    r = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col_algo, r, TRUE);
    gtk_tree_view_column_set_cell_data_func(col_algo, r, rendu_algo, NULL, NULL);
    gtk_tree_view_column_set_expand(col_algo, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(vue_resultats), col_algo);

    col_taille = gtk_tree_view_column_new_with_attributes("", gtk_cell_renderer_text_new(),
                                                          "text", 1, NULL);
    gtk_tree_view_column_set_expand(col_taille, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(vue_resultats), col_taille);

    col_temps = gtk_tree_view_column_new();
    r = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col_temps, r, TRUE);
    gtk_tree_view_column_set_cell_data_func(col_temps, r, rendu_temps, NULL, NULL);
    gtk_tree_view_column_set_expand(col_temps, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(vue_resultats), col_temps);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(sw), vue_resultats);
    tab_tableau = gtk_label_new("");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sw, tab_tableau);

    /* onglet 3 : l'aide */
    lbl_aide = nouveau_label("aide");
    gtk_label_set_line_wrap(GTK_LABEL(lbl_aide), TRUE);
    gtk_widget_set_valign(lbl_aide, GTK_ALIGN_START);
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(sw), lbl_aide);
    tab_aide = gtk_label_new("");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sw, tab_aide);

    gtk_box_pack_start(GTK_BOX(centre), notebook, TRUE, TRUE, 0);

    /* ===== panneau de droite : explorer les valeurs ===== */
    droite = nouvelle_carte();
    gtk_widget_set_size_request(droite, 300, -1);

    lbl_p4 = nouveau_label("titre-section");
    AJOUTER(droite, lbl_p4);
    lbl_index = nouveau_label(NULL);
    AJOUTER(droite, lbl_index);

    ligne = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    entry_index = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_index), "0");
    btn_afficher = nouveau_bouton("discret");
    g_signal_connect(btn_afficher, "clicked", G_CALLBACK(on_afficher), NULL);
    gtk_box_pack_start(GTK_BOX(ligne), entry_index, TRUE, TRUE, 0);
    AJOUTER(ligne, btn_afficher);
    AJOUTER(droite, ligne);

    check_tries = gtk_check_button_new();
    g_signal_connect(check_tries, "toggled", G_CALLBACK(on_check_tries), NULL);
    AJOUTER(droite, check_tries);

    store_valeurs = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
    vue_valeurs = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_valeurs));
    col_indice = gtk_tree_view_column_new_with_attributes("", gtk_cell_renderer_text_new(),
                                                          "text", 0, NULL);
    col_valeur = gtk_tree_view_column_new_with_attributes("", gtk_cell_renderer_text_new(),
                                                          "text", 1, NULL);
    gtk_tree_view_column_set_expand(col_valeur, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(vue_valeurs), col_indice);
    gtk_tree_view_append_column(GTK_TREE_VIEW(vue_valeurs), col_valeur);
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(sw), vue_valeurs);
    gtk_box_pack_start(GTK_BOX(droite), sw, TRUE, TRUE, 0);

    ligne = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_set_homogeneous(GTK_BOX(ligne), TRUE);
    btn_prec = nouveau_bouton("discret");
    btn_suiv = nouveau_bouton("discret");
    g_signal_connect(btn_prec, "clicked", G_CALLBACK(on_precedentes), NULL);
    g_signal_connect(btn_suiv, "clicked", G_CALLBACK(on_suivantes), NULL);
    gtk_box_pack_start(GTK_BOX(ligne), btn_prec, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(ligne), btn_suiv, TRUE, TRUE, 0);
    AJOUTER(droite, ligne);

    lbl_plage = nouveau_label("statut");
    AJOUTER(droite, lbl_plage);

    /* ---------- on met les 3 panneaux dans le corps ---------- */
    AJOUTER(corps, gauche);
    gtk_box_pack_start(GTK_BOX(corps), centre, TRUE, TRUE, 0);
    AJOUTER(corps, droite);

    /* ---------- signaux restants et textes ---------- */
    g_signal_connect(combo_echelle, "changed", G_CALLBACK(on_echelle), NULL);
    g_signal_connect(combo_langue, "changed", G_CALLBACK(on_langue), NULL);
    appliquer_langue();

    return fenetre;
}
