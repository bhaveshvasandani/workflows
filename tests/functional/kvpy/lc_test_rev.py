#!/usr/bin/env python3
from contextlib import ExitStack

from hse2 import hse

from utility import lifecycle, cli


def separate_keys(kvdb: hse.Kvdb, kvs: hse.Kvs):
    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-cn", txn=t)

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-lc", txn=t)
        kvdb.sync()

    with kvdb.transaction() as t:
        kvs.put(b"ab03", b"val-c0", txn=t)

    # Cursor
    with kvs.cursor(filt=b"ab", flags=hse.CursorFlag.REVERSE) as c:
        # Read all keys
        assert c.read() == (b"ab03", b"val-c0")
        assert c.read() == (b"ab02", b"val-lc")
        assert c.read() == (b"ab01", b"val-cn")
        assert c.read() == (None, None) and c.eof == True

        # Seek, read
        c.seek(b"ab03")
        assert c.read() == (b"ab03", b"val-c0")
        c.seek(b"ab02")
        assert c.read() == (b"ab02", b"val-lc")
        c.seek(b"ab01")
        assert c.read() == (b"ab01", b"val-cn")

        # Read, update, read
        c.seek(b"ab03")
        assert c.read() == (b"ab03", b"val-c0")
        c.update_view()
        assert c.read() == (b"ab02", b"val-lc")
        c.update_view()
        assert c.read() == (b"ab01", b"val-cn")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True

        # Seek, update, read
        c.seek(b"ab03")
        c.update_view()
        assert c.read() == (b"ab03", b"val-c0")
        c.seek(b"ab02")
        c.update_view()
        assert c.read() == (b"ab02", b"val-lc")
        c.seek(b"ab01")
        c.update_view()
        assert c.read() == (b"ab01", b"val-cn")


def duplicate_lc_cn(kvdb: hse.Kvdb, kvs: hse.Kvs, cursor_sync: bool=False):
    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-cn", txn=t)

    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-lc", txn=t)
        kvdb.sync()

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-c0", txn=t)

    # Cursor
    with kvs.cursor(filt=b"ab", flags=hse.CursorFlag.REVERSE) as c:
        if cursor_sync:
            kvdb.sync()

        # Read all keys
        assert c.read() == (b"ab02", b"val-c0")
        assert c.read() == (b"ab01", b"val-lc")
        assert c.read() == (None, None) and c.eof == True

        # Seek, read
        c.seek(b"ab03")
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab02")
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab01")
        assert c.read() == (b"ab01", b"val-lc")
        c.seek(b"ab00")
        assert c.read() == (None, None) and c.eof == True

        # Read, update, read
        c.seek(b"ab02")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.update_view()
        assert c.read() == (b"ab01", b"val-lc")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True

        # Seek, update, read
        c.seek(b"ab02")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab01")
        c.update_view()
        assert c.read() == (b"ab01", b"val-lc")
        c.seek(b"ab00")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True


def duplicate_c0_lc(kvdb: hse.Kvdb, kvs: hse.Kvs, cursor_sync: bool=False):
    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-cn", txn=t)

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-lc", txn=t)
        kvdb.sync()

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-c0", txn=t)

    with kvs.cursor(filt=b"ab", flags=hse.CursorFlag.REVERSE) as c:
        if cursor_sync:
            kvdb.sync()

        # Read all keys
        assert c.read() == (b"ab02", b"val-c0")
        assert c.read() == (b"ab01", b"val-cn")
        assert c.read() == (None, None) and c.eof == True

        # Seek, read
        c.seek(b"ab03")
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab02")
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab01")
        assert c.read() == (b"ab01", b"val-cn")
        c.seek(b"ab00")
        assert c.read() == (None, None) and c.eof == True

        # Read, update, read
        c.seek(b"ab03")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.update_view()
        assert c.read() == (b"ab01", b"val-cn")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True

        # Seek, update, read
        c.seek(b"ab02")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab01")
        c.update_view()
        assert c.read() == (b"ab01", b"val-cn")
        c.seek(b"ab00")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True


def tombs_c0_lc(kvdb: hse.Kvdb, kvs: hse.Kvs, cursor_sync: bool=False):
    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-cn", txn=t)

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-lc", txn=t)
        kvdb.sync()

    with kvdb.transaction() as t:
        kvs.delete(b"ab02", txn=t)

    # Cursor
    with kvs.cursor(filt=b"ab", flags=hse.CursorFlag.REVERSE) as c:
        if cursor_sync:
            kvdb.sync()

        # Read all keys
        assert c.read() == (b"ab01", b"val-cn")
        assert c.read() == (None, None) and c.eof == True

        # Seek, read
        c.seek(b"ab02")
        assert c.read() == (b"ab01", b"val-cn")
        c.seek(b"ab01")
        assert c.read() == (b"ab01", b"val-cn")
        c.seek(b"ab00")
        assert c.read() == (None, None) and c.eof == True

        # Read, update, read
        c.seek(b"ab01")
        c.update_view()
        assert c.read() == (b"ab01", b"val-cn")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True

        # Seek, update, read
        c.seek(b"ab01")
        c.update_view()
        assert c.read() == (b"ab01", b"val-cn")
        c.seek(b"ab00")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True


