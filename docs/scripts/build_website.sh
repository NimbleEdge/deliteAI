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
DL_DOCS_BUILD_DIR="${1:-"${DL_DOCS_DIR}/build"}"
mkdir -p "${DL_DOCS_BUILD_DIR}"
DL_DOCS_BUILD_DIR="$(realpath "${DL_DOCS_BUILD_DIR}")"
readonly DL_DOCS_BUILD_DIR

readonly DL_WEBSITE_BUILD_DIR="${DL_DOCS_BUILD_DIR}/deliteai.dev"

# --- Clean build directory ----------------------------------------------------
echo "Removing build dir '${DL_WEBSITE_BUILD_DIR}'"
rm -rf "${DL_WEBSITE_BUILD_DIR}"
mkdir -p "${DL_WEBSITE_BUILD_DIR}"

echo "Building DeliteAI HTML documentation using Sphinx"

# --- Copy website directory ---------------------------------------------------
readonly DL_WEBSITE_DIR="${DL_DOCS_DIR}/deliteai.dev"

cp -R                                                                       \
    "${DL_WEBSITE_DIR}"                                                     \
    "${DL_WEBSITE_BUILD_DIR}/gen"                                           \
    ;

# --- Build DelitePy Markdown documentation ------------------------------------
readonly DL_DELITEPY_DOCS_BUILD_DIR="${DL_WEBSITE_BUILD_DIR}/gen/delitepy"

# TODO (jpuneet): Skip Markdown => HTML step here
"${DL_DELITEPY_DIR}/scripts/build_docs.sh" "${DL_DELITEPY_DOCS_BUILD_DIR}"

# --- Build Android SDK Markdown documentation ---------------------------------

# TODO (jpuneet)

# --- Run Sphinx ---------------------------------------------------------------
"${SPHINX_BUILD_EXECUTABLE_PATH}"                                           \
    --builder html                                                          \
    "${DL_WEBSITE_BUILD_DIR}/gen"                                           \
    "${DL_WEBSITE_BUILD_DIR}/html"                                          \
    ;

echo "[done] Building DeliteAI HTML documentation using Sphinx"
echo "DeliteAI HTML documentation: '${DL_WEBSITE_BUILD_DIR}/html/index.html'"
