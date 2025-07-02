import ast
import ast2json
import pprint
import json
import sys
import os
f =open(sys.argv[1],'r')

tree = ast2json.ast2json(ast.parse(f.read()))
input_file=sys.argv[1]
base_name = os.path.basename(input_file)
output_file = os.path.join(os.path.dirname(input_file), base_name.split('.')[0] + ".ast")

with open(output_file,'w') as file:
    file.write(json.dumps(tree, indent=2))

#pprint.pprint(ast.dump(tree))
