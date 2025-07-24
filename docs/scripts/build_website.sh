#!/usr/bin/env bash
# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

set -eu
set -o pipefail

# --- Validate arguments -------------------------------------------------------
if [ "$#" -gt 1 ]; then
    echo "Usage: '$0' [docs_build_dir]" >&2
    exit 1
fi

# --- Locate sphinx-build ------------------------------------------------------
SPHINX_BUILD_EXECUTABLE_PATH="$(which sphinx-build 2> /dev/null || true)"
readonly SPHINX_BUILD_EXECUTABLE_PATH
if [ -z "${SPHINX_BUILD_EXECUTABLE_PATH}" ]; then
    echo "Executable 'sphinx-build' NOT found in PATH." >&2
    exit 1
fi
echo "Using executable '${SPHINX_BUILD_EXECUTABLE_PATH}'."
echo "         version '$("${SPHINX_BUILD_EXECUTABLE_PATH}" --version)'"

# --- Construct paths ----------------------------------------------------------
: "${DL_DOCS_DIR:?Environment variable DL_DOCS_DIR is not set}"

readonly DL_WEBSITE_DIR="${DL_DOCS_DIR}/deliteai.dev"

DL_DOCS_BUILD_DIR="${1:-"${DL_DOCS_DIR}/build"}"
mkdir -p "${DL_DOCS_BUILD_DIR}"
DL_DOCS_BUILD_DIR="$(realpath "${DL_DOCS_BUILD_DIR}")"
readonly DL_DOCS_BUILD_DIR

readonly DL_WEBSITE_BUILD_DIR="${DL_DOCS_BUILD_DIR}/deliteai.dev"

# --- Clean build directory ----------------------------------------------------
echo "Cleaning build dir '${DL_WEBSITE_BUILD_DIR}'"
rm -rf "${DL_WEBSITE_BUILD_DIR}"
mkdir -p "${DL_WEBSITE_BUILD_DIR}"

echo "Building DeliteAI HTML documentation using Sphinx"

# --- Build DelitePy Markdown documentation ------------------------------------
# TODO (jpuneet): Skip Markdown => HTML step here
"${DL_DELITEPY_DIR}/scripts/build_docs.sh"

# --- Build Android SDK Markdown documentation ---------------------------------

# TODO (jpuneet): run "dokkaGfm" Gradle task in the Android SDK

# --- Run Sphinx ---------------------------------------------------------------
# https://www.sphinx-doc.org/en/master/man/sphinx-build.html
"${SPHINX_BUILD_EXECUTABLE_PATH}"                                           \
    --builder html                                                          \
    --doctree-dir "${DL_WEBSITE_BUILD_DIR}/html_intermediate/.doctrees"     \
    --conf-dir "${DL_WEBSITE_DIR}"                                          \
    --nitpicky                                                              \
    "${DL_GIT_ROOT}"                                                        \
    "${DL_WEBSITE_BUILD_DIR}/html"                                          \
    ;
# TODO (jpuneet): Enable the option `--fail-on-warning` once all the existing warnings get fixed.

echo "[done] Building DeliteAI HTML documentation using Sphinx"
echo "DeliteAI HTML documentation: '${DL_WEBSITE_BUILD_DIR}/html/index.html'"
