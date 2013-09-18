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

/*
 VIM: PLEASE USE:
 :e ++enc=cp850
*/

#ifndef __BUILTINS_PULSE_TXT__
#define __BUILTINS_PULSE_TXT__

#define SEC_TO_WAIT 10
#define MAX_MENU_PASSWD_SIZE 10
// default lang "fr_FR" or "C" or "pt_BR"
#define DEFAULT_LANG "C"

#define PASSWD_FR " Mot de passe:"
#define PASSWD_EN " Password:"
#define PASSWD_PT " Senha:"

#define PASSWD_ERROR_FR "\n Mauvais mot de passe\n"
#define PASSWD_ERROR_PT "\n Senha incorreta\n"
#define PASSWD_ERROR_EN "\n Wrong password\n"

#define PROMPT_PASSWD_FR(x) "\n Merci de saisir le mot de passe d'accŠs au menu.\n\
 Si vous ne savez pas ce que cela signifie ou que vous ne poss‚dez pas\n\
 ce mot de passe, ne faites rien !\n\n\
 Appuyez sur une touche pour saisir le mot de passe ou ESC pour d‚marrer\n\
 imm‚diatement\n\n\
 D‚marrage par d‚faut dans "#x" secondes "

//le Æ n'est pas utilisable
#define PROMPT_PASSWD_PT(x) "\n Por favor digite a senha para acessar o menu de boot via rede.\n\
 Se vocˆ nao sabe o que isso significa ou nao tenha a senha correta,\n\
 entao nao fa‡a nada!\n\n\
 Pressione qualquer tecla para digitar a senha ou ESC para iniciar agora\n\n\
 Tempo de boot padrao em "#x" segundos "

#define PROMPT_PASSWD_EN(x) "\n Please enter the password to access network boot menu.\n\
 If you don't know what it means or don't have the right password,\n\
 don't do anything !\n\n\
 Press any key to enter the password or ESC to start now\n\n\
 Default boot in "#x" seconds ";

#define PASSWD_PROMPT_PT(X) PROMPT_PASSWD_PT(X)
#define PASSWD_PROMPT_FR(X) PROMPT_PASSWD_FR(X)
#define PASSWD_PROMPT_EN(X) PROMPT_PASSWD_EN(X)

#endif
