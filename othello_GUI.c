#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include <signal.h>
#include <sys/signalfd.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <X11/Xlib.h>


/* Variables globales 
  int damier[8][8];	// tableau associe au damier
  int couleur;		// 0 : pour noir, 1 : pour blanc
  
  int port;		// numero port passé lors de l'appel

  char *addr_j2, *port_j2;	// Info sur adversaire


  pthread_t thr_id;	// Id du thread fils gerant connexion socket
  
  int sockfd, newsockfd=-1; // descripteurs de socket
  int addr_size;	 // taille adresse
  struct sockaddr *their_addr;	// structure pour stocker adresse adversaire

  fd_set master, read_fds, write_fds;	// ensembles de socket pour toutes les sockets actives avec select
  int fdmax;			// utilise pour select


/* Variables globales associées à l'interface graphique 
  GtkBuilder  *  p_builder   = NULL;
  GError      *  p_err       = NULL;
   
*/

// Entetes des fonctions  
  
/* Fonction permettant de changer l'image d'une case du damier (indiqué par sa colonne et sa ligne) 
void change_img_case(int col, int lig, int couleur_j);

/* Fonction permettant changer nom joueur blanc dans cadre Score 
void set_label_J1(char *texte);

/* Fonction permettant de changer nom joueur noir dans cadre Score 
void set_label_J2(char *texte);

/* Fonction permettant de changer score joueur blanc dans cadre Score 
void set_score_J1(int score);

/* Fonction permettant de récupérer score joueur blanc dans cadre Score 
int get_score_J1(void);

/* Fonction permettant de changer score joueur noir dans cadre Score 
void set_score_J2(int score);

/* Fonction permettant de récupérer score joueur noir dans cadre Score 
int get_score_J2(void);

/* Fonction transformant coordonnees du damier graphique en indexes pour matrice du damier 
void coord_to_indexes(const gchar *coord, int *col, int *lig);

/* Fonction appelee lors du clique sur une case du damier 
static void coup_joueur(GtkWidget *p_case);

/* Fonction retournant texte du champs adresse du serveur de l'interface graphique 
char *lecture_addr_serveur(void);

/* Fonction retournant texte du champs port du serveur de l'interface graphique 
char *lecture_port_serveur(void);

/* Fonction retournant texte du champs login de l'interface graphique 
char *lecture_login(void);

/* Fonction retournant texte du champs adresse du cadre Joueurs de l'interface graphique 
char *lecture_addr_adversaire(void);

/* Fonction retournant texte du champs port du cadre Joueurs de l'interface graphique 
char *lecture_port_adversaire(void);

/* Fonction affichant boite de dialogue si partie gagnee 
void affiche_fenetre_gagne(void);

/* Fonction affichant boite de dialogue si partie perdue 
void affiche_fenetre_perdu(void);

/* Fonction appelee lors du clique du bouton Se connecter 
static void clique_connect_serveur(GtkWidget *b);

/* Fonction desactivant bouton demarrer partie 
void disable_button_start(void);

/* Fonction appelee lors du clique du bouton Demarrer partie 
static void clique_connect_adversaire(GtkWidget *b);

/* Fonction desactivant les cases du damier 
void gele_damier(void);

/* Fonction activant les cases du damier 
void degele_damier(void);

/* Fonction permettant d'initialiser le plateau de jeu 
void init_interface_jeu(void);

/* Fonction reinitialisant la liste des joueurs sur l'interface graphique 
void reset_liste_joueurs(void);

/* Fonction permettant d'ajouter un joueur dans la liste des joueurs sur l'interface graphique 
void affich_joueur(char *login, char *adresse, char *port);



/* Fonction transforme coordonnees du damier graphique en indexes pour matrice du damier 
void coord_to_indexes(const gchar *coord, int *col, int *lig)
{
  char *c;
  
  c=malloc(3*sizeof(char));
  
  c=strncpy(c, coord, 1);
  c[1]='\0';
  
  if(strcmp(c, "A")==0)
  {
    *col=0;
  }
  if(strcmp(c, "B")==0)
  {
    *col=1;
  }
  if(strcmp(c, "C")==0)
  {
    *col=2;
  }
  if(strcmp(c, "D")==0)
  {
    *col=3;
  }
  if(strcmp(c, "E")==0)
  {
    *col=4;
  }
  if(strcmp(c, "F")==0)
  {
    *col=5;
  }
  if(strcmp(c, "G")==0)
  {
    *col=6;
  }
  if(strcmp(c, "H")==0)
  {
    *col=7;
  }
    
  *lig=atoi(coord+1)-1;
}

/* Fonction transforme coordonnees du damier graphique en indexes pour matrice du damier 
void indexes_to_coord(int col, int lig, char *coord)
{
  char c;

  if(col==0)
  {
    c='A';
  }
  if(col==1)
  {
    c='B';
  }
  if(col==2)
  {
    c='C';
  }
  if(col==3)
  {
    c='D';
  }
  if(col==4)
  {
    c='E';
  }
  if(col==5)
  {
    c='F';
  }
  if(col==6)
  {
    c='G';
  }
  if(col==7)
  {
    c='H';
  }
    
  sprintf(coord, "%c%d\0", c, lig+1);
}
*/

