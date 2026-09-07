"""Stage a validated local release and build its source archive independently."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import zipfile
ROOT=Path(__file__).resolve().parent.parent

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    subprocess.run([sys.executable,str(ROOT/'test/package.py')],check=True,cwd=ROOT)
    package=json.loads((ROOT/'package.json').read_text());version=package['version'];slug=package['name']
    out=ROOT/'release';out.mkdir(exist_ok=True)
    artifact=out/f'{slug}-{version}.pbw';shutil.copy2(ROOT/'build'/f'{ROOT.name}.pbw',artifact)
    archive=out/f'{slug}-{version}-source.zip'
    files=[ROOT/name for name in ['package.json','wscript','Makefile','README.md','LICENSE','.gitignore','.gitattributes']]
    for directory in ['src','resources','reference','test','tools','evidence']:
        files.extend(p for p in (ROOT/directory).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name not in ['independent-build.json','independent-build.log'])
    with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
        for file in sorted(files):z.write(file,file.relative_to(ROOT))
        assert z.testzip() is None
    with tempfile.TemporaryDirectory(prefix=slug+'-portable-') as temp:
        target=Path(temp)/slug;target.mkdir()
        with zipfile.ZipFile(archive) as z:z.extractall(target)
        with (ROOT/'evidence/independent-build.log').open('w') as log:
            subprocess.run(['make','test'],cwd=target,stdout=log,stderr=subprocess.STDOUT,check=True)
            cli=shutil.which('pebble') or str(Path.home()/'.local/bin/pebble')
            subprocess.run([cli,'build','--sdk','4.33.1'],cwd=target,stdout=log,stderr=subprocess.STDOUT,check=True)
        independent=target/'build'/f'{slug}.pbw'
        with zipfile.ZipFile(independent) as z:
            assert z.testzip() is None;info=json.loads(z.read('appinfo.json'))
            assert set(info['targetPlatforms'])==set(package['pebble']['targetPlatforms'])
            assert info['uuid']==package['pebble']['uuid'] and info['versionLabel']==version
        report={'archiveSHA256':sha(archive),'sourceArchive':archive.name,'independentPBWSHA256':sha(independent),'targets':info['targetPlatforms'],'hostMathTests':'passed','sdk':'4.33.1','buildOutsideSourceTree':'passed','note':'Canonical PBW remains the binary tied to native screenshots.'}
    (ROOT/'evidence/independent-build.json').write_text(json.dumps(report,indent=2)+'\n')
    (out/'SHA256SUMS').write_text(''.join(f'{sha(p)}  {p.name}\n' for p in sorted(out.iterdir()) if p.suffix in ['.pbw','.zip']))
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
