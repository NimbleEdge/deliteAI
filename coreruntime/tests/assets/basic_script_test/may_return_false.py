# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

def main(inputs):
    return {
        "__NIMBLE_EXIT_STATUS": not inputs["shouldFail"],
        "numericData": 5,
        "message": "Some error occurred",
    }