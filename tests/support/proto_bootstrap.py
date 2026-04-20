"""Protocol protobuf import bootstrap for test scripts."""

from __future__ import annotations

from types import ModuleType


def load_protocol_module() -> ModuleType:
    """Load protocol_pb2 module from the installed anolis-protocol package."""
    import protocol_pb2 as protocol
    return protocol
