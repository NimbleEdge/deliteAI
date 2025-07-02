#!/usr/bin/env python3
# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import ast
import json
import os
import sys

import ast2json


def gen_python_ast(infile_path: str) -> None:
    with open(infile_path, "r") as file:
        infile_content = file.read()

    tree = ast2json.ast2json(ast.parse(infile_content))

    # import pprint
    # pprint.pprint(ast.dump(tree))

    outfile_content = json.dumps(tree, indent=2)

    infile_dirname = os.path.dirname(infile_path)
    infile_basename = os.path.basename(infile_path)
    outfile_basename = f"{infile_basename.split('.')[0]}.ast"
    outfile_path = os.path.join(infile_dirname, outfile_basename)

    with open(outfile_path, "w") as file:
        file.write(outfile_content)


if __name__ == "__main__":
    gen_python_ast(sys.argv[1])
