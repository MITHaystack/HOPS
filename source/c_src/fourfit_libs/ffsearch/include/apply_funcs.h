/*
 * capture some interfaces to norm_fx
 */

/* presumes other .h files included first */
#ifndef APPLY_FUNCS_H_
#define APPLY_FUNCS_H_

extern void apply_passband (int sb, int ap, struct freq_corel *fdata,
    hops_complex *xp_spectrum, int npts, struct data_corel *datum,
    struct type_status *status, struct type_param *param);

extern void apply_notches(int sb, int ap, struct freq_corel *fdata,
    hops_complex *xp_spectrum, int npts, struct data_corel *datum,
    struct type_status *status, struct type_param *param);

extern void apply_video_bp (hops_complex *xp_spec, int npts,
    struct type_pass *pass);
extern void fit_vbp(int npts);

extern void apply_cmplxbp(int sb, struct freq_corel *fdata,
    hops_complex *xp_spectrum, int npts, struct type_pass *pass);

extern double adjust_snr(struct type_pass *pass,
    struct type_status *status);

#endif /* APPLY_FUNCS_H_ */
/*
 * eof
 */
