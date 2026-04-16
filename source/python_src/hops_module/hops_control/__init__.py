"""
hops_control - Python-based fringe-fitter control for fourfit4.

Usage in a Python control file (.py)::

    from hops_control import PassInfo, Config

    def configure(p: PassInfo, cfg: Config):
        cfg.ref_freq(215000.0)
        cfg.pc_mode("manual")

        with cfg.IF().station("E"):
            cfg.pc_phases("abcdefgh", [1.0, -2.0, 3.0, -4.0, 5.0, -6.0, 7.0, -8.0])

The ``configure`` function is the single required entry-point.  It receives a
read-only :class:`PassInfo` describing the current fringe pass and a writable
:class:`Config` accumulator.  Every call on *cfg* appends one statement to the
list that fourfit4 will consume. Nested conditional statements are not allowed.
"""

from .pass_info import PassInfo
from .config import Config

__all__ = ["PassInfo", "Config"]
