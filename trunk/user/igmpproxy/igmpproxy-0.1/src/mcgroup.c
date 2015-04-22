/*
**  igmpproxy - IGMP proxy based multicast router 
**  Copyright (C) 2005 Johnny Egeland <johnny@rlo.org>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**----------------------------------------------------------------------------
**
**  This software is derived work from the following software. The original
**  source code has been modified from it's original state by the author
**  of igmpproxy.
**
**  smcroute 0.92 - Copyright (C) 2001 Carsten Schill <carsten@cschill.de>
**  - Licensed under the GNU General Public License, version 2
**  
**  mrouted 3.9-beta3 - COPYRIGHT 1989 by The Board of Trustees of 
**  Leland Stanford Junior University.
**  - Original license can be found in the Stanford.txt file.
**
*/
/**
*   mcgroup contains functions for joining and leaving multicast groups.
*
*/

#include "igmpproxy.h"
       

/**
*   Common function for joining or leaving a MCast group.
*/
int joinleave( int Cmd, int UdpSock, struct IfDesc *IfDp, uint32_t mcastaddr ) {
    struct ip_mreq CtlReq;
    const char *CmdSt = Cmd == 'j' ? "join" : "leave";
    
    memset(&CtlReq, 0, sizeof(CtlReq));
    CtlReq.imr_multiaddr.s_addr = mcastaddr;
    CtlReq.imr_interface.s_addr = IfDp->InAdr.s_addr;
    
    {
        my_log( LOG_NOTICE, 0, "%sMcGroup: %s on %s", CmdSt, 
            inetFmt( mcastaddr, s1 ), IfDp ? IfDp->Name : "<any>" );
    }
    
    if( setsockopt( UdpSock, IPPROTO_IP, 
          Cmd == 'j' ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP, 
          (void *)&CtlReq, sizeof( CtlReq ) ) ) 
    {
        my_log( LOG_WARNING, errno, "MRT_%s_MEMBERSHIP failed", Cmd == 'j' ? "ADD" : "DROP" );
        return 1;
    }
    
    return 0;
}

