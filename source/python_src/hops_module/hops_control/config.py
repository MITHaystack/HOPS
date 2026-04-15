"""
Config - accumulates fringe-fitter control statements.

Methods on :class:`Config` are generated at construction time from the JSON
format dictionary supplied by ``MHO_PyControlEvaluator`` (the C++ side passes
the canonical ``control_format`` dict loaded from the installed ``.json`` files).

Statements are grouped into *conditional blocks* that mirror the DSL structure.
Calls made outside any ``with cfg.if_()`` block go into an implicit ``if true``
block.  Calls inside a ``with`` block go into a conditional block whose
``value`` array matches the DSL token sequence (e.g. ``["station", "G"]``).

Conditions are built with a fluent builder returned by ``cfg.if_()``.  The
builder mirrors the DSL token grammar directly -- ``and_()``, ``or_()``, and
``not_()`` inject boolean operators, and the predicate methods (``station()``,
``baseline()``, etc.) inject the corresponding operand tokens::

    def configure(p, cfg):
        cfg.ref_freq(215000.0)          # goes into the implicit if-true block

        # DSL: if station G
        with cfg.if_().station("G"):
            cfg.sampler_delay_x([-140, 180, 180, 180])

        # DSL: if baseline GE
        with cfg.if_().baseline("GE"):
            cfg.ion_npts(11)

        # DSL: if source 3C279 and f_group X
        with cfg.if_().source("3C279").and_().fgroup("X"):
            cfg.ref_freq(86000.0)

        # DSL: if station E and scan > 100-1200
        with cfg.if_().station("E").and_().scan_after("100-1200"):
            cfg.pc_amp_hcode(0.0001)

        # DSL: if station E or station G
        with cfg.if_().station("E").or_().station("G"):
            cfg.weak_channel(0.05)

Convenience shortcuts (``cfg.if_station()``, ``cfg.if_baseline()``, etc.) are
provided for the common single-predicate case and are equivalent to the fluent
form.
"""

_SCALAR_TYPES = frozenset({
    "real",
    "int",
    "bool",
    "string",
    "list_real",
    "list_int",
    "list_string",
    "fixed_length_list_string",
    "logical_intersection_list_string",
})

_SKIP_STYPES = frozenset({"deprecated", "unknown", "conditional"})
_VALID_STYPES = frozenset({"parameter", "operator"})


# ---------------------------------------------------------------------------
# Caller factories
# ---------------------------------------------------------------------------

def _scalar_caller(keyword, stmt_type, config_ref):
    """Return a callable that appends a scalar statement to the active block."""
    def caller(value):
        stmt = {"name": keyword, "statement_type": stmt_type, "value": value}
        config_ref._active_statements().append(stmt)
    caller.__name__ = keyword
    caller.__doc__ = f"Set '{keyword}' ({stmt_type})."
    return caller


def _compound_caller(keyword, stmt_type, fields, config_ref):
    """Return a callable that appends a compound statement to the active block."""
    required = [f for f in fields if not f.startswith("!")]
    optional  = [f.lstrip("!") for f in fields if     f.startswith("!")]

    def caller(*args, **kwargs):
        value = {}
        for i, name in enumerate(required):
            if i < len(args):
                value[name] = args[i]
            elif name in kwargs:
                value[name] = kwargs[name]
            else:
                raise TypeError(
                    f"{keyword}() missing required argument: '{name}'"
                )
        for j, name in enumerate(optional):
            pos = len(required) + j
            if name in kwargs:
                value[name] = kwargs[name]
            elif pos < len(args):
                value[name] = args[pos]
        stmt = {"name": keyword, "statement_type": stmt_type, "value": value}
        config_ref._active_statements().append(stmt)

    req_sig  = ", ".join(required)
    opt_sig  = ", ".join(f"{n}=None" for n in optional)
    sig_parts = [p for p in [req_sig, opt_sig] if p]
    caller.__name__ = keyword
    caller.__doc__ = (
        f"Configure '{keyword}' ({stmt_type}).\n"
        f"Signature: {keyword}({', '.join(sig_parts)})\n"
        f"  required: {required}\n"
        f"  optional: {optional}"
    )
    return caller


# ---------------------------------------------------------------------------
# Condition builder
# ---------------------------------------------------------------------------