def tombs_lc_cn(kvdb: hse.Kvdb, kvs: hse.Kvs, cursor_sync: bool=False):
    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-cn", txn=t)

    with kvdb.transaction() as t:
        kvs.delete(b"ab01", txn=t)
        kvdb.sync()

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-c0", txn=t)

    # Cursor
    with kvs.cursor(filt=b"ab", flags=hse.CursorFlag.REVERSE) as c:
        if cursor_sync:
            kvdb.sync()

        # Read all keys
        assert c.read() == (b"ab02", b"val-c0")
        assert c.read() == (None, None) and c.eof == True

        # Seek, read
        c.seek(b"ab03")
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab02")
        assert c.read() == (b"ab02", b"val-c0")

        # Read, update, read
        c.seek(b"ab03")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True

        # Seek, update, read
        c.seek(b"ab03")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab02")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")


def ptombs_c0_lc(kvdb: hse.Kvdb, kvs: hse.Kvs, cursor_sync: bool=False):
    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-cn", txn=t)

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-lc", txn=t)
        kvdb.sync()

    with kvdb.transaction() as t:
        kvs.prefix_delete(b"ab", txn=t)

    # Cursor
    with kvs.cursor(filt=b"ab", flags=hse.CursorFlag.REVERSE) as c:
        if cursor_sync:
            kvdb.sync()

        # Read all keys
        assert c.read() == (None, None) and c.eof == True

        # Seek, read
        c.seek(b"ab00")
        assert c.read() == (None, None) and c.eof == True
        c.seek(b"ab01")
        assert c.read() == (None, None) and c.eof == True
        c.seek(b"ab02")
        assert c.read() == (None, None) and c.eof == True
        c.seek(b"ab03")
        assert c.read() == (None, None) and c.eof == True

        # Read, update, read
        c.seek(b"ab01")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True

        # Seek, update, read
        c.seek(b"ab01")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True
        c.seek(b"ab02")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True


def ptombs_lc_cn(kvdb: hse.Kvdb, kvs: hse.Kvs, cursor_sync: bool=False):
    with kvdb.transaction() as t:
        kvs.put(b"ab01", b"val-cn", txn=t)

    with kvdb.transaction() as t:
        kvs.prefix_delete(b"ab", txn=t)
        kvdb.sync()

    with kvdb.transaction() as t:
        kvs.put(b"ab02", b"val-c0", txn=t)

    # Cursor
    with kvs.cursor(filt=b"ab", flags=hse.CursorFlag.REVERSE) as c:
        if cursor_sync:
            kvdb.sync()

        # Read all keys
        assert c.read() == (b"ab02", b"val-c0")
        x = c.read()
        assert c.read() == (None, None) and c.eof == True

        # Seek, read
        c.seek(b"ab03")
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab02")
        assert c.read() == (b"ab02", b"val-c0")

        # Read, update, read
        c.seek(b"ab03")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.update_view()
        assert c.read() == (None, None) and c.eof == True

        # Seek, update, read
        c.seek(b"ab03")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")
        c.seek(b"ab02")
        c.update_view()
        assert c.read() == (b"ab02", b"val-c0")


hse.init(cli.HOME)

try:
    with ExitStack() as stack:
        kvdb_ctx = lifecycle.KvdbContext().rparams("dur_enable=0")
        kvdb = stack.enter_context(kvdb_ctx)

        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            separate_keys(kvdb, kvs)

        # Duplicate keys
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            duplicate_c0_lc(kvdb, kvs, cursor_sync=False)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            duplicate_lc_cn(kvdb, kvs, cursor_sync=False)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            duplicate_c0_lc(kvdb, kvs, cursor_sync=True)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            duplicate_lc_cn(kvdb, kvs, cursor_sync=True)

        # With deletes
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            tombs_c0_lc(kvdb, kvs, cursor_sync=False)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            tombs_lc_cn(kvdb, kvs, cursor_sync=False)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            tombs_c0_lc(kvdb, kvs, cursor_sync=True)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            tombs_lc_cn(kvdb, kvs, cursor_sync=True)

        # With prefix deletes
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            ptombs_c0_lc(kvdb, kvs, cursor_sync=False)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            ptombs_lc_cn(kvdb, kvs, cursor_sync=False)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            ptombs_lc_cn(kvdb, kvs, cursor_sync=True)
        with lifecycle.KvsContext(kvdb, "test_kvs").cparams(
            "pfx_len=2", "sfx_len=1"
        ).rparams("transactions_enable=1") as kvs:
            ptombs_c0_lc(kvdb, kvs, cursor_sync=True)
finally:
    hse.fini()
