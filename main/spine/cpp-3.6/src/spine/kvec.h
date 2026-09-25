










#ifndef AC_KVEC_H
#define AC_KVEC_H

#ifndef _kv_free
#define _kv_free(type, p) (FREE(p))
#endif

#ifndef _kv_alloc
#define _kv_alloc(type, s) ((type*)(MALLOC(type, (s))))
#endif

#ifndef _kv_copy
#define _kv_copy(type, d, s, n) memcpy((d), (s), sizeof(type) * (n))
#endif

#define kv_roundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))

#define kvec_t(type) struct { size_t n, m; type *a; }
#define kv_init(v) ((v).n = (v).m = 0, (v).a = 0)
#define kv_destroy(v) _kv_free(type, (v).a)
#define kv_A(v, i) ((v).a[(i)])
#define kv_array(v) ((v).a)
#define kv_pop(v) ((v).a[--(v).n])
#define kv_size(v) ((v).n)
#define kv_max(v) ((v).m)

#define kv_resize(type, v, s) do {									\
		type* b = _kv_alloc(type, (s));								\
		if (((s) > 0) && ((v).m > 0))								\
			_kv_copy(type, b, (v).a, ((s) < (v).m)? (s) : (v).m);	\
		_kv_free(type, (v).a);										\
		(v).a = b; (v).m = (s);										\
	} while (0)
	
#define kv_trim(type, v) kv_resize(type, (v), kv_size(v))

#define kv_copy(type, v1, v0) do {								\
		if ((v1).m < (v0).n) kv_resize(type, v1, (v0).n);		\
		(v1).n = (v0).n;										\
		_kv_copy(type, (v1).a, (v0).a, (v0).n);					\
	} while (0)													\

#define kv_push(type, v, x) do {								\
		if ((v).n == (v).m) 									\
			kv_resize(type, (v), ((v).m? (v).m<<1 : 2));		\
		(v).a[(v).n++] = (x);									\
	} while (0)

#endif