gboolean desactiver_commandes_jeu(gpointer data);
gboolean activer_commandes_jeu(gpointer data);
gboolean afficher_joueur_tampon(gpointer data);
gboolean reset_liste_joueurs(gpointer data);
gboolean affiche_fenetre_fin(gpointer data);
gboolean update_label_blanc(gpointer data);
gboolean update_label_noir(gpointer data);
gboolean update_score_blanc(gpointer data);
gboolean update_score_noir(gpointer data);
gboolean reset_interface(gpointer data);
gboolean prompt_invite(gpointer data);
gboolean update_titre(gpointer data);
gboolean count_score(gpointer data);
gboolean update_move(gpointer data);
gboolean init_game(gpointer data);

/* FONCTIONS DE RAPPEL STATIQUES DÉCLENCHÉES PAR L'UTILISATEUR */

static void player_move(GtkWidget *p_case);
static void server_connect(GtkWidget *b);
static void start_game(GtkWidget *b);

/* OUTILS UTILISE */

#define MAXDATASIZE 512

void coord_to_indexes(const gchar *coord, int *col, int *row);
void indexes_to_coord(int col, int row, char * coord);
void change_img_case(int col, int row, int color);
void disable_server_connect(void);
void signup(char * login);

char *get_server_address(void);
char *get_server_port(void);
char *get_target(void);
char *get_login(void);

/* STRUCTURES */

typedef struct State State;
typedef struct Move Move;
typedef struct G_Data G_Data;
typedef struct PromptData PromptData;

struct State {
  int * sockfd;
  int * play;
};

struct Move {
  int col;
  int row;
  int player;
};

struct G_Data {
  GtkBuilder * p_builder;
  char data[24];
};

struct PromptData {
  char from[32];
  char to[32];
  int socket_fd;
};

/* VARIABLES GLOBALES */

struct sockaddr_in server;
int serverSize = sizeof(server);
int sockfd;
int damier[8][8];
int couleur;
int game_id = -1;
char * login;
char * target_name;
State * state;
pthread_t read_thread;

// GTK variables
GtkBuilder  *  p_builder   = NULL;
GError      *  p_err       = NULL;