class _ConditionBuilder:
    """
    Fluent builder for a DSL condition token list.

    Returned by ``cfg.if_()``.  Each method appends tokens to an internal list
    and returns ``self``, so calls can be chained before the ``with`` statement.

    Boolean operators (matching DSL ``and`` / ``or`` / ``not``)::

        .and_()   -- appends "and"
        .or_()    -- appends "or"
        .not_()   -- appends "not"

    Predicate methods (each appends its keyword plus argument tokens)::

        .station(s)              -- "station" s
        .baseline(b)             -- "baseline" b
        .source(s)               -- "source" s
        .fgroup(fg)              -- "f_group" fg
        .scan_before(scan)       -- "scan" "<" scan
        .scan_after(scan)        -- "scan" ">" scan
        .scan_between(lo, hi)    -- "scan" lo "to" hi

    The token list is passed verbatim to ``MHO_ControlConditionEvaluator``, so
    the full DSL grammar (including ``not`` and parentheses via raw token
    injection) is available.
    """

    def __init__(self, config):
        self._config = config
        self._tokens: list = []
        self._block = None

    # ------------------------------------------------------------------
    # Boolean operator tokens
    # ------------------------------------------------------------------

    def and_(self) -> '_ConditionBuilder':
        """Append "and" to the condition token list."""
        self._tokens.append("and")
        return self

    def or_(self) -> '_ConditionBuilder':
        """Append "or" to the condition token list."""
        self._tokens.append("or")
        return self

    def not_(self) -> '_ConditionBuilder':
        """Append "not" to the condition token list."""
        self._tokens.append("not")
        return self

    # ------------------------------------------------------------------
    # Predicate tokens
    # ------------------------------------------------------------------

    def station(self, s: str) -> '_ConditionBuilder':
        """Append ``station <s>`` tokens."""
        self._tokens += ["station", s]
        return self

    def baseline(self, b: str) -> '_ConditionBuilder':
        """Append ``baseline <b>`` tokens."""
        self._tokens += ["baseline", b]
        return self

    def source(self, s: str) -> '_ConditionBuilder':
        """Append ``source <s>`` tokens."""
        self._tokens += ["source", s]
        return self

    def fgroup(self, fg: str) -> '_ConditionBuilder':
        """Append ``f_group <fg>`` tokens."""
        self._tokens += ["f_group", fg]
        return self

    def scan_before(self, scan: str) -> '_ConditionBuilder':
        """Append ``scan < <scan>`` tokens."""
        self._tokens += ["scan", "<", scan]
        return self

    def scan_after(self, scan: str) -> '_ConditionBuilder':
        """Append ``scan > <scan>`` tokens."""
        self._tokens += ["scan", ">", scan]
        return self

    def scan_between(self, lo: str, hi: str) -> '_ConditionBuilder':
        """Append ``scan <lo> to <hi>`` tokens."""
        self._tokens += ["scan", lo, "to", hi]
        return self

    # ------------------------------------------------------------------
    # Context manager protocol
    # ------------------------------------------------------------------

    def __enter__(self) -> '_ConditionBuilder':
        self._block = {"value": list(self._tokens), "statements": []}
        self._config._blocks.append(self._block)
        self._config._block_stack.append(self._block)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._config._block_stack.pop()
        return False  # do not suppress exceptions


# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------

