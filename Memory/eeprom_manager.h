#ifndef _EEPROM_MANAGER_h
#define _EEPROM_MANAGER_h

#include "arduino.h"


/*
* --- GESTION DE LA MEMOIRE ---
*
* --- Structure de la m�moire ---
* La m�moire est divis�e en 2 parties: la premi�re partie comprend la m�tadata,
* la deuxi�me comprend les donn�es recolt�es en attente d'envoi.
*
* 1�re partie - M�tadata:
*    - Num�ro s�riel du box
*    - Timestamp du premier �chantillon des donn�es dans la deuxi�me partie
*    - Information de r�seau: quelle r�seau, RSSI
*    - ...
*
* 2�me partie - Donn�es recolt�es:
* Concatenation de blocks de 18 bytes avec chaque block un �chantillon de donn�es.
*
* --- Fonctions accessibles aux autres parties du syst�me ---
* 1) void store_sample(byte *sample)
*
* 2) int read_samples(byte *buffer, int maxlen)
*
* 3) void commit_head(bool do_commit)
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
void eeprom_setup(void);

/*
	Ce fonction va stocker un �chantillon de donn�es dans la m�moire � partir de la
	premi�re addresse qui est libre.
*/
void store_sample(uint8_t * data);


/*
	Ce fonction va lire et retourner les donn�es avec longueur 'len' qui sont stock�s
	dans la m�moire � partir de l'adresse 'addresse_lu'.
*/
uint16_t read_samples(uint8_t *buffer, uint16_t maxlen);


/*
	Avancer la t�te de lecture � la derni�re position lue, ou recoule la position le lecture � la t�te de lecture
*/
void commit_head(bool do_commit);








#endif