// Thread de lecture
void * t_read(void * state)
{
  // Nous aurons besoin du contexte de la boucle principale pour les mises à jour thread-safe de notre interface
  GMainContext *main_context = g_main_context_default();

  State * st = (State *) state;
  char buffer[MAXDATASIZE]; 
  ssize_t size; 

  char * token;

  // Titre de fenêtre
  char * wait = g_strdup("C'est le tour de ton adversaire");
  char * play = g_strdup("A ton tour de jouer");
  char * won_message = g_strdup("Fin de la partie.\n\nVous avez gagné!");
  char * lost_message = g_strdup("Fin de la partie.\n\nVous avez perdu!");

  while(1)
  {
    // Effacer le tampon pour la prochaine entrée
    memset(buffer, 0, MAXDATASIZE);

    // lecture continue depuis le serveur
    size = read(*(st->sockfd), buffer, MAXDATASIZE);
    if (size == 0)
    {
      printf("\nLost connection to server...\n");
      exit(EXIT_FAILURE);
    }
    else if (size >= 1)
    {
      fflush(stdout);
      printf("\n<< [%ld bytes] %s\n", size, buffer);

      token = strtok(buffer, "\n");

      if (strcmp(token, "LIST") == 0)
      {
        g_main_context_invoke(main_context, (GSourceFunc) reset_liste_joueurs, p_builder);
        do
        {
          token = strtok(NULL, "\n");
          if (token != NULL)
          {
            G_Data * data = (G_Data *) malloc(sizeof(G_Data));
            data->p_builder = p_builder;
            sprintf(data->data, "%s", token);
            g_main_context_invoke(main_context, (GSourceFunc) afficher_joueur_tampon, data);
          }
        } while (token != NULL);
      }
      else // Si pas de nouvelle liste
      {
        token = strtok(buffer, ":");
        if (token != NULL)
        {
          // S'assurer que nous avons reçu une entrée GAME
          if (strcmp(token, "GAME") == 0)
          {
            // Sous-commande de l'entrée de jeu
            token = strtok(NULL, ":");

            // La commande est une nouvelle demande de jeu
            if (token != NULL && strcmp(token, "NEW") == 0)
            {
              // Désactivez en toute sécurité le bouton game_start dans le fil principal
              g_main_context_invoke(main_context, (GSourceFunc) desactiver_commandes_jeu, p_builder);

              // Game ID à renvoyer au serveur
              token = strtok(NULL, ":");
              if (token != NULL)
              {
                game_id = strtol(token, NULL, 10);

                // Position du joueur (0 pour les noirs, 1 pour les blancs)
                token = strtok(NULL, ":");
                couleur = (int) strtol(token, NULL, 10);

                // Initialiser en toute sécurité l'interface du jeu dans le thread principal
                g_main_context_invoke(main_context, (GSourceFunc) init_game, damier);

                // Statut du jeu pour le joueur
                token = strtok(NULL, "\0");
                if (token != NULL && strcmp(token, "WAIT") == 0)
                {
                  // Mettez à jour en toute sécurité le titre de la fenêtre avec le tour du joueur dans le fil principal
                  g_main_context_invoke(main_context, (GSourceFunc) update_titre, wait);
                  *(st->play) = 0;
                }
                else if (token != NULL && strcmp(token, "PLAY") == 0)
                {
                  // Mettez à jour en toute sécurité le titre de la fenêtre avec le tour du joueur dans le fil principal
                  g_main_context_invoke(main_context, (GSourceFunc) update_titre, play);
                  *(st->play) = 1;
                }
              }
            } else if (strcmp(token, "INVITE") == 0)
            {
              token = strtok(NULL, "\0");
              PromptData prompt;
              sprintf(prompt.from, "%s", token);
              sprintf(prompt.to, "%s", login);
              prompt.socket_fd = sockfd;
              g_main_context_invoke(main_context, (GSourceFunc) prompt_invite, &prompt);
            }
            else if (token != NULL && strcmp(token, "MOVE") == 0)
            {
              // Couleur du coup (0 pour le noir et 1 pour le blanc)
              int move = (int) strtol(strtok(NULL, ":"), NULL, 10);
              // Nous parcourons les jetons jusqu'à la fin de la chaîne
              while((token = strtok(NULL, ":\0")) != NULL)
              {
                // Si le jeton n'est pas un statut de jeu (WAIT / PLAY)
                if (strcmp(token, "PLAY") && strcmp(token, "WAIT") && strcmp(token, "NULL"))
                {
                  char *ptr = token;
                  // obtenir la position de la ligne
                  char * r = strsep(&ptr, "-");
                  // obtenir la position de la colonne
                  char * c = strsep(&ptr, "\0");

                  int col, row;
                  // convertir r et c en nombres entiers
                  row = (int) strtol(r, NULL, 10);
                  col = (int) strtol(c, NULL, 10);

                  // mettre à jour les données de la grille avec la couleur du mouvement à la position donnée
                  damier[row][col] = move;

                  // Stocke les données du mouvement
                  Move * m = (Move *) malloc(sizeof(Move));
                  m->col = col;
                  m->row = row;
                  m->player = move;
                  // Mettez à jour en toute sécurité l'interface du jeu avec la date du déplacement dans le fil principal
                  g_main_context_invoke(main_context, (GSourceFunc) update_move, m);

                  // Mettre à jour en toute sécurité le score du joueur dans le fil principal
                  g_main_context_invoke(main_context, (GSourceFunc) count_score, damier);
                }
                // On arrive à la fin de la liste des coordonnées avec un statut de jeu
                else if (strcmp(token, "WAIT") == 0)
                {
                  // Mettre à jour en toute sécurité le titre de la fenêtre dans le fil principal
                  g_main_context_invoke(main_context, (GSourceFunc) update_titre, wait);
                  *(st->play) = 0;
                }
                else if (strcmp(token, "PLAY") == 0)
                {
                  // Mettre à jour en toute sécurité le titre de la fenêtre dans le fil principal
                  g_main_context_invoke(main_context, (GSourceFunc) update_titre, play);
                  *(st->play) = 1;
                }
              }
            }
            // Le jeu se termine par le message gagnant
            else if (token != NULL && strcmp(token, "WON") == 0)
            {
              game_id = -1;
              *(st->play) = 0;

              g_main_context_invoke(main_context, (GSourceFunc) affiche_fenetre_fin, won_message);
              g_main_context_invoke(main_context, (GSourceFunc) activer_commandes_jeu, p_builder);
            }
            // Le jeu se termine avec le message perdant
            else if (token != NULL && strcmp(token, "LOST") == 0)
            {
              game_id = -1;
              *(st->play) = 0;

              g_main_context_invoke(main_context, (GSourceFunc) affiche_fenetre_fin, lost_message);
              g_main_context_invoke(main_context, (GSourceFunc) activer_commandes_jeu, p_builder);
            }
          }
       }
      }
    }
    else if (size < 0)
    {
      perror("An error occured while trying to read from the server\n");
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
}

gboolean prompt_invite(gpointer data)
{
  PromptData *promptData = (PromptData *) data;
  char title[64];
  sprintf(title, "%s veut jouer avec toi !", promptData->from);

  // Créer une boîte de dialogue avec des boutons oui/non
  GtkWidget *dialog = gtk_message_dialog_new(
      NULL,
      GTK_DIALOG_MODAL,
      GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_YES_NO,
      title);

  // Afficher la boîte de dialogue et attendre une réponse
  int response = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  char message[128];
  // Envoyer la réponse au serveur
  if (response == GTK_RESPONSE_YES) {
    sprintf(message, "GAME:NEW:%s:%s", promptData->from, promptData->to);
    send(promptData->socket_fd, message, sizeof(message), 0);
  } else {
    sprintf(message, "DECLINE");
    send(promptData->socket_fd, message, sizeof(message), 0);
  }
  memset(message, 0, sizeof(message));
  return FALSE;
}

gboolean update_titre(gpointer data)
{
  GtkWidget * p_win = (GtkWidget *) gtk_builder_get_object (p_builder, "window1");
  char * text = (char *) data;
  gtk_window_set_title((GtkWindow *) p_win, text);
  return FALSE;
}

gboolean update_move(gpointer data)
{
  Move * m = (Move *) data;
  int col, row, player;
  col = m->col;
  row = m->row;
  player = m->player;

  char * coord;

  coord=malloc(3*sizeof(char));

  indexes_to_coord(col, row, coord);

  if(player == 1)
  { // image pion blanc
    gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_blanc.png");
  }
  else if(player == 0)
  { // image pion noir
    gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_noir.png");
  }
  else if(player == -1)
  {
    gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_def.png");
  }
  free(m);
  return FALSE;
}

gboolean update_label_blanc(gpointer data)
{
  char * text = (char *) data;
  gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J1")), text);
  return FALSE;
}

gboolean update_label_noir(gpointer data)
{
  char * text = (char *) data;
  gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J2")), text);
  return FALSE;
}


gboolean update_score_blanc(gpointer data)
{
  int * score = (int *) data;
  char *s;
  
  s=malloc(5*sizeof(char));
  sprintf(s, "%d", *score);
  gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ1")), s);
  return FALSE;
}

gboolean update_score_noir(gpointer data)
{
  int * score = (int *) data;
  char *s;
  
  s=malloc(5*sizeof(char));
  sprintf(s, "%d", *score);
  gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ2")), s);
  return FALSE;
}

/* Fonction transforme coordonnees du damier graphique en indexes pour matrice du damier */
void coord_to_indexes(const gchar *coord, int *col, int *row)
{
  char *c;
  
  c=malloc(3*sizeof(char));
  
  c=strncpy(c, coord, 1);
  c[1]='\0';

  *col = ((int) c[0] - 65);
  
  *row=atoi(coord+1)-1;
}

/* Fonction transforme coordonnees du damier graphique en indexes pour matrice du damier */
void indexes_to_coord(int col, int row, char *coord)
{
  char c;
  c=(char) ('A' + col % 26);
  sprintf(coord, "%c%d", c, row+1);
}

/* Fonction permettant de changer l'image d'une case du damier (indiqué par sa colonne et sa ligne) */
void change_img_case(int col, int row, int color)
{
  char * coord;
  coord=malloc(3*sizeof(char));
  indexes_to_coord(col, row, coord);
  if(color == 1)
  { // image pion blanc
    gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_blanc.png");
  }
  else if(color == 0)
  { // image pion noir
    gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_noir.png");
  }
  else if (color == 2)
  {
    gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_rouge.png");
  }
  else gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_def.png");
}

/* Fonction retournant texte du champs adresse du serveur de l'interface graphique */
char *get_server_address(void)
{
  GtkWidget *entry_addr_srv;
  
  entry_addr_srv = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_adr");
  
  return (char *)gtk_entry_get_text(GTK_ENTRY(entry_addr_srv));
}

/* Fonction retournant texte du champs port du serveur de l'interface graphique */
char *get_server_port(void)
{
  GtkWidget *entry_port_srv;
  
  entry_port_srv = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_port");
  
  return (char *)gtk_entry_get_text(GTK_ENTRY(entry_port_srv));
}

/* Fonction retournant texte du champs login de l'interface graphique */
char *get_login(void)
{
  GtkWidget *entry_login;
  
  entry_login = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_login");
  
  return (char *)gtk_entry_get_text(GTK_ENTRY(entry_login));
}

/* Fonction retournant texte du champs adresse du cadre Joueurs de l'interface graphique */
char *get_target(void)
{
  GtkWidget * entry_target_name;
  
  entry_target_name = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_target_name");
  
  return (char *)gtk_entry_get_text(GTK_ENTRY(entry_target_name));
}

/* Fonction affichant boite de dialogue si partie gagnee */
gboolean affiche_fenetre_fin(gpointer data)
{
  char * message = (char *) data;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWidget * dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", message);
  gtk_dialog_run(GTK_DIALOG (dialog));
  
  gtk_widget_destroy(dialog);
  return FALSE;
}

gboolean reset_interface(gpointer data)
{
  GtkBuilder * p_builder = (GtkBuilder *) data;
  char * white = g_strdup("Joueur 1");
  char * black = g_strdup("Joueur 2");
  char * title = g_strdup("Projet Othello");
  update_label_noir(black);
  update_label_blanc(white);
  update_titre(title);
  return FALSE;
}

void signup(char * login)
{
  int n;
  char message[5 + strlen(login)];
  sprintf(message, "NAME:%s", login);
  n = send(sockfd, message, strlen(message), 0);
  printf("\n>> [%d bytes] : %s\n", n, message);
}

/*
/* Fonction desactivant les cases du damier
void gele_damier(void)
{
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH1"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH2"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH3"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH4"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH5"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH6"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH7"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA8"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB8"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC8"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD8"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE8"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF8"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG8"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH8"), FALSE);
}

/* Fonction activant les cases du damier
void degele_damier(void)
{
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH1"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH2"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH3"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH4"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH5"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH6"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH7"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA8"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB8"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC8"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD8"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE8"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF8"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG8"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH8"), TRUE);
}*/

void disable_server_connect(void)
{
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_connect"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), TRUE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "entry_login"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "entry_port"), FALSE);
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "entry_adr"), FALSE);
}

