import glob
import sys
import yaml
import subprocess
import os

from pathlib import Path

PIPE = subprocess.PIPE

def main():
    with open('config.yml') as f:
        # use safe_load instead load
        conf = yaml.safe_load(f)

    # make keys lowercase
    conf =  {k.lower(): v for k, v in conf.items()}
    
    fn_old = Path.cwd() / "hpm1all.img"

    sm_id = conf["sm_id"]

    revision = subprocess.run(["git", "describe", "--tags"], stdout=PIPE, text=True)
    revision = int(revision.stdout.strip().split("-")[1])

    fn_new = Path.cwd() / "built" / f"hpm1all-{revision}-{sm_id}.img"

    fn_new.parent.mkdir(parents=True, exist_ok=True)
    Path.rename(fn_old, fn_new)
    
    files = glob.glob("hpm1*.img")
    for f in files:
        p = Path.cwd() / f"{f}"
        p.unlink()
        
if __name__ == "__main__":
    main()
