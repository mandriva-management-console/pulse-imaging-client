/*
 * (c) 2013 Mandriva, http://www.mandriva.com
 *
 * $Id$
 *
 * This file is part of Pulse 2, http://pulse2.mandriva.org
 *
 * Pulse 2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pulse 2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pulse 2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#ifndef __BUILTINS_PULSE_TXT__
#define __BUILTINS_PULSE_TXT__

#define NB_SECONDE_WAIT 10
// default lang "fr_FR" or "C" or "pt_BR"
#define DEFAULT_LANG "fr_FR"

#define PRONT_PASSWORD_FRANCAIS(x) "\nS'il vous plaŒt, saisissez le mot-de-passe pour modifier le menu de boot.\n\
Si vous ne savez pas ce que cela signifie ou vous ne poss‚dez pas le bon mot-de-passe, ne faite rien !\n\n\
Press une touche pour saisir votre mot-de-passe\n\tor ESC pour d‚marrer\n\n\
boot par d‚faut dans "#x" secondes "

#define PASSWORD_FRANCAIS  "Mot-de-passe :" 
#define PASSWORD_ANGLAIS   "Password :" 
#define PASSWORD_PROTUGAIS "Senha :"

//le Æ n'est pas utilisable
#define PRONT_PASSWORD_PORTUGAIS(x) "Se vocˆ plana, digite a £ltima senha para mudar o menu de inicializa‡ao. \n\
Se vocˆ nao sabe o que significa ou vocˆ nao possdez a senha correta, nao nada\nfeito! \n\n\
Pressione qualquer tecla para digitar sua senha \n\touro ESC para come‡ar\n\n\
Boot padrao por padrao em "#x" segundos "


#define PRONT_PASSWORD_ANGLAIS(x) "\nPlease type the password to change network boot menu.\n\
If you don't know what it means or don't have the right password, don't do anything !n\n\
Press any key to enter your password\n\tgold ESC to start\n\n\
Default boot in "#x" seconds ";



# define PRONT_PORTUGAIS(X) PRONT_PASSWORD_PORTUGAIS(X)
# define PRONT_FRANCAIS(X)  PRONT_PASSWORD_FRANCAIS(X)
# define PRONT_ANGLAIS(X)   PRONT_PASSWORD_ANGLAIS(X)



#endif