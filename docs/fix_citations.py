import sys
import re

def fix_citations(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    content = re.sub(r'\{\[}@([^{}]+?)\{\]\}', r'\\cite{\1}', content)

    with open(file_path, "w", encoding="utf-8") as f:
        f.write(content)

if __name__ == "__main__":
    fix_citations(sys.argv[1])