class Config:
    """
    Accumulator for fringe-fitter control statements.

    Constructed by ``MHO_PyControlEvaluator`` and passed as the second argument
    to the user's ``configure()`` function.

    Each keyword from the control-file format (e.g. ``ref_freq``, ``pc_phases``,
    ``notches``, ...) is available as a method::

        cfg.ref_freq(215000.0)
        cfg.pc_mode("manual")
        cfg.pc_phases_l("abcde", [1.0, -2.0, 3.0, -4.0, 5.0])

    Use ``cfg.if_()`` to open a conditional block::

        with cfg.if_().station("G"):
            cfg.sampler_delay_x([-140, 180, 180, 180])

        with cfg.if_().source("3C279").and_().fgroup("X"):
            cfg.ref_freq(86000.0)

    Convenience shortcuts are provided for the common single-predicate case::

        with cfg.if_station("G"):       # same as cfg.if_().station("G")
            cfg.sampler_delay_x(...)

    Calling an unknown keyword raises :exc:`AttributeError`.
    Call :meth:`available_keywords` to see the full list.
    """

    def __init__(self, format_dict: dict):
        # Implicit global block - always-true, receives unconditional statements
        self._global_block = {"value": ["true"], "statements": []}
        # Ordered list of all blocks (global first, then conditionals in order)
        object.__setattr__(self, "_blocks", [self._global_block])
        # Stack; top is the currently active block (starts as global)
        object.__setattr__(self, "_block_stack", [self._global_block])
        object.__setattr__(self, "_format", format_dict)
        # Build per-keyword callables
        object.__setattr__(self, "_callers", {})
        self._build_callers(format_dict)

    # ------------------------------------------------------------------
    # Active-block access (used by caller closures)
    # ------------------------------------------------------------------

    def _active_statements(self) -> list:
        """Return the statement list of the currently active block."""
        return self._block_stack[-1]["statements"]

    # ------------------------------------------------------------------
    # Primary condition builder
    # ------------------------------------------------------------------

    def if_(self) -> _ConditionBuilder:
        """Return a fresh :class:`_ConditionBuilder` for this config."""
        return _ConditionBuilder(self)

    # ------------------------------------------------------------------
    # Convenience shortcuts (single-predicate common case)
    # ------------------------------------------------------------------

    def if_station(self, s: str) -> _ConditionBuilder:
        """Shortcut for ``cfg.if_().station(s)``."""
        return _ConditionBuilder(self).station(s)

    def if_baseline(self, b: str) -> _ConditionBuilder:
        """Shortcut for ``cfg.if_().baseline(b)``."""
        return _ConditionBuilder(self).baseline(b)

    def if_source(self, s: str) -> _ConditionBuilder:
        """Shortcut for ``cfg.if_().source(s)``."""
        return _ConditionBuilder(self).source(s)

    def if_fgroup(self, fg: str) -> _ConditionBuilder:
        """Shortcut for ``cfg.if_().fgroup(fg)``."""
        return _ConditionBuilder(self).fgroup(fg)

    def if_scan_before(self, scan: str) -> _ConditionBuilder:
        """Shortcut for ``cfg.if_().scan_before(scan)``."""
        return _ConditionBuilder(self).scan_before(scan)

    def if_scan_after(self, scan: str) -> _ConditionBuilder:
        """Shortcut for ``cfg.if_().scan_after(scan)``."""
        return _ConditionBuilder(self).scan_after(scan)

    def if_scan_between(self, lo: str, hi: str) -> _ConditionBuilder:
        """Shortcut for ``cfg.if_().scan_between(lo, hi)``."""
        return _ConditionBuilder(self).scan_between(lo, hi)

    # ------------------------------------------------------------------
    # Dynamic dispatch via __getattr__
    # ------------------------------------------------------------------

    def __getattr__(self, name: str):
        callers = object.__getattribute__(self, "_callers")
        if name in callers:
            return callers[name]
        raise AttributeError(
            f"'{type(self).__name__}' has no control keyword '{name}'. "
            f"Use available_keywords() to see valid options."
        )

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def available_keywords(self) -> list:
        """Return a sorted list of all usable keyword names."""
        return sorted(self._callers.keys())

    def to_json(self) -> list:
        """
        Serialise accumulated blocks into the conditional-block format
        expected by ``MHO_ParameterManager`` / ``MHO_OperatorBuilderManager``.

        Returns a list of conditional blocks in emission order.  Each block has
        the ``value`` array that preserves the original condition tokens so that
        downstream operator builders (e.g. ``MHO_StationDelayCorrectionBuilder``)
        can read ``fConditions["value"]`` to determine which station / baseline
        an operator applies to.
        """
        result = []
        for i, block in enumerate(self._blocks):
            if not block["statements"] and block["value"] != ["true"]:
                continue  # skip empty conditional blocks
            result.append({
                "name": "if",
                "value": block["value"],
                "statement_type": "conditional",
                "line_number": i,
                "statements": list(block["statements"]),
            })
        return result

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _build_callers(self, fmt: dict) -> None:
        callers = {}
        for keyword, spec in fmt.items():
            stype = spec.get("statement_type", "")
            vtype = spec.get("type", "")

            if stype in _SKIP_STYPES or vtype in ("deprecated", "unknown"):
                continue
            if stype not in _VALID_STYPES:
                continue

            if vtype == "compound":
                fields = spec.get("fields", [])
                callers[keyword] = _compound_caller(
                    keyword, stype, fields, self
                )
            elif vtype in _SCALAR_TYPES:
                callers[keyword] = _scalar_caller(
                    keyword, stype, self
                )

        object.__setattr__(self, "_callers", callers)

    def __repr__(self) -> str:
        n = sum(len(b["statements"]) for b in self._blocks)
        nb = len(self._blocks)
        return f"Config({n} statement{'s' if n != 1 else ''} in {nb} block{'s' if nb != 1 else ''})"
