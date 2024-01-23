#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>

    // POURQUOI ON A PAS UTILISÉ LA FONCTION " WAIT (NULL)" ? 
        /* eh bah parce que ;
        1. on a eu des erreurs de blockage du l'appel systém WAIT quand on décode des grands fichiers (grandes images ...)
        2. notre hypothése : c'est surement à cause du buffer du pipe qui arrive pas à faire passé beaucoup d'informations  .
        3. dans la documentation qu'on va passé en commentaire juste aprés ; contient des informations qui remettent en question l'hypothèse initiale qu'on avait formulée.
        Si un processus tente de lire depuis un pipe vide, read(2) bloquera jusqu'à ce que des données soient disponibles.
        Si un processus tente d'écrire dans un pipe plein, write(2) bloque jusqu'à ce que suffisamment de données aient été lues du pipe pour permettre à l'écriture 
        de se terminer. ( le prof nous a dit que : Les primitives Read et le Write sont « bloquantes ».)  ce qui affirme fortement ce commentaire */

    // lien : https://man7.org/linux/man-pages/man7/pipe.7.html#:~:text=If%20a%20process%20attempts%20to%20read%20from%20an%20empty%20pipe%2C%20then%20read(2)%0A%20%20%20%20%20%20%20will%20block%20until%20data%20is%20available

#define BIG_BUFF 10  // pour gérer l'entrée standard ( impossible d'appliquer fseek et ftell sur l'entrée standrad donc on va transformé l'entrée standard en un fichier régulier).
#define BUFSIZE 8 // POUR RÉCUPERER 8 OCTETS À CHAQUE FOIS DANS NOTRE BUFFER ;

// macros d'erreurs . question : on peut considérer que ça c'est du bon code ou c'est inutile ?  
#define erreur_du_fork 73 
#define erreur_d_ouverture_du_fichier_a_decoder 74
#define erreur_execl 75
#define erreur_dup2 76

