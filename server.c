#include <string.h>
#include <stdio.h>
#include <errno.h> 
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>


#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

#define BLACK 0
#define WHITE 1
#define MAX_CLIENTS 10
#define MAX_GAMES 5
#define MAXDATASIZE 512



typedef struct Game Game;
typedef struct Client Client;

struct Game {
  int id;
  Client * p1;
  Client * p2;
  int grid[8][8];
  Game * next;
};

struct Client {
  int socket;
  char * alias;
  int busy;
  Client * next;
};
Client * client_list = NULL;
Game * game_list = NULL;

// Ajouter un client à la liste liée et le renvoyer
Client * add_client(int socket)
{
  Client * client = (Client *) malloc(sizeof(Client));
  Client * current = client_list;
  client->socket = socket;
  client->busy = 0;
  client->alias = (char *) malloc(sizeof(char) * 24);
  client->next = client_list;
  client_list = client;
  return client;
}

void remove_client(int socket)
{
  Client * prev = NULL;
  Client * current = client_list;

  while (current != NULL)
  {
    if (current->socket == socket)
    {
      if (prev == NULL)
      {
        client_list = current->next;
      }
      else
      {
        prev->next = current->next;
      }
      free(current->alias);
      free(current);
      return;
    }
    prev = current;
    current = current->next;
  }
}

