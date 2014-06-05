/* 
 * File:   ch_clif.h
 * Author: lighta
 *
 * Created on June 5, 2014, 4:42 AM
 */

#ifndef CH_CLIF_H
#define	CH_CLIF_H

#include "ch_main.h"

class ch_clif {
public:
    ch_clif();
    ch_clif(const ch_clif& orig);
    virtual ~ch_clif();
    
    void moveCharSlotReply( int fd, const char_session_data& sd, unsigned short index, short reason ) const;
private:

};

#endif	/* CH_CLIF_H */

