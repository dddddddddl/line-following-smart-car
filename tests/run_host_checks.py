"""Compile and run the actual sketch control logic using a C++ host compiler."""
from pathlib import Path
import os
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
compiler = shlex.split(os.environ.get('CXX', 'c++'))
with tempfile.TemporaryDirectory(prefix='smartcar-check-') as directory:
    binary = Path(directory) / 'control_checks'
    subprocess.run(compiler + ['-std=c++11', '-Wall', '-Wextra',
                              str(root / 'tests/control_checks.cpp'), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
