"""Optional x86-emulator check; never starts or attaches to the game."""
import importlib.util
import random
import json
from pathlib import Path
import struct
import sys
import unittest

try:
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
    from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                  UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                                  UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)
except ImportError:
    Uc = None

try:
    import keystone
except ImportError:
    keystone = None

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tests'))
from reference_support import DEVKIT
sys.path.insert(0, str(DEVKIT / 'scripts'))
SPEC = importlib.util.spec_from_file_location('native_probe', DEVKIT / 'scripts/native-probe.py')
probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(probe)



@unittest.skipIf(Uc is None, 'Unicorn is required for emulation checks')
class NativeTests(unittest.TestCase):
    @unittest.skipIf(keystone is None, 'Optional keystone assembler is not installed')
    def test_strategy_selects_separate_root_and_preserves_native_state(self):
        title = 'Imp cache'.encode('utf-16-le')
        routes = [{'title': 'Gargoyles', 'family': '03', 'window_id': 'WORKSHOP_BANK_03'},
                  {'title': 'Imp cache', 'family': '05', 'window_id': 'WORKSHOP_BANK_05'}]
        machine = Uc(UC_ARCH_X86, UC_MODE_32)
        for page, size in [(probe.LAYOUT_ENTRY & ~4095, 4096), (0x200000, 65536),
                           (0x300000, 8192), (0x400000, 8192), (0xfd9000, 4096),
                           (0xa4a000, 4096), (0x575000, 4096), (0xd11000, 4096)]:
            machine.mem_map(page, size)
        machine.mem_write(0x300000, probe.layout_trampoline(0x300000, 0x301000, routes))
        machine.mem_write(0x301000, probe.layout_data(0x301000, routes))
        machine.mem_write(0xfd9668, struct.pack('<I', 0x400300))
        machine.mem_write(0x400200, struct.pack('<I', 0x400700))
        machine.mem_write(0x400700, struct.pack('<I', 0x401200))
        machine.mem_write(0x401200, b'\xb8\x0c\x02\x40\x00\xc3')
        # Factory returns a distinct window; cast exposes its gathering interface.
        machine.mem_write(0xa4aa50, b'\xb8\x00\x0c\x40\x00\xc3')
        machine.mem_write(0x575700, b'\xb8\x00\x0d\x40\x00\xc3')
        machine.mem_write(0x400c00, struct.pack('<I', 0x400c40))
        machine.mem_write(0x400c68, struct.pack('<I', 0x401500))
        machine.mem_write(0x400090, struct.pack('<I', 0x400600))
        machine.mem_write(0x401500, b'\xff\x05\x8c\x00\x40\x00\xa1\x90\x00\x40\x00\xc2\x04\x00')
        machine.mem_write(0x400604, struct.pack('<I', 0x400500))
        machine.mem_write(0xd119fc, b'\xb8\x00\x0a\x40\x00\xc3')
        machine.mem_write(0x400a00, struct.pack('<I', 0x400b00))
        machine.mem_write(0x400b00, struct.pack('<I', 0x401300))
        machine.mem_write(0x401300, b'\x8b\x44\x24\x08\xa3\x84\x00\x40\x00\xc2\x0c\x00')
        registers = [UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                     UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_ESP, UC_X86_REG_EFLAGS]
        initial = [12, 34, 56, 78, 90, 0x400200, 0x208000, 0x246]
        # Repeated switching must leave the stock root untouched and reuse our root.
        for index, matches in enumerate([False, True, False, True, True]):
            actual = title if matches else 'Ore stock'.encode('utf-16-le')
            machine.mem_write(0x400800, actual)
            machine.mem_write(0x40020c, struct.pack('<II', 0x400800, 0x400800 + len(actual)))
            if index == 4:
                # A destroyed UI root must fall back, never route to a stale window.
                machine.mem_write(0x40060b, b'\x80')
            for register, value in zip(registers, initial):
                machine.reg_write(register, value)
            machine.emu_start(0x300000, probe.LAYOUT_ENTRY + len(probe.LAYOUT_ORIGINAL), count=300)
            expected = 0x400600 if matches and index != 4 else 0x400300
            self.assertEqual(machine.reg_read(UC_X86_REG_EBP), expected)
            self.assertEqual([machine.reg_read(r) for r in registers], initial)
            self.assertEqual(struct.unpack('<I', machine.mem_read(0xfd9668, 4))[0], 0x400300)
            self.assertEqual(struct.unpack('<I', machine.mem_read(0x40008c, 4))[0], int(index > 0))
        self.assertEqual(struct.unpack('<III', machine.mem_read(0x301008, 12)), (5, 3, 2))
        # A second family must allocate its own root, even with a dead first cache.
        actual = 'Gargoyles'.encode('utf-16-le')
        machine.mem_write(0x400800, actual)
        machine.mem_write(0x40020c, struct.pack('<II', 0x400800, 0x400800 + len(actual)))
        machine.mem_write(0x400904, struct.pack('<I', 0x400500))
        machine.mem_write(0x400090, struct.pack('<I', 0x400900))
        for register, value in zip(registers, initial):
            machine.reg_write(register, value)
        machine.emu_start(0x300000, probe.LAYOUT_ENTRY + len(probe.LAYOUT_ORIGINAL), count=300)
        self.assertEqual(machine.reg_read(UC_X86_REG_EBP), 0x400900)
        self.assertEqual(struct.unpack('<I', machine.mem_read(0x40008c, 4))[0], 2)
        self.assertEqual(struct.unpack('<II', machine.mem_read(0x301100, 8)), (0x400900, 0x400600))

    def test_counter_preserves_entry_semantics_registers_flags_and_stack(self):
        randomizer = random.Random(19)
        registers = [UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                     UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP,
                     UC_X86_REG_EFLAGS]
        for case in range(32):
            initial = [randomizer.randrange(0x100000000) for _ in range(7)] + [0x208000, 0x202 | (case & 1)]
            results = []
            for use_probe in (False, True):
                machine = Uc(UC_ARCH_X86, UC_MODE_32)
                machine.mem_map(probe.ENTRY & ~4095, 4096)
                machine.mem_map(0x200000, 65536)
                machine.mem_map(0x300000, 8192)
                for register, value in zip(registers, initial):
                    machine.reg_write(register, value)
                machine.mem_write(probe.ENTRY, probe.ORIGINAL)
                if use_probe:
                    machine.mem_write(0x300000, probe.trampoline(0x300000, 0x301000))
                machine.emu_start(0x300000 if use_probe else probe.ENTRY,
                                  probe.ENTRY + len(probe.ORIGINAL), count=20)
                results.append(([machine.reg_read(register) for register in registers],
                                bytes(machine.mem_read(0x207f8c, 4)),
                                bytes(machine.mem_read(0x208000, 64))))
                self.assertEqual(struct.unpack('<I', machine.mem_read(0x301000, 4))[0], int(use_probe))
            # Reserved, uninitialized locals are scratch; saved EDI and caller frame must match.
            self.assertEqual(results[0], results[1])


class RecipeTests(unittest.TestCase):
    def test_recipe_owns_all_reference_families_without_another_mod(self):
        recipe = json.loads((Path(__file__).resolve().parents[1] / 'mod.json').read_text(encoding='utf-8'))
        self.assertEqual(recipe['id'], 'army-reference')
        self.assertNotIn('catalog', recipe['reference_windows'])
        families = list(recipe['object_reference']['banks'].values())
        self.assertEqual(sorted(family['info'] for family in families), [f'{number:02d}' for number in range(1, 14)])
        for family in families:
            self.assertTrue(family['stats'].startswith('Bank'))
            self.assertTrue(family['tiers'])
            self.assertTrue(all(isinstance(tier, int) and tier > 0 for tier in family['tiers']))

    def test_display_labels_accept_the_generator_placeholders(self):
        recipe = json.loads((Path(__file__).resolve().parents[1] / 'mod.json').read_text(encoding='utf-8'))
        labels = recipe['reference_windows']['labels']
        self.assertIn('T2', labels['tier'].format(tier=2))
        count = labels['count'].format(size=10, range_color='<color=#ed760e>', amount='90-135')
        self.assertIn('90-135', count)
        self.assertIn('67%', labels['chance'].format(chance=67))
