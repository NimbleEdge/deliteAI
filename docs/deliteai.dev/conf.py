# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import re
import shutil
import sys
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    import sphinx.application

sys.path.insert(0, f"{os.environ['DL_DELITEPY_DIR']}/library_stubs/src_gen")

# -- Project information -----------------------------------------------------

project = "DeliteAI"
copyright = "2025, DeliteAI Authors"
author = "DeliteAI Authors"
release = "0.1.0-dev"  # TODO (jpuneet): read from "config.yml"?

# -- General configuration ---------------------------------------------------

needs_sphinx = "8.2.3"

extensions = [
    "myst_parser",
    "sphinx.ext.autodoc",
]

autodoc_default_options = {
    "special-members": "__init__",
}

myst_heading_anchors = 3

include_patterns = [
    "*.md",
    "coreruntime/**",
    "mockserver/**",
    "sdks/**",
    "agents/**",
]

root_doc = "README"

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

exclude_patterns = [
    "coreruntime/delitepy/docs_template",
    "coreruntime/delitepy/library_stubs/src_template",
    "sdks/android/nimblenet_core/src/main/cpp",
]

# TODO (jpuneet): Figure out a way to suppress these warnings selectively for
# just the dokkaGfm output.
suppress_warnings = [
    "myst.header",
    "toc.not_included",
]

# -- Options for HTML output -------------------------------------------------

html_theme = "furo"
html_logo = "_static/images/delite-ai-blue-logo.png"  # TODO (jpuneet): resize to width=200px?
# html_favicon = "_static/images/favicon.png"
html_static_path = ["_static"]

# -- Set up the 'conf.py' Sphinx extension -----------------------------------


class _ExtensionUtils:
    @staticmethod
    def patch_html_builder() -> None:
        from sphinx.builders.html import StandaloneHTMLBuilder

        def rename_doc(docname: str) -> str:
            path = Path(docname)
            if path.name == "README":
                path = path.with_name("index")
            return str(path)

        get_target_uri_orig = StandaloneHTMLBuilder.get_target_uri

        def get_target_uri(self, docname: str, typ: str | None = None) -> str:
            docname = rename_doc(docname)
            return get_target_uri_orig(self, docname, typ)

        StandaloneHTMLBuilder.get_target_uri = get_target_uri

    _pattern_doc_blocks_for_sphinx = re.compile(
        r"^ *<!-- +DL_DOCS::WEBSITE_BLOCK_START\r?\n"
        r"([\s\S]*?)"
        r"^ *DL_DOCS::WEBSITE_BLOCK_END +-->\r?\n",
        re.MULTILINE,
    )

    @staticmethod
    def _enable_website_blocks(doc_content: str) -> str:
        """
        Transforms
        ```
        <!-- DL_DOCS::WEBSITE_BLOCK_START
        ...
        DL_DOCS::WEBSITE_BLOCK_END -->
        ```
        into
        ```
        ...
        ```
        """
        return _ExtensionUtils._pattern_doc_blocks_for_sphinx.sub(
            r"\1",
            doc_content,
        )

    _pattern_doc_blocks_not_for_sphinx = re.compile(
        r"^ *<!-- +DL_DOCS::GITHUB_BLOCK_START +-->\r?\n"
        r"[\s\S]*?"
        r"^ *<!-- +DL_DOCS::GITHUB_BLOCK_END +-->\r?\n",
        re.MULTILINE,
    )

    @staticmethod
    def _disable_github_blocks(doc_content: str) -> str:
        """
        Removes the pattern
        ```
        <!-- DL_DOCS::GITHUB_BLOCK_START -->
        ...
        <!-- DL_DOCS::GITHUB_BLOCK_END -->
        ```
        """
        return _ExtensionUtils._pattern_doc_blocks_not_for_sphinx.sub(
            r"",
            doc_content,
        )

    @staticmethod
    def on_source_read(app: "sphinx.application.Sphinx", docname: str, source: list[str]) -> None:
        doc_content = source[0]

        doc_content_modified = doc_content
        doc_content_modified = _ExtensionUtils._enable_website_blocks(doc_content_modified)
        doc_content_modified = _ExtensionUtils._disable_github_blocks(doc_content_modified)

        source[0] = doc_content_modified

    @staticmethod
    def _rename_readme_html_to_index_html(outdir: Path) -> None:
        """Renames all README.html files to index.html inside `outdir`"""
        for root, _, files in outdir.walk():
            if "README.html" in files:
                shutil.move(root / "README.html", root / "index.html")

    @staticmethod
    def on_build_finished(app: "sphinx.application.Sphinx", exception: Exception | None) -> None:
        if exception:
            return

        # Patch StandaloneHTMLBuilder.get_output_path instead?
        _ExtensionUtils._rename_readme_html_to_index_html(app.outdir)


def setup(app: "sphinx.application.Sphinx") -> None:
    _ExtensionUtils.patch_html_builder()

    app.connect("source-read", _ExtensionUtils.on_source_read)
    app.connect("build-finished", _ExtensionUtils.on_build_finished)
