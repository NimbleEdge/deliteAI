# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import pytest
from nimbleedge import simulator

@pytest.fixture(autouse=True)
def after_each():
    yield
    simulator.cleanup()
