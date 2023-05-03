#ifndef _XT_INTERFACES_H
#define _XT_INTERFACES_H

struct xt_interfaces_info {
    char interfaces[32];
    u_int8_t direction;//1:in;  2:out; 3:both
    u_int8_t invert;
};

#endif /*_XT_INTERFACES_H*/

