#!/usr/bin/env bash

set -eu
set -o pipefail

# --- Validate arguments -------------------------------------------------------
if [ "$#" -gt 1 ]; then
    echo "Usage: '$0' [delitepy_build_dir]" >&2
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
DL_CORERUNTIME_DIR="$(git rev-parse --show-toplevel)/coreruntime"
readonly DL_CORERUNTIME_DIR

readonly DL_DELITEPY_DIR="${DL_CORERUNTIME_DIR}/delitepy"

DL_DELITEPY_BUILD_DIR="${1:-"${DL_DELITEPY_DIR}/build"}"
mkdir -p "${DL_DELITEPY_BUILD_DIR}"
DL_DELITEPY_BUILD_DIR="$(realpath "${DL_DELITEPY_BUILD_DIR}")"
readonly DL_DELITEPY_BUILD_DIR

readonly DL_DELITEPY_DOCS_BUILD_DIR="${DL_DELITEPY_BUILD_DIR}/docs"

# --- Clean build directory ----------------------------------------------------
echo "Removing build dir '${DL_DELITEPY_DOCS_BUILD_DIR}'"
rm -rf "${DL_DELITEPY_DOCS_BUILD_DIR}"

echo "Building DelitePy HTML documentation using Sphinx"

# --- Render docs_template -----------------------------------------------------
"${DL_DELITEPY_DIR}/scripts/render_jinja2_templates.py"                     \
    "${DL_DELITEPY_DIR}/docs_template"                                      \
    "${DL_DELITEPY_DOCS_BUILD_DIR}/gen/docs"                                \
    "${DL_CORERUNTIME_DIR}"                                                 \
    ;

# --- Render library_stubs/src_template ----------------------------------------
rm -rf "${DL_DELITEPY_DIR}/library_stubs/src_gen"
"${DL_DELITEPY_DIR}/scripts/render_jinja2_templates.py"                     \
    "${DL_DELITEPY_DIR}/library_stubs/src_template"                         \
    "${DL_DELITEPY_DIR}/library_stubs/src_gen"                              \
    "${DL_CORERUNTIME_DIR}"                                                 \
    ;

# --- Run Sphinx ---------------------------------------------------------------
sphinx-apidoc                                                               \
    -o "${DL_DELITEPY_DOCS_BUILD_DIR}/gen/docs/library/delitepy"            \
    "${DL_DELITEPY_DIR}/library_stubs/src_gen/delitepy"                     \
    ;

"${SPHINX_BUILD_EXECUTABLE_PATH}"                                           \
    --builder html                                                          \
    "${DL_DELITEPY_DOCS_BUILD_DIR}/gen/docs"                                \
    "${DL_DELITEPY_DOCS_BUILD_DIR}/html"                                    \
    ;

echo "[done] Building DelitePy HTML documentation using Sphinx"
echo "DelitePy HTML documentation: '${DL_DELITEPY_DOCS_BUILD_DIR}/html/index.html'"
