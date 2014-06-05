/* 
 * File:   char.h
 * Author: lighta
 *
 * Created on June 5, 2014, 4:47 AM
 */

#ifndef CHAR_H
#define	CHAR_H

#include "../ra_common/mmo.h"

class ch_main {
public:
    ch_main();
    ch_main(const char& orig);
    virtual ~ch_main();
        
private:

};

class char_session_data {
    public:
	bool auth; // whether the session is authed or not
	int account_id, login_id1, login_id2, sex;
	int found_char[MAX_CHARS]; // ids of chars on this account
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	int group_id; // permission
	int char_slots; // total number of characters that can be created
	int chars_vip;
	int chars_billing;
	int version;
	int clienttype;
	char new_name[NAME_LENGTH];
	char birthdate[10+1];  // YYYY-MM-DD
	// Pincode system
	char pincode[PINCODE_LENGTH+1];
	int pincode_seed;
	time_t pincode_change;
	int pincode_try;
	// Addon system
	int bank_vault;
	uint32 char_moves[MAX_CHARS]; // character moves left
	int isvip;
	time_t unban_time[MAX_CHARS];
	int charblock_timer;
};

#endif	/* CHAR_H */

