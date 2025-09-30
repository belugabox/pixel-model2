# miniz vendorisation

Ce dépôt contient une copie vendorisée de miniz (version 3.1.0) utilisée pour l'extraction ZIP en-process sans dépendre d'outils externes.

## Emplacement

- **Implementation C**: `miniz-3.1.0/miniz.c`
- **Headers publics**: `miniz-3.1.0/miniz.h` (ajouté à l'include path via CMake)
- Le CMake ajoute une librairie statique `third_party_miniz` compilant `miniz.c`.

**Note**: Les fichiers dupliqués dans `include/` et `third_party/` ont été supprimés pour maintenir une seule source de vérité dans `miniz-3.1.0/`.

## Structure nettoyée

```
miniz-3.1.0/
├── miniz.c          # Implementation complète
├── miniz.h          # Header public
├── ChangeLog.md     # Historique des versions
├── LICENSE          # License (domaine public)
└── readme.md        # Documentation originale
```

Les exemples (`examples/`) ont été supprimés car ils ne sont pas nécessaires pour l'émulateur.

## Usage

- Le code utilise les APIs `mz_zip_reader_*` pour lire un fichier ZIP en mémoire et extraire ses entrées.
- Exemple: `extract_zip_to_memory(const std::string&, std::map<std::string, std::vector<uint8_t>>&)` dans `src/memory.cpp`.
- Include simple: `#include "miniz.h"` (le path est configuré dans CMakeLists.txt)

## Lancer le test ZIP

1. Générer et construire via CMake (depuis le dossier racine):
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

2. Exécuter le test léger qui n'ouvre pas de fenêtre et extrait `roms/vcop2.zip`:
```bash
# Linux/macOS
./ZipExtractTest

# Windows
.\Release\ZipExtractTest.exe
```

## Notes

- Le test vérifie quelques tailles attendues et affiche les premiers octets d'un fichier extrait.
- Toute la logique d'extraction ZIP utilise miniz en interne, sans dépendances externes.
- La bibliothèque est compilée en tant que librairie statique et liée à tous les exécutables qui en ont besoin.