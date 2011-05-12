/*
 * vim:set ts=3:
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "upsampler.h"
#include "define_common.h"


/**
 * prend les données sous échantillonnées de MCU_ds, et fait le formatage
 * nécessaire pour pouvoir afficher le résultat
 * Le résultat est stocké dans MCU_us
 * 	o h_factor : facteur de sous échantillonage horizontal pour le bloc (si 1 :
 *    	pas de sous échantillonage, si 2, 1valeur pour 2 pixels adjacents, etc...)
 * 	o v_factor : facteur de sous échantiollonage vertical
 * 	o nb_MCU_H : nombre de blocs 8x8 en largeur (en sortie !)
 * 	o nb_MCU_V : nombre de blocs 8x8 en hauteur (en sortie toujours)
 */
void upsampler(uint8_t *MCU_ds, uint8_t *MCU_us, uint8_t h_factor, uint8_t v_factor, uint16_t nb_MCU_H, uint16_t nb_MCU_V) {

	int us_index = 0, ds_index = 0 , index = 0, c_index;
	int base_index = 0;
	int stop_cond = 64 * nb_MCU_V * nb_MCU_H;
	
	// 	_ h_factor > 1, v_factor = 1 : 422 classique
	// 	_ h_factor = 1, v_factor > 1 : 440
	// 	_ h_factor > 1, v_factor > 1 : 420
	// 	_ h_factor = 1, v_factor = 1 : 444, ou Y
	if ((v_factor == 1) && (h_factor == 1)) {
		// on ne reformate que si aucun sous échantillonage a été testé ( ca doit
		// pouvoir poser problème dans le cas d'échantillonage bizarre...)
		// le reformatage est nécessaire pour la suite : le bloc obtenu après
		// upsampler correspond à une MCU de taille 8*nb_MCU_Hx8*nb_MCU_V
		// la récupération des MCUs, par contre, nous donne des blocs indépendants
		// (le premier bloc prend les premières adresses, etc). On doit donc
		// reformater pour que les premières adresses correspondent à la première
		// ligne du bloc final
		// on parcourt le bloc de sortie, le fonctionnement reste identique aux
		// cas précédents
		while (us_index < stop_cond) {
			//
			int increment = ((64 * nb_MCU_H) - 8);
			for (index = 0 ; index < nb_MCU_H ; index ++) {
				memcpy(MCU_us+us_index, MCU_ds+base_index, 8);
				us_index += 8;
				base_index += 64;
			}
			base_index -= increment	;
			if (base_index == 64) {
				base_index = 64 * nb_MCU_H;
			}
		}
	} else {
		// Il faut étendre les données
		// On parcourt le buffer d'entrée, et on double les données en sorti
		// la condition d'arrêt concerne la sortie 
		// us_index : indice pour le buffer de sortie
		// ds_index : indice pour le buffer d'entrée
		// index : indice générique pour la duplication

		// tant qu'on n'a pas rempli le buffer de sorti
		while (us_index < stop_cond) {
			base_index = us_index;
			// on duplique horizontalement sur une ligne
			for(c_index = 0;c_index < 8;c_index++)
			{
				// on boucle sur le facteur de sous-échantillonage, pour dupliquer
				// h_factor fois
				// A chaque fois, on incrémente us_index, ds_index est constant

				for (index = 0 ; index < h_factor ; index++) {
					MCU_us[us_index] = MCU_ds[ds_index];
					us_index++;
				}
				ds_index++;
			}
			// on recopie la ligne en fonction du sous echantillonage vertical
			for (index = 1 ; index < v_factor ; index++) {
				memcpy(MCU_us + us_index, MCU_us + base_index , 8 * nb_MCU_H);
				us_index += 8 * nb_MCU_H;
			}
		}
	}
}

