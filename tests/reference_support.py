from pathlib import Path

MOD_ROOT = Path(__file__).resolve().parents[1]
DEVKIT = MOD_ROOT / 'devkit'
if not (DEVKIT / 'scripts/native-probe.py').is_file():
    raise RuntimeError('Initialize the pinned devkit: git submodule update --init --recursive')