gboolean desactiver_commandes_jeu(gpointer data)
{
  GtkBuilder * p_builder = (GtkBuilder *) data;
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), FALSE);
  return FALSE;
}

gboolean activer_commandes_jeu(gpointer data)
{
  GtkBuilder * p_builder = (GtkBuilder *) data;
  gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), TRUE);
  return FALSE;
}

/* Fonction permettant d'initialiser le plateau de jeu */
gboolean init_game(gpointer data)
{
  int (*damier)[8] = (int(*)[8])data;
  char * text_you = g_strdup("Toi");
  char * text_opponent = g_strdup("Adversaire");
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++) {
      damier[i][j] = -1;
      change_img_case(i, j, -1);
    }

  // Initilisation du damier (D4=blanc, E4=noir, D5=noir, E5=blanc)
  change_img_case(3, 3, 1);
  change_img_case(4, 3, 0);
  change_img_case(3, 4, 0);
  change_img_case(4, 4, 1);

  damier[3][3] = 1;
  damier[4][3] = 0;
  damier[3][4] = 0;
  damier[4][4] = 1;
  
  // Initialisation des scores et des joueurs
  if(couleur == 1)
  {
    update_label_blanc(text_you);
    update_label_noir(text_opponent);
  }
  else if(couleur == 0)
  {
    update_label_noir(text_you);
    update_label_blanc(text_opponent);
  }
  int white = 2;
  int black = 2;
  update_score_blanc(&white);
  update_score_noir(&black);

  return FALSE;
}

