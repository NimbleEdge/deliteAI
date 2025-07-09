# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys

sys.path.insert(0, f"{os.environ['DL_DELITEPY_DIR']}/library_stubs/src_gen")

# -- Project information -----------------------------------------------------

project = "DeliteAI"
copyright = "2025, DeliteAI Authors"
author = "DeliteAI Authors"
release = "0.1.0-dev"               # TODO (jpuneet): read from "config.yml"?

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

exclude_patterns = [
    "**/android/nimblenet_core/src/main/cpp/coreruntime",
    "**/android/nimblenet_core/src/main/cpp/onnx_builds",
    "**/android/nimblenet_core/src/main/cpp/third_party",
]

# -- Options for HTML output -------------------------------------------------

html_theme = "furo"
html_logo = "_static/images/delite-ai-blue-logo.png"    # TODO (jpuneet): resize to width=200px?
# html_favicon = "_static/images/favicon.png"
html_static_path = ["_static"]
