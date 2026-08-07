"""Example: a custom fourfit fringe-plot plugin that *wraps* the stock renderer.

fourfit's default fringe plot is produced by
``hops_visualization.fourfit_plot``. To customize it you do **not** need to copy
that ~1000-line module -- import the stock renderer and override only the pieces
you want to change. This keeps your plugin tiny and, more importantly, means it
automatically inherits every future fix and layout change to the default plot
instead of silently drifting from it.

How fourfit calls this
----------------------
Put this file where the embedded interpreter can import it -- the directory in
``$HOPS_USER_PLUGINS_DIR`` (or the default plugin dir), which HOPS appends to
``sys.path`` at startup -- then name it in the control file::

    plot_backend         matplotlib
    python_custom_plot   custom_fourfit_plot   make_custom_fourfit_plot_wrapper

The ``python_custom_plot <module_path> <function_name>`` statement tells
MHO_DefaultPythonPlotVisitor to import ``module_path`` (dot-separated) and call
``function_name(fringe_data_interface)`` instead of the default
``hops_visualization.fourfit_plot:make_fourfit_plot_wrapper``. It only takes
effect with ``plot_backend matplotlib``, and only in a build with embedded
Python (HOPS_ENABLE_EMBEDDED_PYTHON=ON); the libpython-free subprocess backend
cannot run custom plots and renders the default plot instead.

The customization below is deliberately trivial (a small footer annotation) so
the *pattern* is obvious: keep a handle to the stock helper, wrap it, and
reinstall it on the imported module so ``make_fourfit_plot()`` picks it up.
"""

from hops_visualization import fourfit_plot

# Keep a reference to the stock helper we want to extend. All of the panel /
# table / text builders live on the fourfit_plot module namespace (imported
# there from fourfit_plot_common), and make_fourfit_plot() calls them by name,
# so reassigning the module attribute below is enough to take effect.
_stock_make_top_info_text = fourfit_plot.make_top_info_text


def _custom_make_top_info_text(plot_dict):
    """Draw the stock top-of-page info, then add a small custom footer."""
    _stock_make_top_info_text(plot_dict)

    from matplotlib import pyplot as plt
    plt.text(0.5, 0.004, "rendered by custom_fourfit_plot.py (example plugin)",
             transform=plt.gcf().transFigure, fontsize=6, color="0.5",
             horizontalalignment='center', verticalalignment='bottom')


# Install the override so the stock make_fourfit_plot() uses it.
fourfit_plot.make_top_info_text = _custom_make_top_info_text


def make_custom_fourfit_plot_wrapper(fringe_data_interface):
    """Entry point fourfit calls (see module docstring).

    Deliberately named differently from the stock
    ``fourfit_plot.make_fourfit_plot_wrapper`` so it is clear which one the
    control file selects. We simply delegate to the stock wrapper; the
    customization is already installed above. Override more helpers the same
    way, or pre/post-process ``fringe_data_interface.get_plot_data()`` here
    before delegating.
    """
    fourfit_plot.make_fourfit_plot_wrapper(fringe_data_interface)
