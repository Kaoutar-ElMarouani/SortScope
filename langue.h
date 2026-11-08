#ifndef LANGUE_H
#define LANGUE_H

typedef enum { LANGUE_FR, LANGUE_EN, LANGUE_AR, NB_LANGUES } Langue;

/* La liste des identifiants vient du fichier textes.def */
typedef enum {
#define X(id, fr, en, ar) id,
#include "textes.def"
#undef X
    NB_TEXTES
} IdTexte;

void   langue_definir(Langue l);
Langue langue_actuelle(void);
const char *langue_texte(IdTexte id);

#define TXT(id) langue_texte(id)

#endif
