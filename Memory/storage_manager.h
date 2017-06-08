#ifndef _STORAGE_MANAGER_h
#define _STORAGE_MANAGER_h

#include "arduino.h"


/*
* --- GESTION DE LA MEMOIRE ---
*
* --- Structure de la m�moire ---
* Concatenation de blocks de 18 bytes avec chaque block un �chantillon de donn�es.
*
* --- Fonctions accessibles aux autres parties du syst�me ---
* 1) void stor_write_sample(byte *sample)
*
* 2) int stor_read_sample(byte *buffer, int maxlen)
*
* 3) void stor_confirm_read(bool do_commit)
*
* --- Variables internes pour gestion ---
* 1) addresse_actuel: l'addresse o� le prochain �chantillon va �tre stock�
* Cette adresse est incr�ment�e apr�s chaque appel de fonction 'store_echantillon'
* Cette adresse est remis � son valeur initielle quand toute la m�moire
* utilis�e est report�e
*
* 2) addresse_lu: l'addresse o� la prochaine lecture de donn�es va se effectuer
* Cette adresse est incr�ment�e apr�s chaque appel de fonction 'get_data'
* Cette adresse est remis � son valeur initielle quand toute la m�moire
* utilis�e est lue
*
*/

/*
	Set up memory interface
*/
void stor_setup(void);

/*
	Ce fonction va stocker un �chantillon de donn�es dans la m�moire � partir de la
	premi�re addresse qui est libre.
*/
void stor_write_sample(uint8_t * data);

/*
	Ce fonction va lire et retourner les donn�es avec longueur 'len' qui sont stock�s
	dans la m�moire � partir de l'adresse 'addresse_lu'.
*/
uint16_t stor_read_sample(uint8_t *buffer, uint16_t maxlen);

/*
	Avancer la t�te de lecture � la derni�re position lue, ou recoule la position de lecture � la t�te de lecture
*/
void stor_confirm_read(bool do_commit);

/*
	Query available lenght of data store
*/
uint16_t stor_available(void);

void stor_erase_eeprom();





#endif

