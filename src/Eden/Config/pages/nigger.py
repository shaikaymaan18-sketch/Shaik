#!/usr/bin/env python3

import re
import sys
import shutil
from pathlib import Path

# Pattern to match the apply() function block
APPLY_PATTERN = re.compile(
    r'^(\s*)function apply\(\) \{\s*\n'          # opening line
    r'((?:^\1\s{4}.*\n)+)'                       # body: lines indented further
    r'^\1\}',                                   # closing line
    re.MULTILINE
)

def process_file(file_path: Path) -> bool:
    """Process a single file. Returns True if the file was modified."""
    content = file_path.read_text(encoding='utf-8')

    def repl(match):
        base_indent = match.group(1)
        body_block = match.group(2)

        # Extract identifiers from each body line
        identifiers = []
        for line in body_block.splitlines():
            stripped = line.strip()
            if stripped and '.' in stripped:
                identifier = stripped.split('.')[0].strip()
                if identifier:
                    identifiers.append(identifier)

        if not identifiers:
            return match.group(0)

        # Build sync() function with consistent 4-space indentation
        sync_lines = []
        sync_lines.append(f"{base_indent}function sync() {{")
        for ident in identifiers:
            sync_lines.append(f"{base_indent}    {ident}.sync()")
        sync_lines.append(f"{base_indent}}}")

        sync_block = "\n".join(sync_lines) + "\n"

        # Insert after the original apply() block
        return match.group(0) + "\n" + sync_block

    new_content, count = APPLY_PATTERN.subn(repl, content)

    if count > 0:
        # Create backup
        backup_path = file_path.with_suffix(file_path.suffix + '.bak')
        shutil.copy2(file_path, backup_path)
        print(f"Backup created: {backup_path}")

        # Write new content
        file_path.write_text(new_content, encoding='utf-8')
        print(f"Modified: {file_path}")
        return True

    return False

def main(target_dir: Path = Path('.')):
    qml_files = sorted(target_dir.rglob('*.qml'))

    if not qml_files:
        print("No *.qml files found in the specified directory.")
        return

    changed_count = 0

    print(f"Processing {len(qml_files)} QML file(s) in: {target_dir.resolve()}\n")

    for file_path in qml_files:
        if process_file(file_path):
            changed_count += 1

    print(f"\nDone. {changed_count} file(s) modified.")
    if changed_count > 0:
        print("Backups have been saved with the '.bak' extension.")
        print("You can restore a file with: mv file.qml.bak file.qml")

if __name__ == "__main__":
    dir_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('.')
    if not dir_path.is_dir():
        print(f"Error: '{dir_path}' is not a valid directory.")
        sys.exit(1)

    main(dir_path)
