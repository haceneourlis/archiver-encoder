/* dans tous les livres que j'ai vu ; ils utilisent des unsinged char pour lire dans un fichier ouvert en binaire ; 
car celà nous aide à lire une gamme complète de valeurs càd qu'on peut representé de 0 à 255 caracteres sans se soucié du
débodrdement signé*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 5 // macros qui sert de récupération d'octets dans le fichier argv[1].

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
char bin_en_imprimable(unsigned char *valeur_encodee)
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

int main(int argc , char *argv[])
{
    if(argc != 3) {
        perror("manque d'arguments !");
        exit(EXIT_FAILURE);
    }
    FILE *fichier_a_encoder = fopen(argv[1], "rb");
    if (fichier_a_encoder == NULL)
    {
        printf("erreur d'ouverture du fichier_a_encoder");
        return 2;
    }

    FILE *output_file = fopen(argv[2], "w+");
    if (output_file == NULL)
    {
        printf("erreur d'ouverture du fichier");
        return 3;
    }


    // cette variable résoud bien le probléme du BOURAGE OU NON BOURAGE  ;
    int bourrage_ou_pas = 0;
    /*
    peut etre que faire une boucle while(1) n'est pas le code le plus propre au monde ; mais ça nous a aidé vraiment pour débugger et suivre chaque bits chaque position chaque code ascii 
    car on réinitialise nos buffers à ZERO à chaque fois ce qui est vraiment cool .
    */
    while (1)
    {
        // variable qui récupere les octet du fichier original && // initialisation du buffer avec des zeros;
        unsigned char buffer[BUFF_SIZE] = {0};
        // variable qui met les octet encodés dans " sorite encodage " && // on initialise les octets ( du coté droit ) à zéro .
        unsigned char output_octets[8] = {0};

        int nbr_octet_lu = fread(buffer, sizeof(unsigned char), 5, fichier_a_encoder);
        // vérifier que le retour de fread  c'est  bien 5 octets lu à chaque fois.
        if (nbr_octet_lu == 0 && feof(fichier_a_encoder))
        {
            // ça veut dire que on a rien lu ; fin du fichier ; on sort .
            break;
        }
        if (nbr_octet_lu != 5)
        {
            // il faut faire le bourrage ;
            buffer[nbr_octet_lu] |= (1u << 7);
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

        int nbr_bytes_written = fwrite(output_octets, sizeof(output_octets), 1, output_file);
        if (nbr_bytes_written != 1)
        {
            printf("erreur ecriture dans le fichier de sortie !\n");
            break;
        }
        
    }
    // si on a pas fait de bourrage ; on rajoute quand meme un bloc de 8 octet au fichier à décoder apres ... ( sortie encodage )
    // comme ça à la remonté du décodage ; on recopie juste ce qui est nécessaire . sans ce soucier si les bits viennet du fichier original ou pas . 
    if(bourrage_ou_pas == 0)
    {
        unsigned char octets_du_non_bourrage[8]={0};
        octets_du_non_bourrage[0]= octets_du_non_bourrage[0] | (1u << 4);
        for (int t = 0; t <= 7; t++)
        {
            bin_en_imprimable(&octets_du_non_bourrage[t]);
        }
        int nbr_bytes_written = fwrite(octets_du_non_bourrage, sizeof(octets_du_non_bourrage), 1, output_file);
        if (nbr_bytes_written != 1)
        {
            printf("erreur ecriture dans le fichier de sortie ! (DERNIERE ECRITURE A;K;A NON_BOURRAGE) \n");
            return 88;
        }
    }
    fclose(fichier_a_encoder);
    fclose(output_file);
    return 0;
}