Client * get_client_by_socket(int socket)
{
  Client * current = client_list;

  while (current != NULL)
  {
    if (current->socket == socket)
    {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void set_client_alias(Client * client, char * alias)
{
  sprintf(client->alias, "%s", alias);
}

Client * get_client_by_alias(char * alias)
{
  if (alias == NULL) return NULL;
  Client * current = client_list;

  while (current != NULL)
  {
    if (strcmp(current->alias, alias) == 0)
    {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

Game * get_game_from_id(int id)
{
  Game * current = game_list;
  while (current != NULL)
  {
    if (current->id == id) return current;
    current = current->next;
  }
  return NULL;
}

Game * create_game(Client * p1, Client * p2)
{
  // Amorcer le générateur de nombres aléatoires
  srand(time(0));

  // Générer un nombre aléatoire entre min et max
  int min = 1;
  int max = 65536;
  int random_number;
  do {
    random_number = rand() % (max - min + 1) + min;
  } while(get_game_from_id(random_number) != NULL);
  Game * game = (Game *) malloc(sizeof(Game));
  game->id = random_number;
  p1->busy = game->id;
  p2->busy = game->id;
  game->p1 = p1;
  game->p2 = p2;
  for (int row = 0; row < 8; row++)
    for (int col = 0; col < 8; col++)
      game->grid[row][col] = 0;

  game->grid[3][3] = (int) (game->p2)->socket;
  game->grid[4][4] = (int) (game->p2)->socket;
  game->grid[4][3] = (int) (game->p1)->socket;
  game->grid[3][4] = (int) (game->p1)->socket;

  game->next = game_list;
  game_list = game;

  return game;
}

void remove_game(int id)
{
  Game * prev = NULL;
  Game * current = game_list;

  while (current != NULL)
  {
    if (current->id == id)
    {
      if (prev == NULL)
      {
        game_list = current->next;
      }
      else
      {
        prev->next = current->next;
      }
      (current->p1)->busy = 0;
      (current->p2)->busy = 0;
      free(current);
      return;
    }
    prev = current;
    current = current->next;
  }
}

char input[MAXDATASIZE];
char output[MAXDATASIZE];
char * token;
char * delim = ":";
int clients[MAX_CLIENTS];
char * aliases[MAX_CLIENTS];
Game games[MAX_GAMES];

void parse_name(int socket, int i);
void parse_invite(Client * from);
void parse_new_game();
void parse_game_move();

void send_list();
int send_output(int socket);
int get_game_index(int id);
void print_game(Game * game);

void init_game_grid(Game * game);
int is_in_grid(int row, int col);
int valid_move(Game * game, int row, int col, int player);
void move(Game * game, int row, int col, int player, char * captured_pieces);
int can_play(Game * game, int player);
void send_result(Game * game);

int create_server(struct sockaddr_in server, int port){
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);

  int sS = socket(AF_INET, SOCK_STREAM, 0);

  if (sS == -1)
  {
    perror("La création du socket a échoué");
    exit(EXIT_FAILURE);
  }
  if(bind(sS, (struct sockaddr*)&server, sizeof(struct sockaddr)) < 0)
  {
    perror("Échec de la liaison");
    exit(EXIT_FAILURE);
  }
  if(listen(sS, 3) < 0)
  {
    perror("La connexion a échoué");
    exit(EXIT_FAILURE);
  }

  printf("%sServeur créé !\n%s", YEL, WHT);

  return sS;
}

int send_output(int socket)
{
  if (socket <= 0) return -1;
  int n = send(socket, output, sizeof(output), 0);
  printf("(%d) >> %ld bytes : %s\n", socket, strlen(output), output);
  memset(output, 0, sizeof(output));

  return n;
}

bool isNumeric(char * str) 
    {
        // boucle sur chaque caractère de la chaîne
        for(char* it = str; *it; ++it) {
            if(!isdigit(*it)) 
              return false;  
        }

      return true;
    } 

int main(int argc, char * argv[])
{
  if (argc < 2){
    printf("Erreur: Veuillez entrer un numéro de port !\n");
    return -1;
}
  
if (!isNumeric(argv[1])){
    printf("Erreur: Entrer un numéro de port valide !\n");
    return -1;
}
    
  int activity, val, sd, max_sd;
  fd_set readfds;

  int * size;
  int i = 0;
  int sockfd;
  int client;
  struct sockaddr_in server;
  int serverSize = sizeof(server);

  memset(&server, 0, sizeof(server));
  printf("Serveur démarrant sur le port %d...\n", atoi(argv[1]));
  sockfd = create_server(server, atoi(argv[1]));

  for (i = 0; i < MAX_GAMES; i++)
  {
    games[i].id = -1;
  }

  while(1)
  {
    // Initialiser l'ensemble des descripteurs de fichiers
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    // max socket est le premier de l'ensemble au début
    max_sd = sockfd;

    Client * current = client_list;

    // Ajouter les sockets clients à l'ensemble des descripteurs de fichiers
    while (current != NULL)
    {
      sd = current->socket;
      if (sd > 0) FD_SET(sd, &readfds);
      if (sd > max_sd) max_sd = sd;
      current = current->next;
    }

    // Surveillez les sockets pour la lisibilité
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    // Aucune activité
    if ((activity < 0) && (errno != EINTR))
    {
      printf("Select erreur\n");
      continue;
    }

    // socket serveur correctement ajouté à l'ensemble
    if (FD_ISSET(sockfd, &readfds))
    {
      if ((client = accept(sockfd, (struct sockaddr *) &server, (socklen_t *) &serverSize)) < 0)
      {
        printf("%s", RED);
        printf("Une erreur s'est produite lors de l'acceptation d'une connexion socket.\n");
        printf("%s", WHT);
        exit(EXIT_FAILURE);
      }

      Client * new_client = get_client_by_socket(client);

      if (new_client == NULL) new_client = add_client(client);

      printf("%s", YEL);
      printf("Connexion entrante :\n");
      printf("%s", WHT);
      printf("Socket fd : %d\n", client);
      printf("On : ");
      printf("%s", GRN);
      printf("%s/%d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
      printf("%s", WHT);
    }

    current = client_list;
    while (current != NULL)
    {
      sd = current->socket;

      if (FD_ISSET(sd, &readfds))
      {
        memset(input, 0, sizeof(input));
        if ((val = read(sd, input, MAXDATASIZE)) == 0)
        {
          getpeername(sd, (struct sockaddr *) &server, (socklen_t *) &serverSize);
          printf("Connexion sortante\nUser: %s\nSocked fd: %d\nOn: %s/%d\n",
            current->alias,
            sd,
            inet_ntoa(server.sin_addr),
            ntohs(server.sin_port)
          );

          FD_CLR(sd, &readfds);
          current = current->next;
          remove_client(sd);
          send_list();
        }
        else
        {
          printf("[%d bytes] From %d : %s\n", val, sd, input);

          token = strtok(input, ":");
          if (strcmp(token, "NAME") == 0)
          {
            token = strtok(NULL, "\0");
            set_client_alias(current, token);
            send_list();
          }
          else if (strcmp(token, "GAME") == 0)
          {
            token = strtok(NULL, ":");
            if (strcmp(token, "NEW") == 0) // nouvelle demande de jeu
            {
              parse_new_game();
            }
            if (strcmp(token, "INVITE") == 0) // nouvelle demande de jeu
            {
              parse_invite(current);
            }
            else if (token != NULL)
            {
              int id = (int) strtol(token, NULL, 10);
              token = strtok(NULL, ":");
              if (token != NULL && strcmp(token, "MOVE") == 0)
              {
                parse_game_move(id);
              }
            }
          }
          current = current->next;
        }
      }
      else 
      {
        current = current->next;
      }
    }
  }
  free(input);
  free(output);
  close(sockfd);

  return EXIT_SUCCESS;
}

void parse_invite(Client * from)
{
  // En cas de nouveau jeu
  char j1[24];
  char * j2;
  
  sprintf(j1, "%s", from->alias);
  j2 = strtok(NULL, "\0");

  // Aucun alias fourni
  if (j1 == NULL || j2 == NULL || strcmp(j1, j2) == 0) {
    return;
  }

  Client * client2 = get_client_by_alias(j2);

  // Aucun client trouvé avec les alias donnés
  if (from == NULL || client2 == NULL) {
    return;
  }

  if (from->busy || client2->busy) {
    return;
  }

  sprintf(output, "GAME:INVITE:%s", j1);
  send_output(client2->socket); 
}

void parse_new_game()
{
  // En cas de nouveau jeu
  char * j1;
  char * j2;
  
  j1 = strtok(NULL, ":"); 
  j2 = strtok(NULL, ":"); 

  // Aucun alias fourni
  if (j1 == NULL || j2 == NULL || strcmp(j1, j2) == 0) {
    return;
  }

  Client * client1 = get_client_by_alias(j1);
  Client * client2 = get_client_by_alias(j2);

  // Aucun client trouvé avec les alias donnés
  if (client1 == NULL || client2 == NULL) {
    return;
  }

  Game * current = game_list;
  while (current != NULL)
  {
    // un joueur est déjà dans une partie
    if (current->p1 == client1 || current->p2 == client2) {
      return;
    }
    current = current->next;
  }
  current = create_game(client1, client2);

  send_list();

  sprintf(output, "GAME:NEW:%d:%d:PLAY", current->id, BLACK);
  send_output((current->p1)->socket); 
  sprintf(output, "GAME:NEW:%d:%d:WAIT", current->id, WHITE);
  send_output((current->p2)->socket); 
}

void parse_game_move(int id)
{
  char * buffer = (char *) malloc(sizeof(char) * MAXDATASIZE);

  Game * game = get_game_from_id(id);
  if (game == NULL) return;

  int col, row;
  char * tk;
  int valid;
  int finished;
  int n;
  token = strtok(NULL, ":");
  int player = (int) strtol(token, NULL, 10);
  token = strtok(NULL, "\0");

  tk = strtok(token, "-");
  row = (int) strtol(tk, NULL, 10);
  tk = strtok(NULL, "\0");
  col = (int) strtol(tk, NULL, 10);

  valid = valid_move(game, row, col, player);
  if (valid)
  {
    memset(buffer, 0, sizeof(buffer));
    move(game, row, col, player, buffer);
    int p1_move = can_play(game, 0);
    int p2_move = can_play(game, 1);
    if (player == 0) // p1 vient de bouger
    {
      printf("Player 1 a bougé\n");
      if (p2_move) // p2 peut-il bouger ?
      {
        printf("Player 2 peut bouger\n");
        sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
        send_output((game->p1)->socket);
        sprintf(output, "GAME:MOVE:%d:%s:PLAY", player, buffer);
        send_output((game->p2)->socket);
      }
      else if (p1_move) // p1 peut-il encore bouger ?
      {
        printf("Player 2 ne peut pas bouger mais le joueur 1 peut. Ne rien changer au statut des joueurs\n");
        sprintf(output, "GAME:MOVE:%d:%s:PLAY", player, buffer);
        send_output((game->p1)->socket);
        sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
        send_output((game->p2)->socket);
      }
      else // personne ne peut bouger
      {
        sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
        send_output((game->p1)->socket);
        sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
        send_output((game->p2)->socket);
        printf("Neither can move. Endgame\n");
        send_result(game);
        remove_game(game->id);
        send_list();
      }
    }
    else if (player == 1) // p2 vient de bouger et p1 peut jouer
    {
      printf("Player 2 a récemment bougé\n");
      if (p1_move) // p1 peut-il bouger ?
      {
        printf("Player 1 can move\n");
        sprintf(output, "GAME:MOVE:%d:%s:PLAY", player, buffer);
        send_output((game->p1)->socket);
        sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
        send_output((game->p2)->socket);
      }
      else if (p2_move) // p2 peut-il encore bouger ?
      {
        printf("Player 1 ne peut pas bouger mais le joueur 2 peut. Ne rien changer aux statuts des joueurs\n");
         sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
         send_output((game->p1)->socket);
         sprintf(output, "GAME:MOVE:%d:%s:PLAY", player, buffer);
         send_output((game->p2)->socket);
      }
      else // personne ne peut bouger
      {
        sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
        send_output((game->p1)->socket);
        sprintf(output, "GAME:MOVE:%d:%s:WAIT", player, buffer);
        send_output((game->p2)->socket);
        printf("Ni l'un ni l'autre ne peut bouger. Fin du jeu\n");
        send_result(game);
        remove_game(game->id);
        send_list();
      }
    }
    memset(buffer, 0, sizeof(buffer));
  } else {
    printf("pas un mouvement valide\n");
  }
  free(buffer);
}

void send_list()
{
  char buffer[64];

  Client * current = client_list;

  while (current != NULL)
  {
    strcpy(output, "LIST\n");
    Client * user = client_list;
    while (user != NULL)
    {
      if (user != current && strlen(user->alias) > 0)
      {
        sprintf(buffer, "%s %s\n", user->alias, user->busy ? "(En partie)" : "");
        strcat(output, buffer);
      }
      user = user->next;
    }
    send_output(current->socket);
    current = current->next;
  }
}

int is_in_grid(int row, int col)
{
  return col >= 0 && col < 8 && row >= 0 && row < 8;
}

int valid_move(Game * game, int row, int col, int player)
{
  if (!is_in_grid(row, col) || game->grid[row][col]) return 0; // occupé ou en dehors des limites

  int color = player ? (game->p2)->socket : (game->p1)->socket;
  int opponent = player ? (game->p1)->socket : (game->p2)->socket;

  int r, c;

  int directions[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
  int has_opponent_pieces = 0;
  int found_color = 0;

  for (int i = 0; i < 8; i++)
  {
    has_opponent_pieces = 0;
    found_color = 0;
    r = row + directions[i][0];
    c = col + directions[i][1];
    while (r >= 0 && r < 8 && c >= 0 && c < 8)
    {
      if (game->grid[r][c] == opponent) // vérifier que la cellule suivante est occupée par l'adversaire
      {
        has_opponent_pieces = 1;
      } else if (game->grid[r][c] == color && has_opponent_pieces) {
        found_color = 1;
        break;
      } else break;

      r += directions[i][0]; 
      c += directions[i][1];
    }

    if (has_opponent_pieces && found_color) {
        printf("%d - %d est un mouvement valide pour %s\n", row, col, player ? "blanc" : "noir");
      return 1;
    }
  }

  return 0;
}

void move(Game * game, int row, int col, int player, char * captured_pieces)
{
  int color = !player ? (game->p1)->socket : (game->p2)->socket;
  int opponent = !player ? (game->p2)->socket : (game->p1)->socket;

  int r, c;

  int directions[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
  int has_opponent_pieces = 0;
  int found_color = 0;

  char piece[5];

  for (int i = 0; i < 8; i++)
  {
    has_opponent_pieces = 0;
    found_color = 0;
    r = row + directions[i][0];
    c = col + directions[i][1];
    while (r >= 0 && r < 8 && c >= 0 && c < 8)
    {
      if (game->grid[r][c] == opponent) // vérifier que la cellule suivante est occupée par l'adversaire
      {
        has_opponent_pieces = 1;
      } else if (game->grid[r][c] == color && has_opponent_pieces) {
        found_color = 1;
        break;
      } else break;

      r += directions[i][0];
      c += directions[i][1];
    }

    if (has_opponent_pieces && found_color)
    {
      // Capturez les pièces de l'adversaire dans cette direction
      r = row + directions[i][0];
      c = col + directions[i][1];
      while ((r != row || c != col) &&
             (r >= 0 && r < 8 && c >= 0 && c < 8) &&
             game->grid[r][c] != color)
      {
        snprintf(piece, sizeof(piece), "%d-%d", r, c);
        strcat(captured_pieces, piece);
        strcat(captured_pieces, ":");
        game->grid[r][c] = color;
        r += directions[i][0];
        c += directions[i][1];
      }
    }
  }

  snprintf(piece, sizeof(piece), "%d-%d", row, col);
  strcat(captured_pieces, piece);
  game->grid[row][col] = color;
}

int can_play(Game * game, int player)
{
  int moves = 0;
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      moves += (!game->grid[i][j] && valid_move(game, i, j, player));
  return !!moves;
}

void send_result(Game * game)
{
  int p1 = 0, p2 = 0;
  Client * winner;
  Client * loser;
  int draw = -1;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (game->grid[i][j] == (game->p1)->socket) p1++;
      else if (game->grid[i][j] == (game->p2)->socket) p2 ++;
    }
  }
  if (p1>p2)
  {
    winner = game->p1;
    loser = game->p2;
  }
  else if (p2>p1)
  {
    winner = game->p2;
    loser = game->p1;
  } else {
    draw = 1;
  }

  if (winner)
  {
    sprintf(output, "GAME:WON");
    send_output(winner->socket);
    memset(output, 0, sizeof(output));
  }
  sprintf(output, "GAME:LOST");
  if (draw == 1)
  {
    send_output((game->p1)->socket);
    send_output((game->p2)->socket);
    memset(output, 0, sizeof(output));
  }
  else if (draw == -1)
  {
    send_output(loser->socket);
  }
}

