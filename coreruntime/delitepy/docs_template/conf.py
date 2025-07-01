# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import subprocess
import sys


def get_git_root() -> str:
    return subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    ).stdout.removesuffix("\n")


sys.path.insert(0, f"{get_git_root()}/coreruntime/delitepy/library_stubs/src_gen")

# -- Project information -----------------------------------------------------

project = "DelitePy"
copyright = "2025, DeliteAI Authors"
author = "DeliteAI Authors"
release = "0.1"

# -- General configuration ---------------------------------------------------

needs_sphinx = "8.2.3"

extensions = [
    "myst_parser",
    "sphinx.ext.autodoc",
]

autodoc_default_options = {
    "special-members": "__init__",
}

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

exclude_patterns = []

# -- Options for HTML output -------------------------------------------------

html_theme = "furo"
