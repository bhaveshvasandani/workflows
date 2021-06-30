# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2020-2021 Micron Technology, Inc. All rights reserved.

import os
from enum import Enum, IntFlag
from types import TracebackType
from typing import Iterator, List, Optional, Tuple, Type, Any, Union

def init(*params: str) -> None:
    """
    @SUB@ hse.init.__doc__
    """
    ...

def fini() -> None:
    """
    @SUB@ hse.fini.__doc__
    """
    ...

class KvdbException(Exception):
    """
    @SUB@ hse.KvdbException.__doc__
    """

    returncode: int
    def __init__(self, returncode: int) -> None: ...

class SyncFlag(IntFlag):
    ASYNC = ...

class Kvdb:
    def close(self) -> None:
        """
        @SUB@ hse.Kvdb.close.__doc__
        """
        ...
    @staticmethod
    def create(home: Optional[Union[str, os.PathLike[str]]] = ..., *params: str) -> None:
        """
        @SUB@ hse.Kvdb.create.__doc__
        """
        ...
    @staticmethod
    def drop(home: Optional[Union[str, os.PathLike[str]]] = ..., *params: str) -> None:
        """
        @SUB@ hse.Kvdb.drop.__doc__
        """
        ...
    @staticmethod
    def open(home: Optional[Union[str, os.PathLike[str]]] = ..., *params: str) -> Kvdb:
        """
        @SUB@ hse.Kvdb.open.__doc__
        """
        ...
    @property
    def kvs_names(self) -> List[str]:
        """
        @SUB@ hse.Kvdb.kvs_names.__doc__
        """
        ...
    def kvs_create(self, name: str, *params: str) -> None:
        """
        @SUB@ hse.Kvdb.kvs_create.__doc__
        """
        ...
    def kvs_drop(self, name: str) -> None:
        """
        @SUB@ hse.Kvdb.kvs_drop.__doc__
        """
        ...
    def kvs_open(self, name: str, *params: str) -> Kvs:
        """
        @SUB@ hse.Kvdb.kvs_open.__doc__
        """
        ...
    def sync(self, flags: SyncFlag = ...) -> None:
        """
        @SUB@ hse.Kvdb.sync.__doc__
        """
        ...
    def compact(self, cancel: bool = ..., samp_lwm: bool = ...) -> None:
        """
        @SUB@ hse.Kvdb.compact.__doc__
        """
        ...
    @property
    def compact_status(self) -> KvdbCompactStatus:
        """
        @SUB@ hse.Kvdb.compact_status.__doc__
        """
        ...
    @property
    def storage_info(self) -> KvdbStorageInfo:
        """
        @SUB@ hse.Kvdb.storage_info.__doc__
        """
        ...
    def transaction(self) -> Transaction:
        """
        @SUB@ hse.Kvdb.transaction.__doc__
        """
        ...

class PutFlag(IntFlag):
    PRIORITY = ...

class CursorFlag(IntFlag):
    REVERSE = ...
    STATIC_VIEW = ...
    BIND_TXN = ...

class Kvs:
    def close(self) -> None:
        """
        @SUB@ hse.Kvs.close.__doc__
        """
        ...
    def put(
        self,
        key: bytes,
        value: Optional[bytes],
        txn: Optional[Transaction] = ...,
        flags: PutFlag = ...,
    ) -> None:
        """
        @SUB@ hse.Kvs.put.__doc__
        """
        ...
    def get(
        self,
        key: bytes,
        txn: Optional[Transaction] = ...,
        buf: bytearray = ...,
    ) -> Optional[bytes]:
        """
        @SUB@ hse.Kvs.get.__doc__
        """
        ...
    def get_with_length(
        self,
        key: bytes,
        txn: Optional[Transaction] = ...,
        buf: Optional[bytearray] = ...,
    ) -> Tuple[Optional[bytes], int]:
        """
        @SUB@ hse.Kvs.get_with_length.__doc__
        """
        ...
    def delete(self, key: bytes, txn: Optional[Transaction] = ...) -> None:
        """
        @SUB@ hse.Kvs.delete.__doc__
        """
        ...
    def prefix_delete(self, filt: bytes, txn: Optional[Transaction] = ...) -> int:
        """
        @SUB@ hse.Kvs.prefix_delete.__doc__
        """
        ...
    def cursor(
        self,
        filt: Optional[bytes] = ...,
        txn: Optional[Transaction] = ...,
        flags: CursorFlag = ...,
    ) -> Cursor:
        """
        @SUB@ hse.Kvs.cursor.__doc__
        """
        ...

