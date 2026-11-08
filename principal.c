#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include "interface.h"
#include "langue.h"

int main(int argc, char *argv[])
{
    GtkWidget *fenetre;

    gtk_disable_setlocale();      /* garde le point decimal "." pour les nombres */
    gtk_init(&argc, &argv);
    srand((unsigned)time(NULL));  /* nombres aleatoires differents a chaque lancement */

    langue_definir(LANGUE_FR);
    fenetre = interface_creer();
    gtk_widget_show_all(fenetre);

    gtk_main();
    return 0;
}
