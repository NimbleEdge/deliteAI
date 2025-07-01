#!/usr/bin/env python3

import os
import sys
from pathlib import Path

from jinja2 import Template


def extract_delitepy_doc_blocks(infile_path: str):
    block_begin_marker = "DELITEPY_DOC_BLOCK_BEGIN"
    block_end_marker = "DELITEPY_DOC_BLOCK_END"

    inside_doc_block = False

    with open(infile_path, "r") as infile:
        for line in infile:
            stripped_line = line.strip()

            if stripped_line.startswith(block_begin_marker):
                inside_doc_block = True
                continue

            if stripped_line.startswith(block_end_marker):
                inside_doc_block = False
                continue

            if inside_doc_block:
                yield line


def render_jinja2_templates(source_dir: str, target_dir: str, base_dir: str) -> None:
    for root, _, file_names in os.walk(source_dir):
        root_rel_path = os.path.relpath(root, source_dir)
        root_out_dir = os.path.join(target_dir, root_rel_path)
        os.makedirs(root_out_dir, exist_ok=True)

        for file_name in file_names:
            template_path = os.path.join(root, file_name)
            with open(template_path, "r") as file:
                template_content = file.read()

            template = Template(template_content, keep_trailing_newline=True)
            render_context = {
                "extract_delitepy_doc_blocks": lambda infile_path: "".join(
                    extract_delitepy_doc_blocks(
                        str(Path(base_dir).joinpath(infile_path).resolve()),
                    ),
                ),
            }
            rendered_template_content = template.render(render_context)

            rendered_template_path = os.path.join(root_out_dir, file_name)
            with open(rendered_template_path, "w") as file:
                file.write(rendered_template_content)


def main(args: list[str]) -> None:
    assert len(args) == 4, "Incorrect usage."

    source_dir = str(Path(args[1]).resolve())
    target_dir = str(Path(args[2]).resolve())
    base_dir = str(Path(args[3]).resolve())

    print(f"Rendering Jinja2 templates: '{source_dir}' => '{target_dir}'")
    render_jinja2_templates(source_dir, target_dir, base_dir)
    print(f"[done] Rendering Jinja2 templates: '{source_dir}' => '{target_dir}'")


if __name__ == "__main__":
    main(sys.argv)
