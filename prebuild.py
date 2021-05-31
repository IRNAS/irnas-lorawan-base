import subprocess
import os.path
 
arduinoCorePath = os.path.expanduser("~/Library/Arduino15/packages/IRNAS/hardware/stm32l0/0.0.12")
if not os.path.isdir(arduinoCorePath):
    print(f"ERROR: Cannot find Arduino core directory {arduinoCorePath}")
    exit(1)

coreBranch = subprocess.check_output(["git", "branch", "--show-current"], cwd=arduinoCorePath).decode().strip()

if coreBranch != "master":
    # Try to switch branch
    subprocess.check_call(["git", "checkout", "master"], cwd=arduinoCorePath)

coreBranch = subprocess.check_output(["git", "branch", "--show-current"], cwd=arduinoCorePath).decode().strip()

if coreBranch != "master":
    # git checkout master
    print(f"ERROR: Arduino core should be 'master' branch in {arduinoCorePath}")
    exit(1)