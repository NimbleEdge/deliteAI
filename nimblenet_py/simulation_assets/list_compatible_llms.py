# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

def get_compatible_llms(inp):
    llms = nm.list_compatible_llms()
    return {
        "llms": llms
    }
