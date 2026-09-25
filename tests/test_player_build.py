"""Game-free build/installer integration; uses only a synthetic routing archive."""
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from zipfile import ZipFile

from reference_support import MOD_ROOT

SPEC = importlib.util.spec_from_file_location('player_build', MOD_ROOT / 'scripts/build-player.py')
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)


class PlayerBuildTests(unittest.TestCase):
    def test_native_selector_rejects_wrong_bytes_and_preserves_memory_protection(self):
        with tempfile.TemporaryDirectory(prefix='h5-bank-native-') as temporary:
            root = Path(temporary)
            package = root / 'fixture.h5u'
            routes = [{'title': 'Test Bank', 'family': 'test', 'window_id': 'TestReference'}]
            with ZipFile(package, 'w') as archive:
                archive.writestr('UI/WorkshopArmy/routes.json', json.dumps(routes))
            result = builder.generate(package, root / 'generated/bank_payload.hpp')
            self.assertEqual(result['routes'], 1)
            # Native CTest calls the production installer against its own memory,
            # never an actual game process and never a game resource package.
            commands = [
                ['cmake', '-S', str(MOD_ROOT), '-B', str(root / 'build'), '-A', 'Win32',
                 '-DBANK_PAYLOAD_DIR=' + str(root / 'generated')],
                ['cmake', '--build', str(root / 'build'), '--config', 'Release'],
                ['ctest', '--test-dir', str(root / 'build'), '-C', 'Release', '--output-on-failure'],
            ]
            for command in commands:
                completed = subprocess.run(command, capture_output=True, text=True, errors='replace', timeout=120)
                self.assertEqual(completed.returncode, 0, completed.stdout + completed.stderr)

            # Exercise the production file installer, but only under this fresh
            # temporary directory. Existing differing content must survive.
            executable = root / 'build/Release/bank_selector_test.exe'
            target = root / 'game/UserMODs/workshop-army-reference.h5u'
            original = package.read_bytes()
            command = [str(executable), str(package), str(target)]
            first = subprocess.run(command, capture_output=True, text=True, timeout=15)
            self.assertEqual(first.returncode, 0, first.stderr)
            self.assertEqual(target.read_bytes(), original)
            timestamp = target.stat().st_mtime_ns
            second = subprocess.run(command, capture_output=True, text=True, timeout=15)
            self.assertEqual(second.returncode, 0, second.stderr)
            self.assertEqual(target.stat().st_mtime_ns, timestamp)

            target.write_bytes(b'different existing mod')
            refused = subprocess.run(command, capture_output=True, text=True, timeout=15)
            self.assertNotEqual(refused.returncode, 0)
            self.assertIn('Nothing was overwritten', refused.stderr)
            self.assertEqual(target.read_bytes(), b'different existing mod')
            self.assertEqual(package.read_bytes(), original)

    def test_invalid_public_routes_cannot_generate_a_launcher(self):
        for routes in ([], [{'title': '', 'family': 'x', 'window_id': 'X'}],
                       [{'title': 'x', 'family': 'x', 'window_id': 'X'}] * 2):
            with self.subTest(routes=routes), self.assertRaises(ValueError):
                builder.payload(routes)
