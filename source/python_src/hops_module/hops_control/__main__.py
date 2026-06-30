"""
hops_control subprocess entry point.

This is the Python half of the no-embed (subprocess) control-file backend. The
C++ side (MHO_SubprocessPyControlEvaluator + MHO_PythonSubprocessRunner) invokes
it as::

    python -m hops_control <request_file> <response_file>

The C++ side pre-creates both temp files: it writes the request into the first
and reads the JSON response back from the second. Returning the contract through
its own file (rather than stdout) means the exec'd user control script is free
to print() without corrupting what the C++ side parses. It mirrors what the
embedded MHO_PyControlEvaluator does in-process: build a PassInfo + Config, exec
the user's control script to obtain configure(), call configure(pass_info,
config), and return config.to_json(). The condition filtering + command-line
set-string overrides are applied on the C++ side (shared
MHO_ControlEvaluatorSupport), so this entry point returns just the RAW statement
list.

JSON contract (keep in sync with MHO_PythonSubprocessContract.hh):
    request  : { schema_version, pass_info, config, script_path }
    response : { schema_version, ok, statements, error }
"""

import json
import sys

from hops_control import PassInfo, Config

# bump together with kSchemaVersion in MHO_PythonSubprocessContract.hh
SCHEMA_VERSION = 2


def _read_request(path):
    """Load the request dict from the given file path (or stdin if '-')."""
    if path == "-":
        return json.load(sys.stdin)
    with open(path, "r") as f:
        return json.load(f)


def _load_configure(script_path):
    """Exec the user's control script and return its configure() callable."""
    with open(script_path, "r") as f:
        code = compile(f.read(), script_path, "exec")
    ns = {"__name__": "__hops_control__", "__file__": script_path}
    exec(code, ns)
    configure = ns.get("configure")
    if configure is None:
        raise ValueError(
            "Python control file '%s' does not define a 'configure' function." % script_path
        )
    return configure


def _write_response(path, response):
    """Write the JSON response into the response file the C++ caller reads."""
    with open(path, "w") as f:
        json.dump(response, f)


def main(argv):
    # The C++ caller passes two temp-file paths: the request to read and the
    # response file to write into. The response goes through its own file, so the
    # exec'd user control script / configure() call cannot corrupt the contract.
    if len(argv) < 3:
        sys.stderr.write(
            "hops_control: usage: python -m hops_control <request_file> <response_file>\n"
        )
        return 2
    request_path = argv[1]
    response_path = argv[2]
    try:
        request = _read_request(request_path)

        req_version = request.get("schema_version")
        if req_version != SCHEMA_VERSION:
            raise ValueError(
                "control request schema_version %r != expected %d" % (req_version, SCHEMA_VERSION)
            )

        pass_info_dict = request.get("pass_info", {})
        control_format = request.get("config", {})
        script_path = request.get("script_path")
        if not script_path:
            raise ValueError("request is missing 'script_path'")

        configure = _load_configure(script_path)

        pass_info = PassInfo(pass_info_dict)
        config = Config(control_format)
        configure(pass_info, config)
        statements = config.to_json()

        _write_response(response_path, {"schema_version": SCHEMA_VERSION, "ok": True, "statements": statements})
        return 0
    except Exception as exc:  # noqa: BLE001 - surface any failure as a contract error
        _write_response(response_path, {"schema_version": SCHEMA_VERSION, "ok": False, "error": "%s" % exc})
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
