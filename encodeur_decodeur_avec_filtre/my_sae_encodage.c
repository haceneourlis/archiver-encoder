#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFF_SIZE 5

// macros d'erreurs .
#define erreur_du_fork 73 
#define erreur_d_ouverture_du_fichier_a_encoder 74
#define erreur_execl 75
#define erreur_dup2 76
#define erreur_du_wait 77

 
// fonction qui sert à traiter chaqu'un des (<= 5) octets du fichier original et créer les 8 octets do sortie ( ceux à droite ). 
void transformer(unsigned char original_byte, unsigned char *octet_X, unsigned char *octet_Y, int position_de_debut_octet_original, int position_de_fin_octet_original, int starting_pos_octet_X, int starting_pos_octet_Y)
{
    unsigned char bit_value = 0;
    for (int i = position_de_debut_octet_original; i >= position_de_fin_octet_original; i--)
    {
        if (i % 2 != 0)
        {
            // valeur du bit récuperer à la position " i impaire " .
            bit_value = 0;
            bit_value = ((1u << i) & original_byte) >> i;
            *octet_X |= (bit_value << starting_pos_octet_X);
            starting_pos_octet_X--;
        }
        else
        {
            // valeur du bit récuperer à la position " i pair " .
            bit_value = 0;
            bit_value = ((1u << i) & original_byte) >> i;
            *octet_Y |= (bit_value << starting_pos_octet_Y);
            starting_pos_octet_Y--;
        }
    }
}