class TransactionState(Enum):
    """
    @SUB@ hse.TransactionState.__doc__
    """

    INVALID: int
    ACTIVE: int
    COMMITTED: int
    ABORTED: int

class Transaction:
    """
    @SUB@ hse.Transaction.__doc__
    """

    def __enter__(self) -> Transaction: ...
    def __exit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_val: Optional[Any],
        exc_tb: Optional[TracebackType],
    ) -> None: ...
    def begin(self) -> None:
        """
        @SUB@ hse.Transaction.begin.__doc__
        """
        ...
    def commit(self) -> None:
        """
        @SUB@ hse.Transaction.commit.__doc__
        """
        ...
    def abort(self) -> None:
        """
        @SUB@ hse.Transaction.abort.__doc__
        """
        ...
    @property
    def state(self) -> TransactionState:
        """
        @SUB@ hse.Transaction.state.__doc__
        """
        ...

class Cursor:
    def __enter__(self) -> Cursor: ...
    def __exit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_val: Optional[Any],
        exc_tb: Optional[TracebackType],
    ) -> None: ...
    def destroy(self) -> None:
        """
        @SUB@ hse.Cursor.destroy.__doc__
        """
        ...
    def items(self) -> Iterator[Tuple[bytes, Optional[bytes]]]:
        """
        @SUB@ hse.Cursor.items.__doc__
        """
        ...
    def update(
        self,
        txn: Optional[Transaction] = ...,
        flags: CursorFlag = ...,
    ) -> None:
        """
        @SUB@ hse.Cursor.update.__doc__
        """
        ...
    def seek(self, key: Optional[bytes]) -> Optional[bytes]:
        """
        @SUB@ hse.Cursor.seek.__doc__
        """
        ...
    def seek_range(
        self, filt_min: Optional[bytes], filt_max: Optional[bytes]
    ) -> Optional[bytes]:
        """
        @SUB@ hse.Cursor.seek_range.__doc__
        """
        ...
    def read(self) -> Tuple[Optional[bytes], Optional[bytes]]:
        """
        @SUB@ hse.Cursor.read.__doc__
        """
        ...
    @property
    def eof(self) -> bool:
        """
        @SUB@ hse.Cursor.eof.__doc__
        """
        ...

class KvdbCompactStatus:
    """
    @SUB@ hse.KvdbCompactStatus.__doc__
    """

    @property
    def samp_lwm(self) -> int:
        """
        @SUB@ hse.KvdbCompactStatus.samp_lwm.__doc__
        """
        ...
    @property
    def samp_hwm(self) -> int:
        """
        @SUB@ hse.KvdbCompactStatus.samp_hwm.__doc__
        """
        ...
    @property
    def samp_curr(self) -> int:
        """
        @SUB@ hse.KvdbCompactStatus.samp_curr.__doc__
        """
        ...
    @property
    def active(self) -> int:
        """
        @SUB@ hse.KvdbCompactStatus.active.__doc__
        """
        ...
    @property
    def canceled(self) -> int:
        """
        @SUB@ hse.KvdbCompactStatus.canceled.__doc__
        """
        ...

class KvdbStorageInfo:
    """
    @SUB@ hse.KvdbStorageInfo.__doc__
    """

    @property
    def total_bytes(self) -> int:
        """
        @SUB@ hse.KvdbStorageInfo.total_bytes.__doc__
        """
        ...
    @property
    def available_bytes(self) -> int:
        """
        @SUB@ hse.KvdbStorageInfo.available_bytes.__doc__
        """
        ...
    @property
    def allocated_bytes(self) -> int:
        """
        @SUB@ hse.KvdbStorageInfo.allocated_bytes.__doc__
        """
        ...
    @property
    def used_bytes(self) -> int:
        """
        @SUB@ hse.KvdbStorageInfo.used_bytes.__doc__
        """
        ...
