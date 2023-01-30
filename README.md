# OTHELLO

Pour compiler et démarrer le serveur il faut ouvrir un terminal et insérer : 


gcc -Wall -o server server.c && ./server <num_port>


Ensuite pour compiler et démmarer le client il faut ouvrir plusieurs terminaux ( au moins deux pour jouer ) et insérer : 


gcc -Wall -o othello_GUI othello_GUI.c -lX11 $(pkg-config --cflags --libs gtk+-3.0) && ./othello_GUI


