### Bilan lecture de fichiers avec gcc sous Windows

readfile("content_and_name_ascii.txt");                    ==> OK
readfile("content_and_name_unicode_çابتث背备本鼻.txt");    ==> OK
readfile("content_ascii_and_name_unicode_éهوي调丁.txt");   ==> OK
readfile("content_unicode_and_name_ascii.txt");            ==> OK

==> Le problème se situe sur le nom unicode <==

Voir avec gcc sous linux
Voir avec MSVC sous Windows.
