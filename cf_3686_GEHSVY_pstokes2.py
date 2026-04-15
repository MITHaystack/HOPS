"""
Python control file equivalent of cf_3686_GEHSVY_pstokes2.

Generated from the VT9077 DSL control file.
"""

from hops_control import PassInfo, Config


def configure(p: PassInfo, cfg: Config) -> None:

    # ==========================================================
    # basic setup
    # ==========================================================

    cfg.dr_win([-5.e-6, 5.e-6])
    cfg.pc_mode("multitone")
    cfg.pc_period(1)
    cfg.ion_smooth(True)
    cfg.mbd_anchor("sbd")
    cfg.samplers(["abcdefgh", "ijklmnop", "qrstuvwx", "yzABCDEF"])
    cfg.ref_freq(10000.0)
    cfg.weak_channel(0.1)
    cfg.pc_amp_hcode(0.001)
    # general ionosphere search parameters:
    cfg.ion_npts(45)
    cfg.ion_win([-88.0, 88.0])
    # set tone mask for n*100 MHz and 3090 MHz:
    cfg.pc_tonemask("cdejnprwBC", [2, 16, 16, 1, 16, 16, 16, 2, 16, 16])

    # ==========================================================
    # speed up ionosphere search on GE baseline
    # ==========================================================

    with cfg.if_().baseline("GE"):
        cfg.ion_npts(11)
        cfg.ion_win([-20.0, 20.0])

    # ==========================================================
    # compensating for low pcal amp for station Yj and Oe
    # ==========================================================

    with cfg.if_().station("Y"):
        cfg.pc_amp_hcode(0.0001)

    with cfg.if_().station("T"):
        cfg.pc_amp_hcode(0.0001)

    # ==========================================================
    # station sampler delays
    # ==========================================================

    with cfg.if_().station("G"):
        cfg.sampler_delay_x([-140, 180, 180, 180])
        cfg.sampler_delay_y([-140, 180, 180, 180])

    with cfg.if_().station("E"):
        cfg.sampler_delay_x([-20, -20, -20, -20])
        cfg.sampler_delay_y([-20, -20, -20, -20])

    with cfg.if_().station("H"):
        cfg.sampler_delay_x([-150, 130, 130, 130])
        cfg.sampler_delay_y([-150, 130, 130, 130])

    with cfg.if_().station("V"):
        cfg.sampler_delay_x([20, -10, 20, 20])
        cfg.sampler_delay_y([20, -10, 20, 20])

    with cfg.if_().station("Y"):
        cfg.sampler_delay_x([62, 59, 32, 71])
        cfg.sampler_delay_y([62, 59, 32, 71])

    with cfg.if_().station("I"):
        cfg.sampler_delay_x([15, -15, -45, -50])
        cfg.sampler_delay_y([15, -15, -45, -50])

    with cfg.if_().station("S"):
        cfg.sampler_delay_x([82, 96, 84, 72])
        cfg.sampler_delay_y([82, 96, 84, 72])

    with cfg.if_().station("T"):
        cfg.sampler_delay_x([-35, -23, -29, -44])
        cfg.sampler_delay_y([-35, -36, -40, -55])

    # ==========================================================
    # station pc_phases
    # ==========================================================

    with cfg.if_().station("E"):
        cfg.pc_phases_x("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-1.9, -1.3, 2.1, -6.6, -3.3, 8.4, 17.3, 18.2,
                         5.9, 11.1, 8.4, 8.3, 6.7, 5.5, -0.6, 3.7,
                         -23.2, -22.3, -24.9, -23.3, -21.6, -19.7, -16.0, -14.4,
                         22.1, 15.1, 13.4, 4.8, 4.1, 2.6, 9.2, 13.5])
        cfg.pc_phases_y("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-14.3, -5.0, -5.0, -27.7, -10.6, 8.0, 26.2, 39.9,
                         6.5, 12.5, 16.1, 22.2, 20.9, 11.3, 7.6, 10.4,
                         -21.1, -21.8, -27.9, -29.8, -19.7, -19.7, -14.3, -9.1,
                         10.2, 8.7, 6.1, 11.0, 4.3, -3.1, 2.6, 2.8])

    with cfg.if_().station("G"):
        cfg.pc_phases_y("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-0.4, 3.1, 4.4, 2.3, -4.3, -2.4, 0.4, 1.1,
                         -0.9, 2.0, 1.4, 0.1, 1.0, 1.2, -0.1, -2.4,
                         -0.8, -0.5, -0.1, 0.4, 0.3, 0.5, -0.8, -0.8,
                         -4.0, -6.3, -3.6, -3.5, -4.4, -4.9, -1.4, -1.2])

    with cfg.if_().station("H"):
        cfg.pc_phases_x("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-6.3, -10.8, -8.1, -1.4, -3.8, 4.6, 2.8, 3.5,
                         -3.4, -0.3, 0.4, 4.1, 11.2, 14.8, 8.7, 6.5,
                         0.8, -3.0, -1.7, 1.0, -1.1, -2.1, -3.0, -1.2,
                         -15.2, -16.5, -12.0, -6.6, -2.8, -3.4, 5.4, 4.6])
        cfg.pc_phases_y("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-6.4, -8.8, -7.6, 0.2, -4.0, -1.6, 0.4, 2.0,
                         -0.3, 1.3, 1.9, 4.1, 13.3, 14.8, 9.2, 7.9,
                         2.5, -0.3, -1.2, -1.4, -2.5, -1.6, -0.6, -1.3,
                         -11.6, -12.5, -4.9, -1.6, 4.7, 2.9, 6.6, 6.5])

    with cfg.if_().station("S"):
        cfg.pc_phases_x("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [12.1, 11.8, 11.7, 3.1, -8.6, -3.9, -0.4, -5.5,
                         -20.5, -10.5, -10.0, -6.6, -4.7, -14.6, -10.2, -11.3,
                         21.6, 16.0, 16.3, 10.0, 11.2, 11.4, 5.9, 2.6,
                         2.7, 1.8, -1.9, -2.7, -3.5, -10.8, -5.0, -0.6])
        cfg.pc_phases_y("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [9.4, 2.5, -6.1, -2.3, 2.5, 13.5, 15.1, 9.0,
                         -23.7, -18.0, -14.1, -11.7, -8.5, -17.4, -14.8, -7.8,
                         19.0, 15.2, 13.7, 11.0, 11.7, 10.1, 6.4, 8.2,
                         -10.4, -14.8, -10.6, -4.5, 2.5, 3.5, 7.6, 5.6])

    with cfg.if_().station("V"):
        cfg.pc_phases_x("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-41.2, -34.2, -22.2, 5.8, 24.6, 30.6, 31.7, 25.6,
                         -2.3, 0.6, 1.4, 5.4, 9.1, 19.2, 27.1, 21.1,
                         -7.7, -6.4, -7.8, -10.0, -11.1, -19.4, -20.8, -26.3,
                         2.3, 5.0, 11.0, 2.4, -12.8, -7.8, 5.4, 8.5])
        cfg.pc_phases_y("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-40.2, -31.2, -22.5, 9.9, 30.4, 39.3, 38.9, 30.3,
                         -9.6, -1.5, -7.4, -8.0, -0.9, 7.8, 15.1, 15.5,
                         -5.6, -2.9, -7.7, -9.7, -13.2, -19.4, -22.3, -30.6,
                         -5.4, -4.9, -3.3, -0.7, 2.9, 11.1, 20.4, 18.2])

    with cfg.if_().station("Y"):
        cfg.pc_phases_x("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-47.1, -26.8, -20.8, 13.7, 21.5, 18.9, 27.4, 36.1,
                         -2.8, -5.3, -5.1, -6.5, -2.4, -16.3, -18.1, -16.1,
                         15.4, 12.5, 12.2, 13.6, 18.8, -13.7, -15.9, -14.2,
                         -7.5, -19.6, -8.6, 13.4, 7.0, 4.0, 14.3, 21.4])
        cfg.pc_phases_y("abcdefghijklmnopqrstuvwxyzABCDEF",
                        [-69.1, -46.5, -40.1, 3.8, 30.2, 18.9, 29.6, 36.1,
                         -1.7, -0.4, -6.4, -8.9, 8.8, -19.9, -22.7, -12.9,
                         29.1, 24.1, 20.4, 20.5, 42.2, -10.9, -13.1, -10.2,
                         -12.7, -30.6, -20.1, 9.9, 3.9, 2.7, 14.3, 19.5])

    # ==========================================================
    # station pc_delay and pc_phase_offset
    # ==========================================================

    with cfg.if_().station("E"):
        cfg.pc_delay_x(0.0)
        cfg.pc_delay_y(0.734)        # (ns) estimated error is +/- 0.005
        cfg.pc_phase_offset_x(0.0)
        cfg.pc_phase_offset_y(138.2) # (deg) estimated error is +/- 7.0

    with cfg.if_().station("G"):
        cfg.pc_delay_x(0.0)
        cfg.pc_delay_y(1.684)        # (ns) estimated error is +/- 0.002
        cfg.pc_phase_offset_x(0.0)
        cfg.pc_phase_offset_y(35.0)  # (deg) estimated error is +/- 4.3

    with cfg.if_().station("H"):
        cfg.pc_delay_x(0.0)
        cfg.pc_delay_y(0.141)        # (ns) estimated error is +/- 0.003
        cfg.pc_phase_offset_x(0.0)
        cfg.pc_phase_offset_y(-53.7) # (deg) estimated error is +/- 6.4

    with cfg.if_().station("S"):
        cfg.pc_delay_x(0.0)
        cfg.pc_delay_y(-0.109)       # (ns) estimated error is +/- 0.003
        cfg.pc_phase_offset_x(0.0)
        cfg.pc_phase_offset_y(117.8) # (deg) estimated error is +/- 4.0

    with cfg.if_().station("V"):
        cfg.pc_delay_x(0.0)
        cfg.pc_delay_y(-0.06)        # (ns) estimated error is +/- 0.003
        cfg.pc_phase_offset_x(0.0)
        cfg.pc_phase_offset_y(-135.0) # (deg) estimated error is +/- 6.8

    with cfg.if_().station("Y"):
        cfg.pc_delay_x(0.0)
        cfg.pc_delay_y(0.523)        # (ns) estimated error is +/- 0.002
        cfg.pc_phase_offset_x(0.0)
        cfg.pc_phase_offset_y(58.5)  # (deg) estimated error is +/- 3.1
