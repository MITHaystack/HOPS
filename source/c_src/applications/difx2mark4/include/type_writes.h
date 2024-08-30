/* collect all the write prototypes */
#ifndef __type_writes_h
#define __type_writes_h
extern void write_t100 (struct type_100 *pt100, FILE *fout);
extern void write_t101 (struct type_101 *pt101, FILE *fout);
extern void write_t120 (struct type_120 *pt120, FILE *fout);
extern void write_t300 (struct type_300 *pt300, FILE *fout);
extern void write_t301 (struct type_301 *pt301, FILE *fout);
extern void write_t302 (struct type_302 *pt302, FILE *fout);
extern void write_t303 (struct type_303 *pt303, FILE *fout);
extern void write_t309 (struct type_309 *pt309, FILE *fout);
#endif /* __type_writes_h */
