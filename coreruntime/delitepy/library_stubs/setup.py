# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import subprocess

import setuptools


def get_git_root() -> str:
    return subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        check=True,
        stdout=subprocess.PIPE,
        text=True,
    ).stdout.removesuffix("\n")


def render_src_template() -> None:
    coreruntime_dir = f"{get_git_root()}/coreruntime"
    delitepy_dir = f"{coreruntime_dir}/delitepy"
    library_stubs_dir = f"{delitepy_dir}/library_stubs"

    subprocess.run(
        ["rm", "-rf", f"{library_stubs_dir}/src_gen"],
        check=True,
    )
    subprocess.run(
        ["cp", "-r", f"{library_stubs_dir}/src_template", f"{library_stubs_dir}/src_gen"],
        check=True,
    )


render_src_template()

setuptools.setup()
