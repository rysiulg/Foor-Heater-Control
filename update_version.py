import re
import yaml
import sys

# Define filenames as variables
CHANGELOG_FILE = 'CHANGELOG.md'
YAML_FILE = 'heatpump.yaml'

# Get the version from command-line arguments
new_version = sys.argv[1]

# Update the YAML file
with open(YAML_FILE, 'r') as yaml_file:
    yaml_content = yaml.safe_load(yaml_file)

yaml_content['esphome']['project']['version'] = new_version

with open(YAML_FILE, 'w') as yaml_file:
    yaml.safe_dump(yaml_content, yaml_file)

print(f'Updated version to {new_version} in {YAML_FILE}')

# Replace unreleased section in CHANGELOG.md
with open(CHANGELOG_FILE, 'r') as f:
    changelog = f.read()

updated_changelog = re.sub(r'\[Unreleased\](.*?)\n##', f'[{new_version}]\\1\n##', changelog, flags=re.DOTALL)

with open(CHANGELOG_FILE, 'w') as f:
    f.write(updated_changelog)

print(f'Replaced [Unreleased] with [{new_version}] in {CHANGELOG_FILE}')
