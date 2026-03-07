#!/bin/bash
# Update script for vibe-journal
# Download latest release from GitHub
curl -s https://api.github.com/repos/noblebrown-69/vibe-journal/releases/latest | grep 'browser_download_url.*vibe-journal' | cut -d '"' -f 4 | xargs curl -L -o vibe-journal.new
chmod +x vibe-journal.new
mv vibe-journal.new vibe-journal
echo 'Updated to latest version.'