/* Fonction reinitialisant la liste des joueurs sur l'interface graphique */
gboolean reset_liste_joueurs(gpointer data)
{
  GtkBuilder * p_builder = (GtkBuilder *) data;
  GtkTextIter start, end;
  
  gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &start);
  gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &end);
  
  gtk_text_buffer_delete(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &start, &end);
  return FALSE;
}

/* Fonction permettant d'ajouter un joueur dans la liste des joueurs sur l'interface graphique */
gboolean afficher_joueur_tampon(gpointer data)
{
  G_Data * arg = (G_Data *) data;
  char * login = arg->data;
  GtkBuilder * p_builder = arg->p_builder;
  const gchar *joueur;
  
  joueur=g_strconcat(login, "\n", NULL);
  gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), joueur, strlen(joueur));
  free(data);
  return FALSE;
}

gboolean count_score(gpointer data)
{
  int (*damier)[8] = (int(*)[8])data;
  int nb_p1 = 0;
  int nb_p2 = 0;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (damier[i][j] == 1) nb_p1++;
      else if (damier[i][j] == 0) nb_p2++;
    }
  }

  update_score_blanc(&nb_p1);
  update_score_noir(&nb_p2);
  return FALSE;
}

static void player_move(GtkWidget *p_case)
{
  if (*(state->play) == 0) return;

  int col, row, n;
  char msg[MAXDATASIZE] = "";

  // Traduction coordonnees damier en indexes matrice damier
  coord_to_indexes(gtk_buildable_get_name(GTK_BUILDABLE(gtk_bin_get_child(GTK_BIN(p_case)))), &col, &row);

  sprintf(msg, "GAME:%d:MOVE:%d:%d-%d", game_id, couleur, row, col);

  n = send(*(state->sockfd), msg, strlen(msg), 0);
  printf(">> [%d bytes] %s\n", n, msg);
}

