import os
import sys
import subprocess
import argparse
from pathlib import Path
from dotenv import load_dotenv

# Load environment variables (e.g., DROPBOX_ACCESS_TOKEN)
load_dotenv()

# Configuration
PROJECT_ROOT = Path(__file__).parent.parent.resolve()
INSTALLER_EXE = PROJECT_ROOT / "WeightOfTheSkyInstaller" / "INSTALLER" / "WeightOfTheSky_Setup.exe"

def run(command, check=True):
    """Utility to run a shell command and return the result."""
    print(f"> {' '.join(command)}")
    return subprocess.run(command, check=check, capture_output=True, text=True)

def upload_github(tag: str, repo: str | None = None) -> None:
    """Uploads the installer to a GitHub release."""
    repo_flags = ["--repo", repo] if repo else []
    
    print("\n=== Uploading to GitHub Releases ===")
    print(f"Tag: {tag}")
    print(f"Installer: {INSTALLER_EXE}")

    if not INSTALLER_EXE.exists():
        print(f"ERROR: Installer not found at {INSTALLER_EXE}!")
        sys.exit(1)

    # Create the release if it doesn't exist
    result = run(["gh", "release", "view", tag, *repo_flags], check=False)
    if result.returncode != 0:
        print(f"Creating release {tag}...")
        run(["gh", "release", "create", tag, *repo_flags, "--title", tag, "--notes", f"Installer for {tag}"])
    
    # Upload the installer (--clobber overwrites if asset already exists)
    print("Uploading installer...")
    run(["gh", "release", "upload", tag, str(INSTALLER_EXE), *repo_flags, "--clobber"])

    print("\n=== SUCCESS ===")
    # Print the download URL
    url_result = run(["gh", "release", "view", tag, *repo_flags, "--json", "assets", "--jq", ".assets[].url"])
    print(f"Download URL:\n{url_result.stdout.strip()}")

def upload_dropbox(folder_path: str = "/") -> None:
    """Uploads the installer to Dropbox using the Dropbox API."""
    import dropbox
    from dropbox.files import WriteMode

    dbx_token = os.getenv("DROPBOX_ACCESS_TOKEN")
    if not dbx_token:
        print("ERROR: DROPBOX_ACCESS_TOKEN not set in environment!")
        return

    print("\n=== Uploading to Dropbox ===")
    
    with dropbox.Dropbox(dbx_token) as dbx:
        with open(INSTALLER_EXE, "rb") as f:
            file_name = INSTALLER_EXE.name
            target_path = f"{folder_path}{file_name}".replace("//", "/")
            print(f"Folder: {folder_path}")
            print(f"File: {file_name}")
            print(f"Uploading installer...")
            
            dbx.files_upload(f.read(), target_path, mode=WriteMode.overwrite)
            print("\n=== SUCCESS ===")
            print(f"Uploaded to Dropbox: {target_path}")

def main():
    parser = argparse.ArgumentParser(description="Upload WeightOfTheSky Installer")
    parser.add_argument("destination", choices=["github", "dropbox"], help="Upload destination")
    parser.add_argument("tag", nargs="?", default="v0.0.1", help="Release tag (required for GitHub)")
    parser.add_argument("--repo", help="Optional: Specify GitHub repository (owner/repo)")
    
    args = parser.parse_args()

    if args.destination == "github":
        upload_github(args.tag, args.repo)
    elif args.destination == "dropbox":
        upload_dropbox()

if __name__ == "__main__":
    main()
