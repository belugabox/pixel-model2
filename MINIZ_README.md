miniz vendorisation

Ce dépôt contient une copie vendorisée de miniz (version 3.1.0) utilisée pour l'extraction ZIP en-process sans dépendre d'outils externes.

Emplacement
- Implementation C: `miniz-3.1.0/miniz.c`
- Includes publics: `miniz-3.1.0/` (ajoutés à l'include path via CMake)
- Le CMake ajoute une librairie statique `third_party_miniz` compilant `miniz.c`.

Usage
- Le code utilise les APIs `mz_zip_reader_*` pour lire un fichier ZIP en mémoire et extraire ses entrées.
- Exemple: `extract_zip_to_memory(const std::string&, std::map<std::string, std::vector<uint8_t>>&)` dans `src/memory.cpp`.

Lancer le test ZIP
1. Générer et construire en Debug via CMake (depuis le dossier `build`):
```powershell
cmake ..
cmake --build . --config Debug
```
2. Exécuter le test léger qui n'ouvre pas de fenêtre et extrait `roms/vcop2.zip` :
```powershell
cd build\Debug
.\ZipExtractTest.exe
```

Notes
- Le test vérifie quelques tailles attendues et affiche les premiers octets d'un fichier extrait.
- Si vous préférez désactiver les fallbacks externes, supprimez les portions de code qui invoquent des outils extérieurs (la logique principale utilise désormais miniz).