static void server_connect(GtkWidget *b)
{
  login = get_login();

  if (strlen(login) < 1) return;

  //printf("Connection button triggered\n");

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("An error occured while trying to create a socket : ");
    return;
  }

  memset(&server, 0, sizeof(server));

  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(get_server_port()));
  server.sin_addr.s_addr = inet_addr(get_server_address());


  if (connect(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0) {
    return;
  }

  disable_server_connect();

  (*state).sockfd = &sockfd;
  (*state).play = (int *) malloc(sizeof(int));
  *(*state).play = -1;

  pthread_create(&read_thread, NULL, t_read, (void *) state);

  signup(login);

  return;
}

static void start_game(GtkWidget *b)
{
  printf("start\n");
  char message[32];
  int n;
  if(game_id < 0)
  {
    // Recuperation  adresse et port adversaire au format chaines caracteres
    target_name=get_target();
  
    sprintf(message, "GAME:INVITE:%s", target_name);
    n = send(sockfd, message, sizeof(message), 0);
    printf(">> [%d bytes] : %s\n", n, message);
    memset(message, 0, sizeof(message));
  }
}

int main (int argc, char ** argv)
{
/*  int i, j, ret;

   if(argc!=2)
   {
     printf("\nPrototype : ./othello num_port\n\n");
     
     exit(1);
   }
   */
  state = (State *) malloc(sizeof(State));   
  XInitThreads();
   
  /* Initialisation de GTK+ */
  gtk_init (& argc, & argv);
   
  /* Creation d'un nouveau GtkBuilder */
  p_builder = gtk_builder_new();
 
  if (p_builder != NULL)
  {
    /* Chargement du XML dans p_builder */
    gtk_builder_add_from_file (p_builder, "UI_Glade/Othello.glade", & p_err);
 
    if (p_err == NULL)
    {
      /* Recuparation d'un pointeur sur la fenetre. */
      GtkWidget * p_win = (GtkWidget *) gtk_builder_get_object (p_builder, "window1");

      /* Gestion evenement clic pour chacune des cases du damier */

      char id[11];
      for (int i = 1; i <= 8; i++)
      {
        for (int j = 0; j < 8; j++)
        {
          sprintf(id, "eventbox%c%d", (char) (65 + j), i);
          g_signal_connect(gtk_builder_get_object(p_builder, id), "button_press_event", G_CALLBACK(player_move), NULL);
        }
      }
      gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), FALSE);

      /* Gestion evenement clic pour chacune des cases du damier */
/*       g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
         g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
*/

      /* Gestion clic boutons interface */
      g_signal_connect(gtk_builder_get_object(p_builder, "button_connect"), "clicked", G_CALLBACK(server_connect), NULL);
      g_signal_connect(gtk_builder_get_object(p_builder, "button_start"), "clicked", G_CALLBACK(start_game), NULL);

      /* Gestion clic bouton fermeture fenetre */
      g_signal_connect_swapped(G_OBJECT(p_win), "destroy", G_CALLBACK(gtk_main_quit), NULL);
      gtk_widget_show_all(p_win);
      gtk_main();
    }
    else
    {
      /* Affichage du message d'erreur de GTK+ */
      g_error ("%s", p_err->message);
      g_error_free (p_err);
    }
  }

  pthread_join(read_thread, NULL);
  gtk_main_quit();
 
  return EXIT_SUCCESS;
}
