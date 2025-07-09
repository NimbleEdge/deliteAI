#!/usr/bin/env python3
# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import os
import subprocess
from pathlib import Path


def get_git_root() -> Path:
    res = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        check=True,
        cwd=Path(__file__).resolve().parent,
        stdout=subprocess.PIPE,
        text=True,
    )
    git_root = res.stdout.removesuffix("\n")
    return Path(git_root)


GIT_ROOT = get_git_root()
os.environ["DL_GIT_ROOT"] = str(GIT_ROOT)

CORERUNTIME_DIR = GIT_ROOT / "coreruntime"
os.environ["DL_CORERUNTIME_DIR"] = str(CORERUNTIME_DIR)

DELITEPY_DIR = CORERUNTIME_DIR / "delitepy"
os.environ["DL_DELITEPY_DIR"] = str(DELITEPY_DIR)

DOCS_DIR = GIT_ROOT / "docs"
os.environ["DL_DOCS_DIR"] = str(DOCS_DIR)

SDK_ANDROID_DIR = GIT_ROOT / "sdks" / "android"
os.environ["DL_SDK_ANDROID_DIR"] = str(SDK_ANDROID_DIR)

SDK_IOS_DIR = GIT_ROOT / "sdks" / "ios"
os.environ["DL_SDK_IOS_DIR"] = str(SDK_IOS_DIR)


def discover_commands() -> dict:
    commands = {}

    scripts_dir = DOCS_DIR / "scripts"
    for path in scripts_dir.glob("*"):
        if path.is_file() and os.access(path, os.X_OK):
            cmd_name = path.stem
            commands[cmd_name] = path

    del commands["run"]
    return commands


def run_command(script_path: Path, script_args: list[str]) -> None:
    subprocess.run(
        [str(script_path)] + script_args,
        check=True,
    )


def main():
    commands = discover_commands()

    parser = argparse.ArgumentParser(description="Script runner")
    parser.add_argument("command", choices=commands.keys(), help="command to run")
    parser.add_argument("args", nargs=argparse.REMAINDER, help="arguments to pass to the command")
    args = parser.parse_args()

    assert args.command in commands, "Invalid command."
    script_path = commands[args.command]
    script_args = args.args
    run_command(script_path, script_args)


if __name__ == "__main__":
    main()
