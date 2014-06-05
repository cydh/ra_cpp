/* 
 * File:   ch_clif.cpp
 * Author: lighta
 * 
 * Created on June 5, 2014, 4:42 AM
 */

#include "ch_main.h"
#include "ch_clif.h"

ch_clif::ch_clif() {
}

ch_clif::ch_clif(const ch_clif& orig) {
}

ch_clif::~ch_clif() {
}

void ch_clif::moveCharSlotReply( int fd,const char_session_data& sd, unsigned short index, short reason ) const {
//	WFIFOHEAD(fd,8);
//	WFIFOW(fd,0) = 0x8d5;
//	WFIFOW(fd,2) = 8;
//	WFIFOW(fd,4) = reason;
//	WFIFOW(fd,6) = sd.char_moves[index];
//	WFIFOSET(fd,8);
}

