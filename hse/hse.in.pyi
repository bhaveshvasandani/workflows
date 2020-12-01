# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2020 Micron Technology, Inc. All rights reserved.

"""
KVDB_VERSION_STRING:

@SUB@ hse.KVDB_VERSION_STRING.__doc__

KVDB_VERSION_TAG:

@SUB@ hse.KVDB_VERSION_TAG.__doc__

KVDB_VERSION_SHA:

@SUB@ hse.KVDB_VERSION_SHA.__doc__
"""


from enum import Enum
from types import TracebackType
from typing import Dict, Iterable, Iterator, List, Optional, Tuple, Type, Any, Union


KVDB_VERSION_STRING: str
KVDB_VERSION_TAG: str
KVDB_VERSION_SHA: str


class KvdbException(Exception):
    """
    @SUB@ hse.KvdbException.__doc__
    """

    returncode: int

    def __init__(self, returncode: int) -> KvdbException:
        ...


class Kvdb:
    def close(self) -> None:
        """
        @SUB@ hse.Kvdb.close.__doc__
        """
        ...

    @staticmethod
    def init() -> None:
        """
        @SUB@ hse.Kvdb.init.__doc__
        """
        ...

    @staticmethod
    def fini() -> None:
        """
        @SUB@ hse.Kvdb.fini.__doc__
        """
        ...

    @staticmethod
    def make(mp_name: str, params: Optional[Params] = ...) -> None:
        """
        @SUB@ hse.Kvdb.make.__doc__
        """
        ...

    @staticmethod
    def open(mp_name: str, params: Optional[Params] = ...) -> Kvdb:
        """
        @SUB@ hse.Kvdb.open.__doc__
        """
        ...

    @property
    def names(self) -> List[str]:
        """
        @SUB@ hse.Kvdb.names.__doc__
        """
        ...

    def kvs_make(self, kvs_name: str, params: Optional[Params] = ...) -> None:
        """
        @SUB@ hse.Kvdb.kvs_make.__doc__
        """
        ...

    def kvs_drop(self, kvs_name: str) -> None:
        """
        @SUB@ hse.Kvdb.kvs_drop.__doc__
        """
        ...

    def kvs_open(self, kvs_name: str, params: Optional[Params] = ...) -> Kvs:
        """
        @SUB@ hse.Kvdb.kvs_open.__doc__
        """
        ...

    def sync(self) -> None:
        """
        @SUB@ hse.Kvdb.sync.__doc__
        """
        ...

    def flush(self) -> None:
        """
        @SUB@ hse.Kvdb.flush.__doc__
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

    def transaction(self) -> Transaction:
        """
        @SUB@ hse.Kvdb.transaction.__doc__
        """
        ...


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
        priority: bool = ...,
        txn: Optional[Transaction] = ...,
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

    def delete(
        self, key: bytes, priority: bool = ..., txn: Optional[Transaction] = ...
    ) -> None:
        """
        @SUB@ hse.Kvs.delete.__doc__
        """
        ...

    def prefix_delete(
        self, filt: bytes, priority: bool = ..., txn: Optional[Transaction] = ...
    ) -> int:
        """
        @SUB@ hse.Kvs.prefix_delete.__doc__
        """
        ...

    def cursor(
        self,
        filt: Optional[bytes] = ...,
        reverse: bool = ...,
        static_view: bool = ...,
        bind_txn: bool = ...,
        txn: Optional[Transaction] = ...,
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

    def __enter__(self) -> Transaction:
        ...

    def __exit__(
        self,
        exc_type: Optional[Type[Exception]],
        exc_val: Optional[Any],
        exc_tb: Optional[TracebackType],
    ) -> None:
        ...

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
    """
    See the concept and best practices sections on the HSE Wiki at
    https://github.com/hse-project/hse/wiki
    """

    def __enter__(self) -> Cursor:
        ...

    def __exit__(
        self,
        exc_type: Optional[Type[Exception]],
        exc_val: Optional[Any],
        exc_tb: Optional[TracebackType],
    ) -> None:
        ...

    def destroy(self) -> None:
        """
        @SUB@ hse.Cursor.destroy.__doc__
        """
        ...

    def items(
        self, max_count: Optional[int] = ...
    ) -> Iterator[Tuple[bytes, Optional[bytes]]]:
        """
        @SUB@ hse.Cursor.items.__doc__
        """
        ...

    def update(
        self,
        reverse: bool = ...,
        static_view: bool = ...,
        bind_txn: bool = ...,
        txn: Optional[Transaction] = ...,
    ) -> None:
        """
        @SUB@ hse.Cursor.update.__doc__
        """
        ...

    def seek(self, key: bytes) -> Optional[bytes]:
        """
        @SUB@ hse.Cursor.seek.__doc__
        """
        ...

    def seek_range(self, filt_min: bytes, filt_max: bytes) -> Optional[bytes]:
        """
        @SUB@ hse.Cursor.seek_range.__doc__
        """
        ...

    def read(self) -> Tuple[Optional[bytes], Optional[bytes], bool]:
        """
        @SUB@ hse.Cursor.read.__doc__
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


Config = Dict[
    str,
    Optional[
        Union[
            str,
            int,
            float,
            bool,
            Iterable[Optional[Union[str, float, int, bool, "Config"]]],
            "Config",
        ]
    ],
]


class Params:
    def get(self, key: str) -> Optional[str]:
        """
        @SUB@ hse.Params.get.__doc__
        """
        ...

    def set(self, key: str, value: Optional[str]) -> Params:
        """
        @SUB@ hse.Params.set.__doc__
        """
        ...

    def from_file(self, path: str) -> Params:
        """
        @SUB@ hse.Params.from_file.__doc__
        """
        ...

    def from_string(self, input: str) -> Params:
        """
        @SUB@ hse.Params.from_string.__doc__
        """
        ...