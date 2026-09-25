"""Build-time only: freeze the tested selector and its relocations for a native launcher."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys
from zipfile import ZipFile

from capstone import Cs, CS_ARCH_X86, CS_MODE_32

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'devkit/scripts'))
SPEC = importlib.util.spec_from_file_location('reference_probe', ROOT / 'devkit/scripts/native-probe.py')
probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(probe)
BASE = 0x10000000


def payload(routes):
    """Keep game addresses fixed while rebasing only owned code/data references."""
    code = probe.layout_trampoline(BASE, BASE + 4096, routes)
    alternate = probe.layout_trampoline(BASE + 0x10000000, BASE + 0x10001000, routes)
    if len(code) != len(alternate) or len(code) > 4096:
        raise ValueError('Unsupported selector code layout')
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    relocations = []
    for instruction in decoder.disasm(code, BASE):
        for offset, size in [(instruction.imm_offset, instruction.imm_size),
                             (instruction.disp_offset, instruction.disp_size)]:
            if size != 4:
                continue
            position = instruction.address - BASE + offset
            original = struct.unpack_from('<I', code, position)[0]
            changed = struct.unpack_from('<I', alternate, position)[0]
            difference = (changed - original) & 0xffffffff
            if difference == 0:
                continue
            if difference not in (0x10000000, 0xf0000000):
                raise ValueError('Unknown selector relocation')
            relocations.append((position, 1 if difference == 0x10000000 else -1))
    data = probe.layout_data(BASE + 4096, routes)
    offsets = [64, 68, 72]
    for index in range(len(routes)):
        offsets.extend(1024 + index * 512 + field for field in (16, 20, 24, 28))
    # Compare several rebased payloads with the canonical generator, including
    # addresses below the seed. A bad relocation must fail before compilation.
    for address in (0x12340000, 0x02000000, 0x60000000):
        rebased_code = rebase(code, relocations, address - BASE)
        rebased_data = rebase(data, [(offset, 1) for offset in offsets], address - BASE)
        if rebased_code != probe.layout_trampoline(address, address + 4096, routes):
            raise ValueError('Selector code rebase differs from devkit')
        if rebased_data != probe.layout_data(address + 4096, routes):
            raise ValueError('Selector data rebase differs from devkit')
    return code, data, relocations, offsets


def rebase(source, relocations, delta):
    result = bytearray(source)
    for position, direction in relocations:
        value = struct.unpack_from('<I', result, position)[0]
        struct.pack_into('<I', result, position, (value + direction * delta) & 0xffffffff)
    return bytes(result)


def generate(package, output):
    with ZipFile(package) as archive:
        routes = json.loads(archive.read('UI/WorkshopArmy/routes.json').decode('utf-8'))
    code, data, relocations, offsets = payload(routes)
    lines = ['#pragma once', '#include <cstdint>', 'namespace bank_payload {',
             f'constexpr uint32_t base = 0x{BASE:x};',
             f'constexpr char packageHash[] = "{hashlib.sha256(package.read_bytes()).hexdigest()}";',
             'struct Relocation { unsigned offset; int direction; };']
    for name, values in [('code', code), ('data', data)]:
        lines.append(f'constexpr unsigned char {name}[] = {{' + ','.join(str(value) for value in values) + '};')
    lines.append('constexpr Relocation codeRelocations[] = {' +
                 ','.join('{' + f'{offset},{direction}' + '}' for offset, direction in relocations) + '};')
    lines.append('constexpr unsigned dataRelocations[] = {' + ','.join(map(str, offsets)) + '};')
    lines.append('}')
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text('\n'.join(lines) + '\n', encoding='ascii')
    return {'routes': len(routes), 'code_bytes': len(code), 'data_bytes': len(data),
            'package_sha256': hashlib.sha256(package.read_bytes()).hexdigest()}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('package', type=Path)
    parser.add_argument('--output', type=Path, default=ROOT / '.local/generated/bank_payload.hpp')
    arguments = parser.parse_args()
    print(json.dumps(generate(arguments.package, arguments.output)))
