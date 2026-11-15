#!/usr/bin/env python3
"""The saved game regression (docs/architecture-plan.md, 1.4).

Each case of tests/regression.json runs tests/saves/<name>.sav for its game days in tutankhamun_sim and compares the
result with tests/reference/<name>.sav through tutankhamun_sim --diff-saves. The references are saved by the current
build and guard the refactoring: a change that keeps the game's behaviour keeps every case the same.

    python tests/run_regression.py --sim <tutankhamun_sim> --game <game dir>            check every case
    python tests/run_regression.py --sim ... --game ... --update                         save the references again
    python tests/run_regression.py --sim ... --game ... --only abu-ferry kerma-invasions  some cases

--sim and --game default to TUTANKHAMUN_SIM and TUTANKHAMUN_GAME_DIR. The game directory is the user's copy of
Pharaoh + Cleopatra (with its Data folder); the configs are found beside tutankhamun_sim. The results and the logs go
to out/regression (--out). Exit 0 when every case is the same as its reference.
"""

import argparse
import concurrent.futures
import json
import os
import shutil
import subprocess
import sys
import time

TESTS = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(TESTS)


def run_case(case, args):
    name = case['name']
    save = os.path.join(TESTS, 'saves', name + '.sav')
    reference = os.path.join(TESTS, 'reference', name + '.sav')
    result = os.path.join(args.out, name + '.sav')
    log = os.path.join(args.out, name + '.log')
    started = time.monotonic()
    run = subprocess.run([args.sim, args.game, save, '--days', str(case['days']), '--out', result, '--log', log],
                         capture_output=True, text=True)
    seconds = time.monotonic() - started
    if run.returncode != 0:
        return name, 'ERROR', seconds, (run.stderr or run.stdout).strip() + f' (see {log})'
    if args.update:
        shutil.copyfile(result, reference)
        return name, 'SAVED', seconds, ''
    if not os.path.exists(reference):
        return name, 'NO REFERENCE', seconds, 'run with --update to save one'
    diff = subprocess.run([args.sim, '--diff-saves', reference, result], capture_output=True, text=True)
    if diff.returncode == 0:
        return name, 'SAME', seconds, ''
    return name, 'DIFFERS' if diff.returncode == 1 else 'ERROR', seconds, (diff.stdout + diff.stderr).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--sim', default=os.environ.get('TUTANKHAMUN_SIM'))
    parser.add_argument('--game', default=os.environ.get('TUTANKHAMUN_GAME_DIR'))
    parser.add_argument('--out', default=os.path.join(ROOT, 'out', 'regression'))
    parser.add_argument('--update', action='store_true', help='save the results as the references')
    parser.add_argument('--only', nargs='+', metavar='NAME', help='run these cases only')
    parser.add_argument('--jobs', type=int, default=max(1, (os.cpu_count() or 2) // 2))
    args = parser.parse_args()
    if not args.sim or not args.game:
        parser.error('--sim and --game (or TUTANKHAMUN_SIM and TUTANKHAMUN_GAME_DIR) are needed')
    with open(os.path.join(TESTS, 'regression.json'), encoding='utf-8') as f:
        cases = json.load(f)['cases']
    if args.only:
        unknown = set(args.only) - {c['name'] for c in cases}
        if unknown:
            parser.error('no such case: ' + ', '.join(sorted(unknown)))
        cases = [c for c in cases if c['name'] in args.only]
    os.makedirs(args.out, exist_ok=True)
    os.makedirs(os.path.join(TESTS, 'reference'), exist_ok=True)

    started = time.monotonic()
    failed = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for name, state, seconds, detail in pool.map(lambda c: run_case(c, args), cases):
            print(f'{name:24} {state:12} {seconds:6.1f} s')
            if detail:
                print('    ' + detail.replace('\n', '\n    '))
            failed += state not in ('SAME', 'SAVED')
    print(f'{len(cases) - failed} of {len(cases)} cases {"saved" if args.update else "the same"}, '
          f'{time.monotonic() - started:.1f} s')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
