/* dans tous les livres que j'ai vu ; ils utilisent des unsinged char pour lire dans un fichier ouvert en binaire ; 
car celà nous aide à lire une vamme vomplète de valeurs càd qu'on peut representé de 0 à 255 caracteres sans se soucié du
débodrdement signé*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 8


// fonction qui va transformer les octets du décodage qui sont IMPRIMABLE à leur forme d'origine binaire .
void imprimable_en_bin(char *caractere_imprimable)
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
// fonction génerique , qui va transformer les octets de gauche en leur forme originelle.
void transformer(char octet_X, char octet_Y, char *original_target_Z, int position_de_debut_octet_original, int position_de_fin_octet_original, int starting_pos_octetX, int starting_pos_octetY)
{
    unsigned char bit_value = 0;
    for (int i = position_de_debut_octet_original; i >= position_de_fin_octet_original; i--)
    {
        if (i % 2 != 0)
        {
            // valeur du bit récuperer à la position " position du bit à loctet Y " .
            bit_value = 0;
            bit_value = ((1u << starting_pos_octetX) & octet_X) >> starting_pos_octetX;
            *original_target_Z |= (bit_value << i);
            starting_pos_octetX--;
        }
        else
        {
            // valeur du bit récuperer à la position " position du bit à loctet Y " .
            bit_value = 0;
            bit_value = ((1u << starting_pos_octetY) & octet_Y) >> starting_pos_octetY;
            *original_target_Z |= (bit_value << i);
            starting_pos_octetY--;
        }
    }
}
int main(int argc , char *argv[])
{
    if(argc != 3) {
        printf("manque d'arguments !\n");
        return 3;
    }
    FILE *fichier_a_decoder = fopen(argv[1], "rb");
    if (fichier_a_decoder == NULL)
    {
        printf(" erreur d'ouverture du fichier A dEcoder !");
        return 4;
    }
    FILE *sortie_decodage = fopen(argv[2], "w");
    if (sortie_decodage == NULL)
    {
        printf(" erreur d'ouverture du fichier de sortie !\n");
        return 5;
    }

    // enregistrer la position à la fin du fichier .
    fseek(fichier_a_decoder, 0, SEEK_END);
    long position_fin = ftell(fichier_a_decoder);
    rewind(fichier_a_decoder);

    while (1)
    {
        // buffer qui récupère 8 octet à chaque fois .
        unsigned char buffer[BUFSIZE];
        // avec cette variable on va reconstruire les octets d'origines .
        unsigned char original_byte[5] = {0};
        int nbr_octets_lu = fread(buffer, sizeof(unsigned char), 8, fichier_a_decoder);
        if (nbr_octets_lu == 0 && feof(fichier_a_decoder))
        {
            // ça veut dire qu'on a lu tout les octets du fichier & on a essayé de lire au délà de la EOF .
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
    fclose(fichier_a_decoder);
    fclose(sortie_decodage);

    return 0;
}