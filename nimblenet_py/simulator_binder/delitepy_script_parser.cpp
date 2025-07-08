/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pybind11/embed.h>

#include <string>

namespace py = pybind11;
using namespace py::literals;

std::string parseScriptToAST(const std::string& scriptPath) {
  auto localVars = py::dict("fileName"_a = scriptPath.c_str());
  py::exec(R"(
        if fileName.endswith(".zip"):
          import ast
          import ast2json
          import json
          import sys
          import io
          import zipfile
          import os
  
          zip_file = zipfile.ZipFile(fileName)
  
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
          import ast
          import ast2json
          import json
          f = open(fileName, 'r')
          tree = ast2json.ast2json(ast.parse(f.read()))
          parsedAST = json.dumps(tree, indent=2)
    )",
           py::dict(), localVars);
  return localVars["parsedAST"].cast<std::string>();
}