// fonction qui va transformer les octets du décodage qui sont IMPRIMABLE à leur forme d'origine binaire .
void imprimable_en_bin(unsigned char *caractere_imprimable)
{
    if (*caractere_imprimable >= 97 && *caractere_imprimable <= 122)
    {
        *caractere_imprimable = *caractere_imprimable - 'a';
    }
    if (*caractere_imprimable >= 48 && *caractere_imprimable <= 53)
    {
        *caractere_imprimable = *caractere_imprimable - 22;
    }
}
// fonction génerique , qui va transformer les octets de gauche en leur forme originelle (ABCDE12345fi)
void transformer(unsigned char octet_X, unsigned char octet_Y, unsigned char *l_octet_original, int position_de_debut_octet_original, int position_de_fin_octet_original, int starting_pos_octetX, int starting_pos_octetY)
{
    unsigned char bit_value = 0;
    for (int i = position_de_debut_octet_original; i >= position_de_fin_octet_original; i--)
    {
        if (i % 2 != 0)
        {
            // valeur du bit récuperer à la position " position du bit à loctet Y " .
            bit_value = 0;
            bit_value = ((1u << starting_pos_octetX) & octet_X) >> starting_pos_octetX;
            *l_octet_original |= (bit_value << i);
            starting_pos_octetX--;
        }
        else
        {
            // valeur du bit récuperer à la position " position du bit à loctet Y " .
            bit_value = 0;
            bit_value = ((1u << starting_pos_octetY) & octet_Y) >> starting_pos_octetY;
            *l_octet_original |= (bit_value << i);
            starting_pos_octetY--;
        }
    }
}
int main(int argc , char *argv[])
{
    int t[2];
    pipe(t);
    FILE *fichier_a_decoder;
    FILE *sortie_decodage;
    if (argc == 1)
    {
        fichier_a_decoder = stdin ;
        sortie_decodage = stdout ;
    }
    else if(argc == 2)
    {
        if((strcmp(argv[1],"-uu")==0))

        /* sur la ligne de cmd :                on doit re-créer ça :          % uudecode -o /dev/stdout /dev/stdin | ./my_sae_decodage 
          ./my_sae_encodage -uu << tag                                         ` contenue du fichier original `
          begin 664 sortie_file_blah_blah               ==>
          toto toto toto 
          titi code ascii ....
          =======
          tag

        */
        {
            int id = fork();
            if (id < 0) return erreur_du_fork ;
            if (id == 0) // PROCESSUS FILS
            {
            close(t[0]);
            if(dup2(t[1],STDOUT_FILENO)==-1){ // redirection de la sortie standard vers le pipe | ...
                perror("erreur de dup2 \n");
                return erreur_dup2;
            } 
            // tout affichage sur l'écran aprés la ligne précedente va etre redirigé vers le descripteur t[1].
            close(t[1]);    
            execl("/usr/bin/uudecode","uudecode","-o","/dev/stdout","/dev/stdin",NULL);
            perror("erreur du execl !\n");
            return erreur_execl;
            }
            else
            {
                close(t[1]);
                if ( dup2(t[0],STDIN_FILENO) == -1) // redirection de l'entrée standard .
                {
                    perror("erreur de dup2");
                    return erreur_dup2;
                } 
                close(t[0]);
                fichier_a_decoder = stdin ;
                sortie_decodage = stdout;
            }

        }
        else
        {
            fichier_a_decoder = fopen (argv[1],"r");
            if (fichier_a_decoder == NULL)
            {
                printf("impossible d'ouvrir argv[1] :%s",argv[1]);
                return erreur_d_ouverture_du_fichier_a_decoder;
            }
            sortie_decodage = stdout;
        }
    }else if(argc == 3)
    {
        if((strcmp(argv[1],"-uu") == 0)) // argv[2] doit absolument etre le fichier qui vient de l'encodage :./my_sae_encodage -uu exo.txt "sortie_sae_et_uuencode.uu"

        /* dans la ligne de cmd :                                      recréer ça :
            ./my_sae_decodage -uu "sortie_sae_uuencode.uu"         ==>              % uudecode -o /dev/stdout "sortie_sae_uuencode.uu" | ./my_sae_decoding 
                                                                        sur stdout : `contenu du fichier original : "exo.txt" `
        */
        {
            int id = fork();
            if(id < 0) return erreur_du_fork ;
            if (id == 0) // fils
            {
                close(t[0]); // pas besoin de lire 
                if (dup2(t[1],STDOUT_FILENO)== -1) // rediriger la sortie standard vers le pipe | .....
                {
                    perror("erreur de dup2 \n");
                    return erreur_dup2;
                }
                close(t[1]); // comme on a dupliquer le descripteur (ex: 5) ; sur le descripteur 0 ; on a plus besoin du 5.

                execl("/usr/bin/uudecode","uudecode","-o","/dev/stdout",argv[2],NULL);
                perror("erreur du execl !");
                return erreur_execl;
            }
            else // pére
            {
                close(t[1]); // ce descripteur il faut OBLIGATOIREMENT LE FERMER . si non le programme ne se termine jamais 
                // redirigé l'entrée standard .
                if (dup2(t[0], STDIN_FILENO) == -1) {
                    perror("erreur de dup2\n");
                    return erreur_dup2;
                }
                close(t[0]);

                fichier_a_decoder = stdin ; // qui est en vrai le contenue de t[0].
                sortie_decodage = stdout ;
            }
        }
        else
        {
            fichier_a_decoder = fopen (argv[1],"rb");
            sortie_decodage = fopen (argv[2],"w");
            if (fichier_a_decoder == NULL || sortie_decodage == NULL)
            {
                printf("impossible d'ouvrir les fichiers de cette ligne de commande \n");
                return erreur_d_ouverture_du_fichier_a_decoder;
            }
        }
    }else if( argc == 4)
    {
        if((strcmp(argv[1],"-uu"))==0)// argv[2] doit absolument etre le fichier qui vient de l'encodage :./my_sae_encodage -uu exo.txt "sortie_sae_et_uuencode.uu"

        /* dans la ligne de cmd :                                             on doit  recréer ça :
            ./my_sae_decodage -uu "sortie_sae_uuencode.uu" sortie_decodage           ==>              % uudecode -o /dev/stdout "sortie_sae_uuencode.uu" | ./my_sae_decoding  > sortie_decodage
                                                                                          sur stdout :  RIEN DU TOUT .
        */ 
        {
            int id = fork();
            if(id < 0) return erreur_du_fork ;
            if(id == 0) // fils
            {
                close(t[0]); // PAS BESION DONC ON LE FERME .
                
                if ( dup2(t[1],STDOUT_FILENO)== -1){
                    perror("errur de dup2 \n");
                    return erreur_dup2;
                }
                close(t[1]); // comme on l'a dupliqué ; on a interet à le fermé ici /

                execl("/usr/bin/uudecode","uudecode","-o","/dev/stdout",argv[2],NULL);
                perror("ERREUR DU EXECL \n");
                return erreur_execl;
            }
            else // pére
            {
                close(t[1]);
                if (dup2(t[0],STDIN_FILENO)==-1) // redirection de l'entrée standard ;
                {
                    perror("erreur de dup2");
                    return erreur_dup2;
                }
                close(t[0]);
                int fichier_de_redirection_stdout = open(argv[3],O_CREAT | O_WRONLY , 0777);
                if(fichier_de_redirection_stdout < 0) 
                {   
                    printf(" impossible de créer le fichier argv[3]");
                    return 37 ;
                }
                
                if(dup2(fichier_de_redirection_stdout,STDOUT_FILENO)==-1)//redirigée la sortie standrad .
                {
                     perror("erreur de dup2");
                    return erreur_dup2;
                }
                close(fichier_de_redirection_stdout);
                fichier_a_decoder = stdin ;
                sortie_decodage = stdout ;
                // le pére continue ça vie à décoder 
            }
        }    
        else
        {
            printf(" trop d'argument !\n");
            return 99999;
        }
    }
    /* si on encode l'entrée standard ou si on passe par l'option -uu :*/
    long position_fin; // position fin du fichier .
    if(fichier_a_decoder == stdin) {
        // on va compter le nombre d'octet sur l'entrée standard  ; et on va créer un nouveau fichier pour traiter l'entrée standrad comme un fichier comme tous les autres (on pourra aprés faire avec "ftell").
        // on utilise cette méthode car ce n'est pas possible de faire avec FSEEK ET FTELL ET REWIND ; 
        int i =0;
        int size_now = BIG_BUFF;
        char c;
        char *simuler_fichier = calloc(BIG_BUFF,sizeof(char));
        while((c=getchar())!=EOF)
        {
                if (c != '\n')
                {
                    simuler_fichier[i]=c;
                    i++;
                }
                if(i == size_now - 1)
                {
                    simuler_fichier[i]='\0';
                    size_now = strlen(simuler_fichier) + size_now + 1;
                    simuler_fichier = realloc(simuler_fichier, size_now);
                }
        }
        simuler_fichier[i]='\0';
        position_fin = strlen(simuler_fichier);
        fichier_a_decoder = fopen ("new_file","w");
        if (fichier_a_decoder == NULL)
        {
            printf(" Une erreur est survenue lors de la création du fichier  : new_file (fichier simulateur de l'entrée standard ) . \n");
            return erreur_d_ouverture_du_fichier_a_decoder;
        }
        
        int nbr_octets_ecrit = fwrite(simuler_fichier,sizeof(unsigned char),strlen(simuler_fichier), fichier_a_decoder);
        if (nbr_octets_ecrit != strlen(simuler_fichier))
        {
            printf(" PROBLEME LORS DE L'ECRITURE DANS LE FICHIER A DECODER . ( ERREUR LECTURE SUR L'ENTREE STANDARD ) ./");
            return 66;
        }
        fclose(fichier_a_decoder); // il faut fermer le fichier ( ouvert en écriture) ; pour ensuite le ré ouvrir ( en lecture) .
        fichier_a_decoder = fopen("new_file","r");
        if(fichier_a_decoder == NULL)
        {
            printf("erreur d'ouverture en lecture du fichier : new_file\n");
            return erreur_d_ouverture_du_fichier_a_decoder;
        }
        free(simuler_fichier); // libération de la mémoire 
    }
    else
    {
        // enregistrer la position à la fin du fichier .
        fseek(fichier_a_decoder, 0, SEEK_END);
        position_fin = ftell(fichier_a_decoder);
        rewind(fichier_a_decoder);
    } 
           
    while (1)
    {
        // buffer qui récupère 8 octet à chaque fois .
        unsigned char buffer[BUFSIZE] = {0};
        // avec cette variable on va reconstruire les octets d'origines .
        unsigned char original_byte[5] = {0};
        int nbr_octets_lu = fread(buffer, sizeof(unsigned char), 8, fichier_a_decoder);
        if (nbr_octets_lu == 0 && feof(fichier_a_decoder))
        {
            break;
        }

        // on sait trés bien que si on passe pas ce test ; notre encodage est faux !
        if (nbr_octets_lu != 8)
        {
            printf(" erreur de lecture dans le fichier A decoder ; ( verifier bien l'encodage alors)!");
            break;
        }

        // il faut transformer les caracteres imprimables en MODE BINAIRE ( leurs ORIGINE ) .
        for (int i = 0; i <= 7; i++)
        {
            imprimable_en_bin(&buffer[i]);
        }

        // reconstruction des octets d'origine.

        // octet numero 0
        transformer(buffer[0], buffer[1], &original_byte[0], 7, 0, 4, 4);
        // octet numero 1
        transformer(buffer[0], buffer[1], &original_byte[1], 7, 6, 0, 0);
        transformer(buffer[2], buffer[3], &original_byte[1], 5, 0, 4, 4);
        // octet numero 2
        transformer(buffer[2], buffer[3], &original_byte[2], 7, 4, 1, 1);
        transformer(buffer[4], buffer[5], &original_byte[2], 3, 0, 4, 4);
        // octet numero 3
        transformer(buffer[4], buffer[5], &original_byte[3], 7, 2, 2, 2);
        transformer(buffer[6], buffer[7], &original_byte[3], 1, 0, 4, 4);
        // octet numero 4
        transformer(buffer[6], buffer[7], &original_byte[4], 7, 0, 3, 3);

        // je calcule la position du pointeur de fichier : fichier_a_decoder ; si il n'est pas à la fin , on écrit ans le fichier dans la façon la plus normal .
        // car on s'intérresse spécialement aux derniers octets du fichier encodé.
        long current_position = ftell(fichier_a_decoder);
        if (current_position != position_fin)
        {
            int nbr_octets_ecrit = fwrite(original_byte, sizeof(original_byte), 1, sortie_decodage);
            if (nbr_octets_ecrit != 1)
            {
                printf(" PROBLEME LORS DE L'ECRITURE DANS LE FICHIER DE SORTIE ./");
                break;
            }
        }
        else
        {
            // si je suis là ; càd que je viens de lire les derniers 8 octets de mon fichier

            // variable qui m'aide à se déplacer de la droite vers la gauche de mes octets décoder .
            int pos;

            // j'initialise à 55 car c'est impossible que la variable 'marquer' soit mise à 55 dans la boucle suivante.
            int marquer = 55; // variable qui sert à marque l'octet du premier 1 venant de la droite .
            for (int  i = 4; i >= 0 ;i--)
            {
                if (original_byte[i]== 128)
                {
                    marquer = i;
                    break;
                }
            }
            if (marquer != 55)
            {
                for (int j = 0; j < marquer; j++)
                {
                    fputc(original_byte[j],sortie_decodage);
                }
            }
    
        }
    }
    close(t[0]);
    close(t[1]);
    fclose(fichier_a_decoder);
    fclose(sortie_decodage);

    return 0;
}
