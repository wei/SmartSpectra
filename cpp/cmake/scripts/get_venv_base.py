###########################################################
# get_venv_base.cmake
# Created by Greg on 2/14/2025.
# Copyright (C) 2025 Presage Security, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
###########################################################
import os
import sys

PROGRAM_EXIT_FAILURE = 1

def activate_venv(venv_path):
    activate_script = os.path.join(venv_path, 'bin', 'activate_this.py')
    if os.path.exists(activate_script):
        with open(activate_script) as f:
            exec(f.read(), {'__file__': activate_script})
    else:
        print(f"Activation script {activate_script} not found.")
        sys.exit(1)

def get_venv_base():
    if hasattr(sys, 'real_prefix') or (hasattr(sys, 'base_prefix') and sys.base_prefix != sys.prefix):
        return sys.prefix
    elif 'VIRTUAL_ENV' in os.environ:
        return os.environ['VIRTUAL_ENV']
    else:
        return None

def main():
    venv_base = get_venv_base()
    if not venv_base:
        # Assuming the virtual environment path is passed as an environment variable
        venv_path = os.getenv('VIRTUAL_ENV_PATH')
        if venv_path:
            activate_venv(venv_path)
            venv_base = get_venv_base()

    if venv_base:
        print(venv_base)
    else:
        print("Not running inside a virtual environment")
        sys.exit(PROGRAM_EXIT_FAILURE)

if __name__ == "__main__":
    main()
