#ifndef BITS_H
#define BITS_H

#define set_bit(m,b)	((m) |= (b))
#define	unset_bit(m,b)	((m) &= ~(b))
#define bit_seted(m,b)  ((b) == ((m) & (b)))

#endif
