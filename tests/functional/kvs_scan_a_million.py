#!/usr/bin/env python3

from contextlib import ExitStack
from hse2 import hse

from utility import lifecycle, cli

# Scan a million keys

keycount = 1000 * 1000
kvsname = "scan_a_million"

hse.init(cli.HOME)

try:
    with ExitStack() as stack:
        kvdb_ctx = lifecycle.KvdbContext()
        kvdb = stack.enter_context(kvdb_ctx)
        kvs_ctx = lifecycle.KvsContext(kvdb, kvsname)
        kvs = stack.enter_context(kvs_ctx)

        for i in range(keycount):
            key = f"key{i}".encode()
            kvs.put(key, None)

        with kvs.cursor() as cur:
            s = sum(1 for _ in cur.items())
            assert s == keycount
finally:
    hse.fini()
