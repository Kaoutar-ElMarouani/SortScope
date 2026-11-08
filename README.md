# SortScope

Comparaison experimentale des algorithmes de tri (selection, insertion, bulles,
fusion, Shell, rapide) avec une interface graphique GTK3 en une seule fenetre.
TP1 - Structures de Donnees Avancee - FST Casablanca.

## Fonctionnalites
- Generation de tableaux (int, float, char, double) : aleatoire, deja trie, decroissant
- Saisie manuelle de petits tableaux (10 a 30 valeurs)
- Mesure du temps moyen de chaque tri (sur plusieurs repetitions)
- Courbes (echelle lineaire ou logarithmique), tableau des mesures, aide
- Export CSV et image PNG
- Interface en francais, anglais et arabe

## Organisation du code
| Fichier | Role |
|---|---|
| `tableau.c/.h` | generation, copie et lecture des tableaux |
| `tri.c/.h` | les 6 algorithmes de tri |
| `mesure.c/.h` | chronometre et export CSV |
| `langue.c/.h`, `textes.def` | les 3 langues |
| `graphique.c/.h` | dessin des courbes (cairo) |
| `interface.c/.h` | la fenetre GTK |
| `principal.c` | point d'entree |
| `theme.css` | couleurs et style |

## Compilation (Windows, MSYS2 MinGW64)
```
gcc *.c -o sortscope.exe `pkg-config --cflags --libs gtk+-3.0` -lm
./sortscope.exe
```
Lancer le programme depuis le dossier du projet (pour trouver `theme.css`).
