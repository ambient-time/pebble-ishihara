// Copyright 2026 Luke Steuber. MIT License.
// Compare native math against the actual pinned JavaScript, not a second port.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {execFileSync} from 'node:child_process';
import vm from 'node:vm';
const source=readFileSync(new URL('../reference/source.html',import.meta.url),'utf8');
const start=source.indexOf('    function labToHex(');
const end=source.indexOf('    // ---- current plate state',start);
const context=vm.createContext({});vm.runInContext(source.slice(start,end),context);
let fields=0,dots=0;
const quantize=hex=>0xc0|hex.slice(1).match(/../g).map(c=>Math.round(parseInt(c,16)/85)).reduce((a,c)=>a*4+c,0);
for(const [w,h] of [[144,168],[180,180],[200,228],[260,260]])for(let hour=1;hour<=12;hour++) {
  const R=w*.44,seed=(Math.floor((1788728880+hour*3600)/60)^0x9e37^(Math.round(R)*131))>>>0;
  const native=JSON.parse(execFileSync(new URL('../build/math-test',import.meta.url).pathname,['--dump',String(w),String(h),String(hour),String(seed)],{encoding:'utf8'}));
  const rng=context.mulberry32(seed),roll=rng(),palette=roll<.45?0:roll<.82?1:2;
  assert.equal(native.palette,palette);
  const recipe=context.DEFS[['deutan','protan','tritan'][palette]];
  const reference=context.poissonDiskSample(w/2,h/2,R,Math.max(2,R*.014),Math.max(4.5,R*.036),rng);
  assert.equal(native.dots.length,reference.length,`${w} hour ${hour}: packing count`);
  reference.forEach((dot,i)=>{
    const n=native.dots[i];
    for(const [value,index] of [[dot.x,0],[dot.y,1],[dot.r,2]])assert.ok(Math.abs(value-n[index])<1e-8,`${w} hour ${hour} dot ${i}: geometry`);
    assert.equal(n[4],quantize(context.rndColor(n[3]?recipe.fg:recipe.bg,rng)),`${w} hour ${hour} dot ${i}: LAB color`);
  });
  fields++;dots+=reference.length;
}
console.log(`Pinned-source parity: ${fields} complete fields, ${dots} circles, all coordinates/radii, palette choices and quantized LAB colors match.`);
