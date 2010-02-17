/*
 * standard INET to Wii net_ translation
 * For use with ClientSocket class (or whatever)
 *
 * Enables you to use standard pseudo POSIX network api calls
 *
 * Copyright (C) 2008  Alejandro Valenzuela Roca
 *
 * lanjoe9@mexinetica.com
 *
 * This file is part of MotorJ, a free framework for videogame development.
 * <http://wiki.lidsol.net/wiki/index.php?title=MotorJ >
 * <http://motorj.mexinetica.com >
 *
 * MotorJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MotorJ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MotorJ.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NET_TRANS_H
#define NET_TRANS_H

 #define PF_INET6 PF_INET
 #define AF_INET6 AF_INET
 #define WII_NET_INIT_SUCCESS 0x0
 #define WII_NET_INIT_WAIT    0xfffffff5
 #define listen(s,backlog)   net_listen(s,backlog)
 #define accept(s,addr,addrlen)  net_accept(s,addr,addrlen)
 #define socket(domain,type,protocol)       net_socket(domain,type,protocol)
 #define setsockopt(s,level,optname,optval,optlen)         net_setsockopt(s,level,optname,optval,optlen)
 #define bind(s,name,namelen) net_bind(s,name,namelen)
 #define connect(s,sockaddr,socklen_t) net_connect(s,sockaddr,socklen_t)
 #define shutdown(s,how) net_shutdown(s,how)
 #define SHUT_RDWR 0
 //FIXME!!! SHUT_RDWR = 0 ?
 #define send(s,data,size,flags)        net_send(s,data,size,flags)
 #define sendto(s,data,len,flags,to,tolen)          net_sendto(s,data,len,flags,to,tolen)
 #define recv(s,mem,len,flags)      net_recv(s,mem,len,flags)
 #define recvfrom(s,mem,len,flags,from,fromlen)     net_recvfrom(s,mem,len,flags,from,fromlen)
 #define gethostbyname(addr)            net_gethostbyname(addr)


#endif

