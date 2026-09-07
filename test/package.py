"""Validate the PBW and export fresh native evidence without publishing."""
import hashlib
import json
from pathlib import Path
import shutil
import zipfile
from libpebble2.util.stm32_crc import crc32
ROOT=Path(__file__).resolve().parent.parent
TARGETS={'basalt','chalk','diorite','emery','flint','gabbro'}
def main():
    pbw=ROOT/'build'/f'{ROOT.name}.pbw'
    digest=hashlib.sha256(pbw.read_bytes()).hexdigest()
    source=json.loads((ROOT/'package.json').read_text());reports={}
    with zipfile.ZipFile(pbw) as z:
        assert z.testzip() is None
        info=json.loads(z.read('appinfo.json'))
        assert info['uuid']==source['pebble']['uuid']
        assert info['versionLabel']==source['version']
        assert info['watchapp']['watchface'] is True
        assert info['companyName']=='Luke Steuber'
        assert not info['capabilities'] and not info['appKeys']
        assert set(info['targetPlatforms'])==TARGETS
        for platform in sorted(TARGETS):
            manifest=json.loads(z.read(platform+'/manifest.json'))
            for key in ('application','resources'):
                entry=manifest[key];data=z.read(platform+'/'+entry['name'])
                assert len(data)==entry['size'];assert crc32(data)==entry['crc']
            report=json.loads((ROOT/f'build/evidence/{platform}-report.json').read_text())
            assert report['pbwSHA256']==digest,f'{platform}: stale evidence'
            assert report['naturalNoonRollover'] and report['naturalMidnightRollover']
            if ROOT.name=='ishihara':
                assert report['minuteRegenerated'] and report['minuteIdleStable'] and report['handsToggleAndPersist']
                assert report['nativeSourceParityFields']>=8
            else:
                assert report['secondsChanged']
                cadence=report['cadenceFrames'];assert cadence['idle'] and cadence['tap'] and cadence['settled']
            assert all((ROOT/file).is_file() for file in report['frames'])
            reports[platform]=report
    evidence=ROOT/'evidence';evidence.mkdir(exist_ok=True)
    for platform,report in reports.items():
        exported=dict(report);exported['frames']=[]
        for name in report['frames']:
            file=ROOT/name
            if file.stem.endswith('-boot'):continue
            shutil.copy2(file,evidence/file.name);exported['frames'].append('evidence/'+file.name)
        (evidence/f'{platform}-report.json').write_text(json.dumps(exported,indent=2)+'\n')
    shutil.copy2(ROOT/'build/build.log',evidence/'build.log')
    shutil.copy2(ROOT/'build/host-test.log',evidence/'host-test.log')
    summary={'version':source['version'],'uuid':source['pebble']['uuid'],'pbwSHA256':digest,'targets':sorted(reports),'nativeChecks':'Passed; see platform reports.','nativeSourceParityFields':{p:r['nativeSourceParityFields'] for p,r in reports.items()},'limits':'Emulator evidence. Physical-watch readability and battery use remain unmeasured.'}
    (evidence/'validation.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps(summary,indent=2))
if __name__=='__main__':main()