// Function qui transforme les octets encodés en des caractéres imprimables .
char bin_en_imprimable(char *valeur_encodee)
{
    if (*valeur_encodee <= 25)
    {
        *valeur_encodee = 'a' + *valeur_encodee; // pour arriver à 97 ... 122 /
    }
    else
    {
        *valeur_encodee = *valeur_encodee + 22; // pour arriver à 48 , 49 ... , 53 /
    }
}
int main (int argc , char *argv[])
{
    int t[2];
    pipe(t); // création du pond d'information entre les deux processus .
    FILE *fichier_a_encoder;
    FILE *fichier_sortie_encodage;
    if (argc == 1)
    {
        fichier_a_encoder = stdin ;
        fichier_sortie_encodage = stdout;
    }
    else
    {
        if (argc == 2)
        {
            if ((strcmp(argv[1],"-uu"))==0)
            /* 
             ./my_sae_encoding -uu << tag                   donc on doit reproduire ça :
              blah blah                                      --------------------------
              toto titi                                      ./my_sae_encoding << tag | uuencode -m sortie
              tata tutu                 ==>                    blah blah 
              tag                                             toto titi .....
                                                              tag
            */             
            {
                int id = fork();
                if(id < 0) return erreur_du_fork;
                if (id == 0) // PROCESSUS FILS
                {
                    // on ferme le descripteur de lecture car on a pas besoin ( le prof nous a conseillé de faire ça ).
                    close(t[0]);
                    // encoder l'entrée standard et renvoyer l'encodage sur la sortie satndard .
                    fichier_a_encoder = stdin ;
                    fichier_sortie_encodage = stdout;

                    if( dup2(t[1],STDOUT_FILENO) == -1) // comme ça on a relié la sorite standard avec t[1] { descripteur d'ecriture de notre pipe }.
                    //tout ce qui sera ecrit sur la sortie standard au départ de la prochaine instruction va etre redirigé vers t[1]. 
                   {
                    perror("erreur de dup2");
                    return erreur_dup2;
                   }
                    close(t[1]);
                    // encodage ...
                    
                }
                else // PROCESSUS PÉRE 
                {
                    // on ferme le descripteur d'ecriture cas on a pas besoin
                    close(t[1]);

                    
                    if (dup2(t[0], STDIN_FILENO) == 0) // pour les prochaines commandes ; l'entrée standard c'est t[0] - descripteur de lecture du pipe -
                    {
                    perror("erreur de dup2");
                    return erreur_dup2;
                    }
                    close(t[0]);

                    // Executer uuencode sorie
                    execl("/usr/bin/uuencode", "uuencode","-m","nom_du_fichier_Ap_encodage_decodage", NULL);
                    perror("execl failed");
                    if (wait(NULL)<0) return erreur_du_wait;
                    return erreur_execl;
                }
            }
            else
            {
                fichier_a_encoder = fopen(argv[1],"rb");
                if (fichier_a_encoder == NULL)
                {
                    perror("erreur d'ouverture du fichier A encoder !!\n");
                    return erreur_d_ouverture_du_fichier_a_encoder;
                }
                
                fichier_sortie_encodage = stdout ;
            }
            
        }else if (argc == 3)
            {
                if ((strcmp(argv[1],"-uu"))==0)
                /* 
                    ./my_sae_encoding -uu fichier_a_encoder_ici    ==>            donc on doit reproduire ça :
                                                                                ./my_sae_encoding fichier_a_encoder_ici | uuencode -m sortie .
                */    
                {
                    // on doit donc gérer la sortie standard de ./my_sae_encoding ( la rediriger vers uuencode .)
                    int id = fork();
                    if(id < 0 ) return erreur_du_fork ;
                    if (id == 0) // processus fils
                    {
                        fichier_a_encoder = fopen(argv[2],"rb");
                        if (fichier_a_encoder == NULL)
                        {
                            perror("ERREUR d'ouverture du fichier_a_encoder ;\n");
                            return erreur_d_ouverture_du_fichier_a_encoder;
                        }
                        fichier_sortie_encodage = stdout ; 

                        close(t[0]); // fermeture du descripteur 'pipe' de lecture ; on en a pas besoin ...
                        if (dup2(t[1],STDOUT_FILENO)==-1)
                        {
                            perror("erreur de dup2");
                            return erreur_dup2;
                        }
                        close(t[1]);

                        // encodage du fichier argv[2]
                    }
                    else // processus pére
                    {
                        close(t[1]);
                        if (dup2(t[0],STDIN_FILENO) == -1)
                        {
                            perror("erreur de dup2");
                            return erreur_dup2;
                        }
                        close(t[0]);

                        // Executer uuencode nom_a_a_la_sortie
                        execl("/usr/bin/uuencode", "uuencode","-m","nom_du_fichier_Ap_encodage_decodage", NULL);
                        perror("execl failed");
                       if( wait(NULL) < 0) return erreur_du_wait;
                        return erreur_execl;
                    }                    
                }
                else
                {
                    fichier_a_encoder = fopen(argv[1], "rb");
                    fichier_sortie_encodage = fopen(argv[2],"w");
                }
            }
            else if (argc == 4)
            {
                 if ((strcmp(argv[1],"-uu"))==0)
                /*  si argv[1] = '-uu'
               ./my_sae_encoding -uu [file_a_encoder] [file_ou_mettre_l'encodage]  ==> ./my_sae_encoding < [file_a_encoder] | uuencode nom_du_fichier_Ap_encodage_decodage > [file_ou_mettre_l'encodage] 
                */
                {
                    int id = fork();
                    if(id < 0) return erreur_du_fork ;
                    if(id == 0) // fils
                    {
                        fichier_a_encoder = fopen(argv[2],"rb");
                        if (fichier_a_encoder == NULL)
                        {
                            perror("ERREUR d'ouverture du fichier_a_encoder; ligne 180\n");
                            return erreur_d_ouverture_du_fichier_a_encoder;
                        }
                        fichier_sortie_encodage = stdout ;
                        close(t[0]);
                        if ( dup2(t[1],STDOUT_FILENO)==-1)//redirection de la sortie standard .
                        {
                            perror("erreur de dup2");
                            return erreur_dup2;
                        }
                        close(t[1]);
                    }
                    else // pére
                    {
                        close(t[1]);
                        // lié l'entrée standrad au descripteur de lecture du pipe ; comma ça on dit au prochaines commandes 
                        // votre entrée standard est  t[0].
                        if (dup2(t[0],STDIN_FILENO)==-1)
                        {
                            perror("erreur de dup2");
                            return erreur_dup2;
                        }
                        close(t[0]);
                        // redirigé la sortie standard vers argv[3] ; eh oui !
                        int redirection_stdout_descripteur = open(argv[3], O_WRONLY | O_CREAT, 0777);
                        if(redirection_stdout_descripteur < 0)
                        {
                            perror("erreur d'ouverture du fichier : ARGV[3]");
                            return 37;
                        }
                        if (dup2(redirection_stdout_descripteur,STDOUT_FILENO)==-1)
                        {
                            perror("erreur de dup2");
                            return erreur_dup2;
                        }
                        close(redirection_stdout_descripteur);
                        // Executer uuencode nom_a_a_la_sortie
                        execl("/usr/bin/uuencode", "uuencode","-m","nom_du_fichier_Ap_encodage_decodage", NULL);
                        perror("execl failed");
                        if (wait(NULL)<0) return erreur_du_wait ;// attend le fils ..;
                        return erreur_execl;
                    }
                }
                else
                {
                    fichier_a_encoder = fopen(argv[1], "rb");
                    fichier_sortie_encodage = fopen(argv[2],"w");
                }
            }
            
        }
    // variable qui sert à indiquer si y a eu bourrage ou pas ;
    int bourrage_ou_pas= 0;
    while (1)
    {
        // initialisation du beffer avec des zeros;
        unsigned char buffer[BUFF_SIZE] = {0};
        // on initialise les octets ( du coté droit ) à zéro .
        unsigned char output_octets[8] = {0};

        // j'ai eu des erreur en comparant le retour de fgetc avec "c" car c est un unsigned char , fgetc retourne -1 quand == EOF.
        int retour_fget_c ;

        unsigned char c;
        int m = 0;
        while (m != 5) 
        {
            // j'ai eu des erreur en comparant le retour de fgetc avec "c" car c est un unsigned char , fgetc retourne -1 quand == EOF.

            if ((retour_fget_c  = fgetc(fichier_a_encoder))!= EOF) {
                c = (unsigned char)retour_fget_c;
                buffer[m] = c;
                m++;
            } else {
                break; // on sort de la boucle inérieure qui nous sert à lire l'entrée standard .
            }
        }
        // vérifier que le retour c'est  bien 5 octets lu à chaque fois.
        if (m == 0 && feof(fichier_a_encoder))
        {
            // donc pas besoin de faire un bourrge ;;;
            break;
        }
        if(m != 5)
        {
            // il faut faire le bourrage ;
            buffer[m] |= (1u << 7);
            bourrage_ou_pas = 1;
        }

        // création des octets ( à gauche ).
        // création de output_octet[0] et output_octets[1]
        transformer(buffer[0], &output_octets[0], &output_octets[1], 7, 0, 4, 4);
        transformer(buffer[1], &output_octets[0], &output_octets[1], 7, 6, 0, 0);
        // création de output_octet[2] et outpuy_octets[3]
        transformer(buffer[1], &output_octets[2], &output_octets[3], 5, 0, 4, 4);
        transformer(buffer[2], &output_octets[2], &output_octets[3], 7, 4, 1, 1);
        // création de output_octet[4] et output_octet[5]
        transformer(buffer[2], &output_octets[4], &output_octets[5], 3, 0, 4, 4);
        transformer(buffer[3], &output_octets[4], &output_octets[5], 7, 2, 2, 2);
        // création de output_octet[6] et output_octet[7]
        transformer(buffer[3], &output_octets[6], &output_octets[7], 1, 0, 4, 4);
        transformer(buffer[4], &output_octets[6], &output_octets[7], 7, 0, 3, 3);

        // transformation en caractéres imprimables .
        for (int t = 0; t <= 7; t++)
        {
            bin_en_imprimable(&output_octets[t]);
        }

        int nbr_bytes_written = fwrite(output_octets, sizeof(output_octets), 1, fichier_sortie_encodage);
        if (nbr_bytes_written != 1)
        {
            printf("erreur d'ecriture dans le fichier de sortie !");
            break;
        }
    }
    // si on a pas fait de bourrage ; on rajoute quand meme un bloc de 8 octet au fichier à décoder apres ... ( sortie encodage )
    // comme ça à la remonté du décodage ; on recopie juste ce qui est nécessaire .
    // probléme résolue . YOHOO
    if(bourrage_ou_pas == 0)
    {
        unsigned char octets_du_non_bourrage[8]={0};
        octets_du_non_bourrage[0]= octets_du_non_bourrage[0] | (1u << 4);
        for (int t = 0; t <= 7; t++)
        {
            bin_en_imprimable(&octets_du_non_bourrage[t]);
        }
        int nbr_bytes_written = fwrite(octets_du_non_bourrage, sizeof(octets_du_non_bourrage), 1, fichier_sortie_encodage);
        if (nbr_bytes_written != 1)
        {
            printf("erreur ecriture dans le fichier de sortie ! (DERNIERE ECRITURE A;K;A NON_BOURRAGE) \n");
            return 88;
        }
    }
    close(t[0]);
    close(t[1]);
    fclose(fichier_a_encoder);
    fclose(fichier_sortie_encodage);

    return 0;
}
