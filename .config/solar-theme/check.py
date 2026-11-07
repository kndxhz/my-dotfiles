"""Isolated cache/retry checks; never modifies live desktop configuration."""
import datetime as dt
import json
from pathlib import Path
import runpy
import subprocess
import tempfile
from unittest.mock import patch
from zoneinfo import ZoneInfo

module = runpy.run_path(str(Path.home() / '.local/bin/solar-theme'))
solar = module['solar_times']
cfg = {'latitude': 32.684971, 'longitude': 119.009323,
       'timezone': 'Asia/Shanghai', 'retry_seconds': 900}
now = dt.datetime(2026, 11, 6, 12, tzinfo=ZoneInfo('Asia/Shanghai'))
response = json.dumps({'status': 'OK', 'results': {
    'sunrise': '2026-11-06T06:25:36+08:00',
    'sunset': '2026-11-06T17:09:32+08:00'}})
with tempfile.TemporaryDirectory(prefix='solar-theme-test-') as folder:
    with patch.dict(solar.__globals__, CACHE=Path(folder), run=lambda _: response):
        first = solar(cfg, now)
        assert first['sunrise'] == '2026-11-06T06:25:36+08:00'
    def no_network(_):
        raise AssertionError('Valid cache must not call network')
    with patch.dict(solar.__globals__, CACHE=Path(folder), run=no_network):
        assert solar(cfg, now) == first
with tempfile.TemporaryDirectory(prefix='solar-theme-test-') as folder:
    calls = []
    def fail(_):
        calls.append(1)
        raise subprocess.CalledProcessError(6, 'curl')
    with patch.dict(solar.__globals__, CACHE=Path(folder), run=fail):
        assert solar(cfg, now) is None
        assert solar(cfg, now) is None
        assert len(calls) == 1
        assert not (Path(folder) / 'sun.json').exists()
print('PASS: date-specific cache reuse, network failure retention, retry throttling.')
