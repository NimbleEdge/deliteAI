#!/usr/bin/env python3
# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import ast
import json
import os
import sys
import zipfile
import ast2json


def gen_python_ast(input_file: str) -> None:
    if input_file.endswith(".zip"):
        zip_file = zipfile.ZipFile(input_file)

        # Prepare the super JSON object
        super_json = {}
        main = False

        # Process each file in the archive
        for name in zip_file.namelist():
            if name.endswith('/'):
                raise ValueError(f"Directories are not allowed in zip file: {name}")

            if name == 'main.py':
                main = True
            if not name.endswith('.py'):
                raise ValueError(f"Unsupported file type: {name}. Only .py files are allowed.")

            # Read file content
            with zip_file.open(name) as file:
                content = file.read().decode('utf-8')

            # Parse Python source to AST
            tree = ast.parse(content, filename=name)

            # Convert AST to JSON
            json_ast = ast2json.ast2json(tree)

            # Use base fileName without extension as key
            base_name = os.path.splitext(os.path.basename(name))[0]
            super_json[base_name] = json_ast

        if not main:
            raise ValueError("main.py file is required in the zip archive.")
        # Output final JSON object
        parsedAST = json.dumps(super_json)
    else:
        f = open(input_file, 'r')
        tree = ast2json.ast2json(ast.parse(f.read()))
        parsedAST = json.dumps(tree, indent=2)

    base_name = os.path.basename(input_file)
    output_file = os.path.join(os.path.dirname(input_file), base_name.split('.')[0] + ".ast")

    with open(output_file,'w') as file:
        file.write(parsedAST)

if __name__ == "__main__":
    gen_python_ast(sys.argv[1])
