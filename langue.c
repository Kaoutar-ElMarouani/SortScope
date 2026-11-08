#include "langue.h"

static Langue langue_courante = LANGUE_FR;

/* Tableau [texte][langue], construit automatiquement depuis textes.def */
static const char *textes[NB_TEXTES][NB_LANGUES] = {
#define X(id, fr, en, ar) { fr, en, ar },
#include "textes.def"
#undef X
};

void langue_definir(Langue l)
{
    if (l >= 0 && l < NB_LANGUES) langue_courante = l;
}

Langue langue_actuelle(void)
{
    return langue_courante;
}

const char *langue_texte(IdTexte id)
{
    return textes[id][langue_courante];
}
