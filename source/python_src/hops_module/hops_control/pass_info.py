"""
PassInfo - read-only view of the current fringe pass identity.

Mirrors the condition-evaluation logic of ``MHO_ControlConditionEvaluator``
so that Python control files can reproduce every ``if station / baseline /
source / f_group / scan`` guard that the DSL supports.
"""


class PassInfo:
    """
    Immutable description of the fringe pass (single-baseline, single-polproduct) being processed.

    Constructed by ``MHO_PyControlEvaluator`` from the C++ parameter store and
    passed as the first argument to the user's ``configure()`` function.

    The *pass_dict* must contain these keys (all strings):

    * ``baseline``   - full baseline string, e.g. ``"EG"`` or ``"Gs-Wf"``
    * ``ref_mk4id``    - single-character Mk4 ref-station code, e.g. ``"E"``
    * ``rem_mk4id``    - single-character Mk4 rem-station code, e.g. ``"G"``
    * ``ref_code``   - canonical ref-station code (e.g. ``Gs``)
    * ``rem_code``   - canonical rem-station code (e.g. ``Wf``)
    * ``source``     - source name, e.g. ``"3C279"``
    * ``fgroup``     - frequency-group character, e.g. ``"X"``
    * ``scan_name``  - scan name / time string used for ordering comparisons
    * ``polprod``    - polarization product, e.g. ``"RR"``
    """

    WILDCARD = "?"

    def __init__(self, pass_dict: dict):
        self._baseline  = pass_dict.get("baseline",  "??")
        self._ref_mk4id   = pass_dict.get("ref_mk4id",   "?")
        self._rem_mk4id   = pass_dict.get("rem_mk4id",   "?")
        self._ref_code  = pass_dict.get("ref_code",  "?")
        self._rem_code  = pass_dict.get("rem_code",  "?")
        self._source    = pass_dict.get("source",    "?")
        self._fgroup    = pass_dict.get("fgroup",    "?")
        self._scan_name = pass_dict.get("scan_name", "?")
        self._polprod   = pass_dict.get("polprod",   "??")

    # ------------------------------------------------------------------
    # Properties (read-only access to pass identity fields)
    # ------------------------------------------------------------------

    @property
    def baseline(self) -> str:
        """Full baseline string (e.g. ``"EG"`` or ``"Gs-Wf"``)."""
        return self._baseline

    @property
    def ref_station(self) -> str:
        """Single-character Mk4 ID of the reference station."""
        return self._ref_mk4id

    @property
    def rem_station(self) -> str:
        """Single-character Mk4 ID of the remote station."""
        return self._rem_mk4id

    @property
    def source(self) -> str:
        """Source name."""
        return self._source

    @property
    def fgroup(self) -> str:
        """Frequency-group character."""
        return self._fgroup

    @property
    def scan_name(self) -> str:
        """Scan name / time string."""
        return self._scan_name

    @property
    def polprod(self) -> str:
        """Polarization product (e.g. ``"RR"``)."""
        return self._polprod

    # ------------------------------------------------------------------
    # Condition helpers - mirror MHO_ControlConditionEvaluator logic
    # ------------------------------------------------------------------

    def station(self, s: str) -> bool:
        """
        Return True if *s* matches the reference or remote station (or is the
        wildcard ``"?"``).

        Single-character *s* is matched against Mk4 station IDs.
        Multi-character *s* is matched against canonical station names.
        """
        if s == self.WILDCARD:
            return True
        if len(s) == 1:
            return s == self._ref_mk4id or s == self._rem_mk4id
        else:
            return s == self._ref_code or s == self._rem_code

    def baseline_match(self, b: str) -> bool:
        """
        Return True if *b* matches the current baseline.

        *b* may be:

        * A 2-character Mk4 baseline (e.g. ``"EG"``).
        * A long-form baseline with ``-`` separator (e.g. ``"Gs-Wf"``).
        * A wildcard pattern using ``"?"`` for either station.
        """
        if len(b) == 2:
            return self._match_two_char_baseline(b)
        elif "-" in b:
            return self._match_long_baseline(b)
        return False

    def source_match(self, src: str) -> bool:
        """Return True if *src* equals the current source name (or is ``"?"``)."""
        return src == self.WILDCARD or src == self._source

    def fgroup_match(self, fg: str) -> bool:
        """Return True if *fg* equals the current frequency group (or is ``"?"``)."""
        return fg == self.WILDCARD or fg == self._fgroup

    def scan_before(self, scan: str) -> bool:
        """Return True if the current scan name is lexicographically *before* *scan*."""
        return self._scan_name < scan

    def scan_after(self, scan: str) -> bool:
        """Return True if the current scan name is lexicographically *after* *scan*."""
        return self._scan_name > scan

    def scan_between(self, lo: str, hi: str) -> bool:
        """Return True if the scan name falls in [*lo*, *hi*] (inclusive)."""
        return lo <= self._scan_name <= hi

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _match_two_char_baseline(self, b: str) -> bool:
        ref, rem = b[0], b[1]
        if b == self._ref_mk4id + self._rem_mk4id:
            return True
        if ref == self.WILDCARD and rem == self._rem_mk4id:
            return True
        if rem == self.WILDCARD and ref == self._ref_mk4id:
            return True
        if ref == self.WILDCARD and rem == self.WILDCARD:
            return True
        return False

    def _match_long_baseline(self, b: str) -> bool:
        parts = b.split("-", 1)
        if len(parts) != 2:
            return False
        ref, rem = parts[0], parts[1]
        ref_match = (ref == self.WILDCARD or ref == self._ref_code or ref == self._ref_mk4id)
        rem_match = (rem == self.WILDCARD or rem == self._rem_code or rem == self._rem_mk4id)
        return ref_match and rem_match

    def __repr__(self) -> str:
        return (
            f"PassInfo(baseline={self._baseline!r}, source={self._source!r}, "
            f"fgroup={self._fgroup!r}, scan={self._scan_name!r}, "
            f"polprod={self._polprod!r})"
        )
